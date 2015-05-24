#include <rl_pack.h>

#include <string.h>

#include <rl_endian.c>
#include <rl_hash.c>

rl_entry_t* rl_find_entry( void* data, const char* name )
{
  rl_header_t* header = (rl_header_t*)data;
  
  if ( !( header->runtime_flags & RL_ENDIAN_CONVERTED ) )
  {
    if ( isle() )
    {
      header->num_entries = ne32( header->num_entries );
      
      for ( uint32_t i = 0; i < header->num_entries; i++ )
      {
        header->entries[ i ].name_hash     = ne32( header->entries[ i ].name_hash );
        header->entries[ i ].name_offset   = ne32( header->entries[ i ].name_offset );
        header->entries[ i ].data_offset   = ne32( header->entries[ i ].data_offset );
        header->entries[ i ].data_size     = ne32( header->entries[ i ].data_size );
        header->entries[ i ].runtime_flags = 0;
      }
    }
    
    header->runtime_flags = RL_ENDIAN_CONVERTED;
  }
  
  uint32_t name_hash = djb2( name );
  
  for ( uint32_t i = 0; i < header->num_entries - 1; i++ )
  {
    uint32_t ndx = ( name_hash + i ) % header->num_entries;
    rl_entry_t* entry = header->entries + ndx;
    
    if ( entry->name_hash == name_hash )
    {
      const char* entry_name = (char*)data + entry->name_offset;
      
      if ( !strcmp( name, entry_name ) )
      {
        return entry;
      }
    }
  }
  
  return NULL;
}
