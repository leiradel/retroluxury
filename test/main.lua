rl.create( 400, 300 )

local tileset = rl.loadTileSet( 'city.tls' )
local imageset = rl.loadImageSet( 'city.ims' )
local map = rl.loadMap( 'city.map', tileset, imageset )

local pad = {}

return function()
  rl.getInputState( 1, pad )
  
  map:blit0( 0, 0 )
  map:blitn( 1, 0, 0 )
  
  rl.presentVideo()
end
