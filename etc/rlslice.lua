local image = require 'image'
local path  = require 'path'

return function( args )
  if #args == 0 then
    io.write[[
Usage: luai rlslice.lua <width> <height> <image>

]]

    return 0
  end
  
  local width = tonumber( args[ 1 ] )
  local height = tonumber( args[ 2 ] )
  
  local name = path.realpath( args[ 3 ] )
  local png = image.load( name )
  local dir, name, ext = path.split( name )
  
  local index = 1
  
  for y = 0, png:getHeight() - 1, height do
    for x = 0, png:getWidth() - 1, width do
      local sub = png:sub( x, y, x + width - 1, y + width - 1 )
      local fn = dir .. path.separator .. name .. '_' .. index .. ext
      sub:save( fn )
      io.write( fn, '\n' )
      index = index + 1
    end
  end
  
  return 0
end
