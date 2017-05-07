#include <rl_pack.h>

#include <stdlib.h>
#include <string.h>

typedef union
{
  struct
  {
    char name[ 100 ];
    char mode[ 8 ];
    char owner[ 8 ];
    char group[ 8 ];
    char size[ 12 ];
    char modification[ 12 ];
    char checksum[ 8 ];
    char type;
    char linked[ 100 ];
  } s;
  
  char fill[ 512 ];
}
entry_t;

int rl_pack_create( rl_pack_t* pack, const void* buffer, size_t buffer_len )
{
  const entry_t* entry;
  const entry_t* end;
  int i;
  long size;
  char* endptr;
  
  if ( ( buffer_len & 511 ) != 0 || buffer_len == 0 )
  {
    return -1;
  }
  
  entry = (const entry_t*)buffer;
  end = (const entry_t*)( (const char*)buffer + buffer_len );
  
  while ( entry < end )
  {
    for ( i = 0; i < 512; i++ )
    {
      if ( ( (char*)entry )[ i ] != 0 )
      {
        goto regular;
      }
    }
    
    break;
    
  regular:
    size = strtol( entry->s.size, &endptr, 8 );
    
    if ( *endptr != 0 )
    {
      return -1;
    }
    
    entry += ( size + 511 ) / 512 + 1;
  }
  
  pack->buffer_len = ( entry - (const entry_t*)buffer ) * 512;
  
  while ( entry < end )
  {
    for ( i = 0; i < 512; i++ )
    {
      if ( ( (char*)entry )[ i ] != 0 )
      {
        return -1;
      }
    }
    
    entry++;
  }
  
  if ( entry != end )
  {
    return -1;
  }
  
  pack->buffer = buffer;
  return 0;
}

int rl_find_entry( rl_entry_t* entry, const rl_pack_t* pack, const char* name )
{
  const entry_t* tar_entry;
  const entry_t* tar_end;
  long size;
  
  tar_entry = (const entry_t*)pack->buffer;
  tar_end = (const entry_t*)( (const char*)pack->buffer + pack->buffer_len );

  while ( tar_entry < tar_end )
  {
    size = strtol( tar_entry->s.size, NULL, 8 );
    
    if ( !strcmp( tar_entry->s.name, name ) )
    {
      entry->contents = (void*)( tar_entry + 1 );
      entry->size = size;
      return 0;
    }
    
    tar_entry += ( size + 511 ) / 512 + 1;
  }
  
  return -1;
}
