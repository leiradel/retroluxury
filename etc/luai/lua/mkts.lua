local writer = require 'writer'

return function( images )
  local res = writer()
  
  res:add16( images[ 1 ]:getWidth() )
  res:add16( images[ 1 ]:getHeight() )
  res:add16( #images )
  
  for _, img in pairs( images ) do
    for y = 0, img:getHeight() - 1 do
      for x = 0, img:getWidth() - 1 do
        res:add16( getpixel( img, x, y ) )
      end
    end
  end
  
  return res
end
