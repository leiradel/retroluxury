return function( r, g, b )
  r = r * 32 // 256
  g = g * 64 // 256
  b = b * 32 // 256
  
  return ( r << 11 ) | ( g << 5 ) | b
end
