local image = require 'image'
local mkts  = require 'mkts'

return function( args )
  if #args == 0 then
    io.write[[
rltileset.lua reads all images in the given directory and writes tileset data
that can be directly fed to rl_tileset_create. The first image it finds in the
given directory will dictate the width and height of all tiles in the tileset.
Subsequent images with different dimensions won't be written to the tileset.


Usage: luai rltileset.lua [ options ] <directory>

--output <file>  writes the tileset to the given file
                 (the default is <directory>.tls)

]]
    
    return 0
  end
  
  local dir, output
  
  for i = 1, #args do
    if args[ i ] == '--output' then
      output = args[ i + 1 ]
      i = i + 1
    else
      dir = args[ i ]
    end
  end
  
  dir = path.realpath( dir )
  local files = path.scandir( dir )
  local width, height
  local images = {}
  
  for _, file in ipairs( files ) do
    local stat = path.stat( file )
    
    if stat.file then
      local ok, png = pcall( image.load, file )
      
      if ok then
        if not width then
          width, height = png:getSize()
          images[ #images + 1 ] = png
          io.write( string.format( '%s set the tileset dimensions to %dx%d\n', file, width, height ) )
        elseif width == png:getWidth() and height == png:getHeight() then
          images[ #images + 1 ] = png
        else
          io.write( string.format( '%s doesn\'t have the required dimensions\n', file ) )
        end
      else
        io.write( string.format( '%s could not be read as an image\n', file ) )
      end
    end
  end
  
  if #images ~= 0 then
    local out = mkts( images )
    
    if not output then
      local dir, name, ext = path.split( dir )
      output = dir .. path.separator .. name .. '.tls'
    end
    
    out:save( output )
  else
    io.write( 'no images were found\n' )
  end
  
  return 0
end
