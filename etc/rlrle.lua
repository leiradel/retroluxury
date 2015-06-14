local image   = require 'image'
local path    = require 'path'
local rgbto16 = require 'rgbto16'
local mkrle   = require 'mkrle'

return function( args )
  if #args == 0 then
    io.write[[
RLE-encodes an image to a format ready to be used with rl_image_create. The
--margin argument should be given, and must be the same as the value of the
RL_BACKGRND_MARGIN when retroluxury was compiled. If it's not given, the image
is still valid, but it must be entirely contained within the background
boundaries when blit.

Usage: luai rlrle.lua [ options ] <image>

--transp r g b   makes the given color transparent
--tl             makes the color at (0,0) transparent
--bl             makes the color at (0,height-1) transparent
--tr             makes the color at (width-1,0) transparent
--br             makes the color at (width-1,height-1) transparent
--margin x       sets the pixel limit on RLE runs on a row
                 (must be equal to RL_BACKGRND_MARGIN)

]]

    return 0
  end
  
  local name, limit, transp
  
  for i = 1, #args do
    if args[ i ] == '--transp' then
      transp = rgbto16( tonumber( args[ i + 1 ] ), tonumber( args[ i + 2 ] ), tonumber( args[ i + 3 ] ) )
      i = i + 3
    elseif args[ i ] == '--tl' or args[ i ] == '--bl' or args[ i ] == '--tr' or args[ i ] == '--br' then
      transp = args[ i ]
    elseif args[ i ] == '--margin' then
      limit = tonumber( args[ i + 1 ] )
      i = i + 1
    else
      name = args[ i ]
    end
  end
  
  name = path.realpath( name )
  local png = image.load( name )
  
  if transp == '--tl' then
    transp = rgbto16( image.split( png:getPixel( 0, 0 ) ) )
  elseif transp == '--bl' then
    transp = rgbto16( image.split( png:getPixel( 0, png:getHeight() - 1 ) ) )
  elseif transp == '--tr' then
    transp = rgbto16( image.split( png:getPixel( png:getWidth() - 1, 0 ) ) )
  elseif transp == '--br' then
    transp = rgbto16( image.split( png:getPixel( png:getWidth() - 1, png:getHeight() - 1 ) ) )
  end
  
  local res = mkrle( png, limit, transp )
  local dir, name, ext = path.split( name )
  res:save( dir .. path.separator .. name .. '.rle' )
  return 0
end
