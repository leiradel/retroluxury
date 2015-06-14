local writer = require 'writer'
local mkrle  = require 'mkrle'

return function( images, limit )
  local out = writer()
  
  out:add16( #images )
  
  for _, png in ipairs( images ) do
    local rle = mkrle( png, limit )
    out:add32( rle:getsize() )
    out:addwriter( rle )
  end
  
  return out
end
