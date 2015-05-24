local rl = require 'rl'

-- Swap 2nd searcher for one that reads from the pack file.
package.searchers[ 2 ] = function( name )
  local loader = function( name, contents )
    return contents
  end
  
  return loader, rl.loadFile( name )
end

-- Remove the remaining searchers.
local i = 3

while package.searchers[ i ] do
  package.searchers[ i ] = nil
end

-- Replace selected math functions.
math.randomseed = rl.randomseed
math.random = rl.random

-- Remove selected os functions.
os.execute = nil
os.exit = nil
os.getenv = nil
os.remove = nil
os.rename = nil
os.tmpname = nil

-- Rewrite loadfile.
loadfile = function( filename, mode, env )
  if env then
    return load( rl.loadFile( filename ), filename, mode, env )
  elseif mode then
    return load( rl.loadFile( filename ), filename, mode )
  else
    return load( rl.loadFile( filename ), filename )
  end
end

-- Rewrite dofile.
dofile = function( filename )
  local chunk = assert( loadfile( filename ) )
  return chunk()
end

return dofile( 'main.lua' )
