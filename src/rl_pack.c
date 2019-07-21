#include <rl_pack.h>

#include <stdlib.h>
#include <string.h>

#include <physfs.h>

int rl_pack_init( const char* arg0, const char* organization, const char* app_name )
{
  return PHYSFS_init( arg0 ) && PHYSFS_setSaneConfig( organization, app_name, NULL, 0, 0 ) ? 0 : -1;
}

void rl_pack_done( void )
{
  PHYSFS_deinit();
}

int rl_pack_add( const char* path )
{
  return PHYSFS_mount( path, NULL, 1) ? 0 : -1;
}

int rl_pack_size( const char* path, size_t* size )
{
  PHYSFS_file* file = PHYSFS_openRead( path );

  if ( file == NULL )
  {
    return -1;
  }

  PHYSFS_sint64 length = PHYSFS_fileLength( file );

  if ( length == -1 )
  {
    PHYSFS_close( file );
    return -1;
  }

  PHYSFS_close( file );
  *size = (size_t)length;
  return 0;
}

int rl_pack_read( const char* path, void* buffer )
{
  PHYSFS_file* file = PHYSFS_openRead( path );

  if ( file == NULL )
  {
    return -1;
  }

  PHYSFS_sint64 length = PHYSFS_fileLength( file );

  if ( length == -1 )
  {
    return -1;
  }

  PHYSFS_sint64 num_read = PHYSFS_readBytes( file, buffer, length );
  PHYSFS_close( file );

  if ( num_read != length )
  {
    return -1;
  }

  return 0;
}
