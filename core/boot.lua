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

-- Remove the entire io module.
io = nil

-- Remove selected os functions.
os.execute = nil
os.exit = nil
os.getenv = nil
os.remove = nil
os.rename = nil
os.tmpname = nil

-- Rewrite loadfile.
loadfile = function( filename, mode, env )
  if mode and env then
    return load( rl.loadFile( filename ), filename, mode, env )
  elseif mode then
    return load( rl.loadFile( filename ), filename, mode )
  elseif env then
    return load( rl.loadFile( filename ), filename, nil, env )
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
