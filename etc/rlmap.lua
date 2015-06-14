local image  = require 'image'
local path   = require 'path'
local tmx    = require 'tmx'
local mkrle  = require 'mkrle'
local writer = require 'writer'

local function dump( t, i )
  i = i or 0
  local s = string.rep( ' ', i * 2 )
  
  if type( t ) == 'table' then
    io.write( s, '{\n' )
    for k, v in pairs( t ) do
      io.write( s, '  ', tostring( k ), ' = ' )
      dump( v, i + 1 )
    end
    io.write( s, '}\n' )
  elseif type( t ) == 'string' then
    io.write( s, string.format( '%q', t ), '\n' )
  else
    io.write( s, tostring( t ), '\n' )
  end
end

local function split( str, sep )
  sep = sep or ' '
  local res = {}
  local i = 1
  
  while #str ~= 0 do
    local j = str:find( sep, i, true )
    
    if not j then
      j = #str + 1
    end
    
    res[ #res + 1 ] = str:sub( i, j - 1 )
    str = str:sub( j + 1 )
  end
  
  return res
end

local function list_cmd( args )
  local map = tmx.load( args[ 1 ] )
  local layers = false
  local tilesets = false
  
  for i = 3, #args do
    if args[ i ] == '--layers' then
      layers = true
    elseif args[ i ] == '--tilesets' then
      tilesets = true
    else
      error( 'unknown argument to list: ' .. args[ i ] )
    end
  end
  
  if not layers and not tilesets then
    layers, tilesets = true, true
  end
  
  if layers then
    for i, layer in ipairs( map.layers ) do
      io.write( string.format( 'Layer %d: %s\n', i, layer.name ) )
    end
  end
  
  if tilesets then
    for i, tileset in ipairs( map.tilesets ) do
      io.write( string.format( 'Tileset %d: %s\n', i, tileset.name ) )
    end
  end
end

local function render_cmd( args )
  local map = tmx.load( args[ 1 ] )
  local layers = {}
  
  for i = 3, #args do
    layers[ args[ i ] ] = true
  end
  
  if not next( layers ) then
    for i, layer in ipairs( map.layers ) do
      layers[ layer.name ] = true
    end
  end
  
  local dir, name, ext = path.split( args[ 1 ] )
  tmx.render( map, layers ):save( dir .. path.separator .. name .. '.png' )
end

local function compile_cmd( args )
  local map = tmx.load( args[ 1 ] )
  local layers = {}
  local coll, limit
  
  do
    local i = 3
    
    while i <= #args do
      if args[ i ] == '--coll' then
        coll = split( args[ i + 1 ], '+' )
        i = i + 2
      elseif args[ i ] == '--margin' then
        limit = tonumber( args[ i + 1 ] )
        i = i + 2
      else
        layers[ #layers + 1 ] = split( args[ i ], '+' )
        i = i + 1
      end
    end
  end
  
  if #layers == 0 then
    error( 'the built map must have at least one layer' )
  end
  
  local built = { tiles = {}, images = {}, layer0 = {}, layers = {} }
  
  -- build layer 0 and the tileset
  do
    local names = {}
    
    for i = 1, #layers[ 1 ] do
      for _, layer in ipairs( map.layers ) do
        if layer.name == layers[ 1 ][ i ] then
          names[ layer.name ] = layer
        end
      end
    end
    
    local png = render( map, names )
    local tileset = {}
    
    for y = 0, png:getHeight() - 1, map.tileheight do
      built.layer0[ y // map.tileheight + 1 ] = {}
      
      for x = 0, png:getWidth() - 1, map.tilewidth do
        local sub = png:sub( x, y, x + 31, y + 31 )
        
        if not tileset[ sub:getHash() ] then
          built.tiles[ #built.tiles + 1 ] = sub
          tileset[ sub:getHash() ] = #built.tiles - 1
        end
        
        built.layer0[ y // map.tileheight + 1 ][ x // map.tilewidth + 1 ] = tileset[ sub:getHash() ]
      end
    end
  end
  
  -- build the other layers and the imageset
  do
    for l = 2, #layers do
      local names = {}
      
      for i = 1, #layers[ l ] do
        for _, layer in ipairs( map.layers ) do
          if layer.name == layers[ l ][ i ] then
            names[ layer.name ] = layer
          end
        end
      end
      
      local png = render( map, names )
      local imageset = {}
      
      local indices = {}
      built.layers[ l - 1 ] = indices
      
      for y = 0, png:getHeight() - 1, map.tileheight do
        indices[ y // map.tileheight + 1 ] = {}
        
        for x = 0, png:getWidth() - 1, map.tilewidth do
          local sub = png:sub( x, y, x + 31, y + 31 )
          
          if not sub:invisible() then
            if not imageset[ sub:getHash() ] then
              built.images[ #built.images + 1 ] = sub
              imageset[ sub:getHash() ] = #built.images
            end
            
            indices[ y // map.tileheight + 1 ][ x // map.tilewidth + 1 ] = imageset[ sub:getHash() ]
          else
            indices[ y // map.tileheight + 1 ][ x // map.tilewidth + 1 ] = 0
          end
        end
      end
    end
  end
  
  -- output file name
  local filename
  
  do
    local dir, name, ext = path.split( args[ 1 ] )
    filename = dir .. path.separator .. name
  end
  
  -- rl_tileset_t
  do
    local out = writer()
    
    out:add16( map.tilewidth )
    out:add16( map.tileheight )
    out:add16( #built.tiles )
    
    for _, tile in ipairs( built.tiles ) do
      for y = 0, map.tileheight - 1 do
        for x = 0, map.tilewidth - 1 do
          local r, g, b = image.split( tile:getPixel( x, y ) )
          r, g, b = r * 31 // 255, g * 63 // 255, b * 31 // 255
          out:add16( ( r << 11 ) | ( g << 5 ) | b )
        end
      end
    end
    
    out:save( filename .. '.tls' )
  end
  
  -- rl_imageset_t
  do
    local out = writer()
    
    out:add16( #built.images )
    
    for _, image in ipairs( built.images ) do
      local res = mkrle( image, limit )
      out:add32( res:getsize() )
      out:addwriter( res )
    end
    
    out:save( filename .. '.ims' )
  end
  
  -- rl_map_t
  do
    local out = writer()
    
    out:add16( map.width )
    out:add16( map.height )
    out:add16( 1 + #built.layers ) -- layer count
    
    -- map flags
    local flags = 0
    flags = flags | ( coll and 1 or 0 ) -- has collision bits
    
    out:add16( flags )
    
    -- rl_layer0
    for y = 1, map.height do
      for x = 1, map.width do
        out:add16( built.layer0[ y ][ x ] )
      end
    end
    
    -- rl_layern
    for _, layer in ipairs( built.layers ) do
      for y = 1, map.height do
        for x = 1, map.width do
          out:add16( layer[ y ][ x ] )
        end
      end
    end
    
    -- collision bits
    if coll then
      local bits, bit = 0, 1
      
      for y = 1, map.height do
        for x = 1, map.width do
          for i = 1, #coll do
            for _, layer in ipairs( map.layers ) do
              if layer.name == coll[ i ] then
                if layer.tiles[ y ][ x ] ~= 0 then
                  bits = bits | bit
                  break
                end
              end
            end
          end
          
          if bit == 0x80000000 then
            out:add32( bits )
            bits, bit = 0, 1
          else
            bit = bit << 1
          end
        end
      end
      
      if bit ~= 1 then
        out:add32( bits )
      end
    end
    
    out:save( filename .. '.map' )
  end
end

return function( args )
  if #args < 2 then
    io.write[[
An utility to work with Tiled maps and convert them to retroluxury.

rlmap understands the following commands:

* list:    Lists the layers and/or tilesets in a map.

* render:  Renders the map as a PNG image. The command accepts a list of layer
           names and will render only them if the list is given.

* compile: Converts the map into a format ready to be used with rl_map_create.
           If a collision layer is given with --coll, all non-zero tiles
           represent unpassable tiles. The map resulting layers are a
           combination of one or more map layers, they can be compined with
           the + operator. Ex.: floor+flobjs fences+walls will create a map
           with two layers, the first with the combined floor and flobjs
           layers and the other with the fences and walls layers combined.

Usage: rlmap <mapname.tmx> command args...

Commands:

  list                                  lists the map\'s layers and/or tilesets
       [--layers]                       lists only layers
       [--tilesets]                     lists only tilesets

  render                                renders the map as a PNG image
         [layername...]                 names of the layers to render

  compile                               compiles the map as a .map file
          --margin x                    sets the pixel limit on RLE runs on a row
                                        (must be equal to RL_BACKGRND_MARGIN)
          --coll layermame              sets the layer that contains collision
                                        data (any tile means block)
          layername[+layername]...      first set of layers, that will be
                                        merged into layer 0
          layername[+layername]...      second set of layers, that will be
                                        merged into layer 1 (and so forth)

]]

    return 0
  end
  
  args[ 1 ] = path.realpath( args[ 1 ] )
  
  local commands = {
    list    = list_cmd,
    render  = render_cmd,
    compile = compile_cmd
  }
  
  local cmd = commands[ args[ 2 ] ]
  
  if cmd then
    cmd( args )
  else
    error( 'unknown command: ' .. args[ 2 ] )
  end
end
