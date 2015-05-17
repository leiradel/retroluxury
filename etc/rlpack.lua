local function djb2( str )
  local hash = 5381
  
  for i = 1, #str do
    hash = hash * 33 + str:byte( i )
    hash = hash & 0xffffffff
  end
  
  return hash
end

local function filesize( path )
  local file, err = io.open( path, 'rb' )
  
  if not file then
    error( err )
  end
  
  local size, err = file:seek( 'end' )
  
  if not size then
    file:close()
    error( err )
  end
  
  file:close()
  return size
end

local function newwriter()
  return {
    bytes = {},
    write8 = function( self, x )
      self.bytes[ #self.bytes + 1 ] = string.char( x & 255 )
    end,
    write16 = function( self, x )
      self.bytes[ #self.bytes + 1 ] = string.char( ( x >> 8 ) & 255, x & 255 )
    end,
    write32 = function( self, x )
      self.bytes[ #self.bytes + 1 ] = string.char( ( x >> 24 ) & 255, ( x >> 16 ) & 255, ( x >> 8 ) & 255, x & 255 )
    end,
    writestr = function( self, str )
      self.bytes[ #self.bytes + 1 ] = str
    end,
    append = function( self, rle )
      self.bytes[ #self.bytes + 1 ] = table.concat( rle.bytes )
    end,
    align = function( self, x )
      local size = self:size()
      local new_size = ( size + x - 1 ) & -x
      
      while size < new_size do
        self:write8( 0 )
        size = size + 1
      end
    end,
    get = function( self )
      self.bytes = { table.concat( self.bytes ) }
      return self.bytes[ 1 ]
    end,
    size = function( self )
      self.bytes = { table.concat( self.bytes ) }
      return #self.bytes[ 1 ]
    end,
    save = function( self, name )
      local file, err = io.open( name, 'wb' )
      if not file then error( err ) end
      self.bytes = { table.concat( self.bytes ) }
      file:write( self.bytes[ 1 ] )
      file:close()
    end
  }
end

return function( args )
  if #args < 2 then
    io.write[[
Creates a .pak file to be used with retroluxury. The 'cuts' is the number of
folders to cut from the beginning of the given files.

Usage: luai rlpack.lua cuts <filename.pak> files...

]]
    
    return 0
  end
  
  local map = {}
  local num_entries = ( #args - 2 ) * 4 // 3
  
  local insert
  insert = function( entry )
    local hash = djb2( entry.name )
    local ndx  = hash % num_entries
    
    if not map[ ndx ] then
      map[ ndx ] = entry
    else
      local hash2 = djb2( map[ ndx ].name )
      local ndx2  = hash2 % num_entries
      
      if ndx2 ~= ndx then
        local old_entry = map[ ndx ]
        map[ ndx ] = entry
        insert( old_entry )
      else
        for i = 1, num_entries - 0 do
          local ndx2 = ( ndx + i ) % num_entries
          
          if not map[ ndx2 ] then
            map[ ndx2 ] = entry
            return
          end
        end
      end
    end
  end
  
  local cuts = tonumber( args[ 1 ] )
  
  for i = 3, #args do
    local name = args[ i ]
    
    for j = 1, cuts do
      name = name:gsub( '^.-/', '' )
    end
    
    insert( { name = name, path = args[ i ] } )
  end
  
  local offset =
      4 -- num entries
    + 4 -- runtime flags
    + num_entries *
    (
        4 -- name hash
      + 4 -- name offset
      + 4 -- data offset
      + 4 -- data size
      + 4 -- runtime flags
    )
  
  for i = 0, num_entries - 1 do
    local entry
    
    if map[ i ] then
      offset = ( offset + 3 ) & ~3
      entry = { name = map[ i ].name, path = map[ i ].path, name_hash = djb2( map[ i ].name ), name_offset = offset, data_offset = 0, data_size = 0 }
      offset = offset + #map[ i ].name + 1
    else
      entry = nil
    end
    
    map[ i ] = entry
  end
  
  for i = 0, num_entries - 1 do
    if map[ i ] then
      offset = ( offset + 15 ) & ~15
      map[ i ].data_offset = offset
      local size = filesize( map[ i ].path )
      map[ i ].data_size = size
      offset = offset + size
    end
  end
  
  local out = newwriter()
  out:write32( num_entries )
  out:write32( 0 )
  
  for i = 0, num_entries - 1 do
    if map[ i ] then
      local entry = map[ i ]
      
      out:write32( entry.name_hash )
      out:write32( entry.name_offset )
      out:write32( entry.data_offset )
      out:write32( entry.data_size )
      out:write32( 0 )
    else
      out:write32( 0 )
      out:write32( 0 )
      out:write32( 0 )
      out:write32( 0 )
      out:write32( 0 )
    end
  end
  
  for i = 0, num_entries - 1 do
    if map[ i ] then
      out:align( 4 )
      out:writestr( map[ i ].name )
      out:write8( 0 )
    end
  end
  
  for i = 0, num_entries - 1 do
    if map[ i ] then
      local file, err = io.open( map[ i ].path, 'rb' )
      
      if not file then
        error( err )
      end
      
      local all = file:read( '*a' )
      file:close()
      
      out:align( 16 )
      out:writestr( all )
    end
  end
  
  out:save( args[ 2 ] )
  
  for i = 0, num_entries - 1 do
    if map[ i ] then
      print( i, string.format( '%08x', map[ i ].name_hash ), map[ i ].name_offset, map[ i ].data_offset, map[ i ].data_size, map[ i ] and ( djb2( map[ i ].name ) % num_entries ) or '', map[ i ].name )
    else
      print( i )
    end
  end
  
  return 0
end
