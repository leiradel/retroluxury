local image   = require 'image'
local rgbto16 = require 'rgbto16'
local writer  = require 'writer'

local function getpixel( png, x, y, transp )
  local r, g, b, a = image.split( png:getPixel( x, y ) )
  local c = rgbto16( r, g, b )
  
  if c == transp then
    r, g, b, a = 0, 0, 0, 0
  end
  
  if a >= 0 and a <= 31 then
    a = 0 -- transparent
  elseif a >= 32 and a <= 95 then
    a = 1 -- 25%
  elseif a >= 96 and a <= 159 then
    a = 2 -- 50%
  elseif a >= 160 and a <= 223 then
    a = 3 -- 75%
  else
    a = 4 -- opaque
  end
  
  return rgbto16( r, g, b ), a
end

local function rlerow( png, y, limit, transp )
  local rle = {}
  local width = png:getWidth()
  local used = 0
  
  local numcols = ( width + ( limit - 1 ) ) // limit
  
  for i = 1, numcols do
    rle[ #rle + 1 ] = 0
  end
  
  for xx = 0, width - 1, limit do
    rle[ xx // limit + 1 ] = #rle
    
    local x = xx
    rle[ #rle + 1 ] = 0
    local runs = #rle
    
    while x < ( xx + limit ) and x < width do
      local _, a = getpixel( png, x, y, transp )
      local count = 0
      local xsave = x
      
      while x < ( xx + limit ) and x < width do
        local c, aa = getpixel( png, x, y, transp )
        
        if aa ~= a then
          break
        end
        
        count = count + 1
        x = x + 1
      end
      
      rle[ #rle + 1 ] = ( a << 13 | count )
      rle[ runs ] = rle[ runs ] + 1
      
      if a ~= 0 then
        for i = 0, count - 1 do
          local c = getpixel( png, xsave + i, y, transp )
          rle[ #rle + 1 ] = c
          used = used + 1
        end
      end
    end
  end
  
  local final = writer()
  
  for i = 1, #rle do
    final:add16( rle[ i ] )
  end
  
  return final, used
end

return function( png, limit, transp )
  local width, height = png:getSize()
  local rows = {}
  local used = 0
  
  limit = limit or 1e10
  
  for y = 0, height - 1 do
    local row, u =  rlerow( png, y, limit, transp )
    rows[ y ] = row
    used = used + u
  end
  
  local rle = writer()
  
  rle:add16( width )
  rle:add16( height )
  rle:add32( used )
  
  local count = 0
  
  for y = 0, height - 1 do
    rle:add32( count )
    count = count + rows[ y ]:getsize()
  end
  
  for y = 0, height - 1 do
    rle:addwriter( rows[ y ] )
  end
  
  return rle
end
