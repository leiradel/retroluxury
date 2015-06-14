local image = require 'xml'
local xml   = require 'xml'

return {
  load = function( filename )
    local dir = path.split( filename ) .. path.separator
    local file = assert( io.open( filename ) )
    
    file:read( '*l' ) -- skip <?xml ... ?>
    local contents = file:read( '*a' )
    file:close()
    
    local tmx = xml.parse( contents )
    
    local map = {}
    
    map.version = xml.findAttr( tmx, 'version' )
    map.orientation = xml.findAttr( tmx, 'orientation' )
    map.width = tonumber( xml.findAttr( tmx, 'width' ) )
    map.height = tonumber( xml.findAttr( tmx, 'height' ) )
    map.tilewidth = tonumber( xml.findAttr( tmx, 'tilewidth' ) )
    map.tileheight = tonumber( xml.findAttr( tmx, 'tileheight' ) )
    map.widthpixels = map.width * map.tilewidth
    map.heightpixels = map.height * map.tileheight
    map.backgroundcolor = image.color( 0, 0, 0 )
    
    local backgroundcolor = xml.findAttr( tmx, 'backgroundcolor' )
    
    if backgroundcolor then
      backgroundcolor = tonumber( backgroundcolor, 16 )
      local r = backgroundcolor >> 16
      local g = backgroundcolor >> 8 & 255
      local b = backgroundcolor & 255
      map.backgroundcolor = image.color( r, g, b )
    end
    
    local tilesets = {}
    map.tilesets = tilesets
    
    local gids = {}
    map.gids = gids
    
    for _, child in ipairs( tmx ) do
      if child.label == 'tileset' then
        local tileset =
        {
          firstgid = tonumber( xml.findAttr( child, 'firstgid' ) ),
          name = xml.findAttr( child, 'name' ),
          tilewidth = tonumber( xml.findAttr( child, 'tilewidth' ) ),
          tileheight = tonumber( xml.findAttr( child, 'tileheight' ) )
        }
        
        if map.tilewidth ~= tileset.tilewidth or map.tileheight ~= tileset.tileheight then
          error( string.format( 'tile dimensions in %s are different from tile dimensions in map', tileset.name ) )
        end
        
        for _, child2 in ipairs( child ) do
          if child2.label == 'image' then
            local filename = path.realpath( dir .. xml.findAttr( child2, 'source' ) )
            tileset.image = image.load( filename )
            local trans = xml.findAttr( child2, 'trans' )
            
            if trans then
              trans = tonumber( trans, 16 )
              local r = trans >> 16
              local g = trans >> 8 & 255
              local b = trans & 255
              trans = image.color( r, g, b )
              tileset.image:colorToAlpha( trans )
            end
          end
        end
        
        tileset.lastgid = tileset.firstgid + ( tileset.image:getWidth() // tileset.tilewidth ) * ( tileset.image:getHeight() // tileset.tileheight ) - 1
        
        local imagewidth = tileset.image:getWidth()
        local tilewidth = tileset.tilewidth
        local tileheight = tileset.tileheight
        
        for i = tileset.firstgid, tileset.lastgid do
          local id = i - tileset.firstgid
          local j = id * tileset.tilewidth
          local x = j % imagewidth
          local y = math.floor( j / imagewidth ) * tileset.tileheight
          gids[ i ] =
          {
            tileset = tileset,
            id = id,
            x = x,
            y = y,
            width = tilewidth,
            height = tileheight,
            image = tileset.image:sub( x, y, x + tilewidth - 1, y + tileheight - 1 )
          }
        end
        
        tilesets[ #tilesets + 1 ] = tileset
      end
    end
    
    local layers = {}
    map.layers = layers
    
    for _, child in ipairs( tmx ) do
      if child.label == 'layer' then
        local layer =
        {
          name = xml.findAttr( child, 'name' ),
          width = tonumber( xml.findAttr( child, 'width' ) ),
          height = tonumber( xml.findAttr( child, 'height' ) ),
          tiles = {}
        }
        
        for _, child2 in ipairs( child ) do
          if child2.label == 'data' then
            local index = 1
            
            for y = 1, layer.height do
              local row = {}
              layer.tiles[ y ] = row
              
              for x = 1, layer.width do
                local tile = child2[ index ]
                index = index + 1
                row[ x ] = tonumber( xml.findAttr( tile, 'gid' ) )
              end
            end
          end
        end
        
        layers[ #layers + 1 ] = layer
      end
    end
    
    return map
  end,
  render = function( map, layers )
    local png = image.create( map.widthpixels, map.heightpixels, image.color( 0, 0, 0, 0 ) )
    
    for _, layer in ipairs( map.layers ) do
      if layers[ layer.name ] then
        local yy = 0
        
        for y = 1, layer.height do
          local row = layer.tiles[ y ]
          local xx = 0
          
          for x = 1, layer.width do
            if row[ x ] ~= 0 then
              local tile = map.gids[ row[ x ] ]
              
              if not tile then
                error( 'Unknown gid ' .. row[ x ] .. ' in layer ' .. layer.name )
              end
              
              tile.image:blit( png, xx, yy )
            end
              
            xx = xx + map.tilewidth
          end
          
          yy = yy + map.tileheight
        end
      end
    end
    
    return png
  end
}
