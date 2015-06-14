local image = require 'image'
local mkis  = require 'mkis'

return function( args )
  if #args == 0 then
    io.write[[
rlimageset.lua creates an imageset containing all files given on the command
line.

Usage: luai rlimageset.lua [ options ] images...

--output <file>  writes the imageset to the given file
--margin x       sets the pixel limit on RLE runs on a row (must be equal to
                 RL_BACKGRND_MARGIN)
]]
    
    return 0
  end
  
  local output, limit
  local images = {}
  
  do
    local i = 1
    
    while i <= #args do
      if args[ i ] == '--output' then
        i = i + 1
        output = args[ i ]
      elseif args[ i ] == '--margin' then
        i = i + 1
        limit = tonumber( args[ i ] )
      else
        images[ #images + 1 ] = image.load( args[ i ] )
      end
    
      i = i + 1
    end
  end
  
  local out = mkis( images, limit )
  out:save( output )
  
  return 0
end
