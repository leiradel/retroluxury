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

int rl_pack_open( rl_stream_t* stream, const char* path, int write )
{
  if ( write )
  {
    stream->opaque = (void*)PHYSFS_openWrite( path );
  }
  else
  {
    stream->opaque = (void*)PHYSFS_openRead( path );
  }

  return stream->opaque != NULL ? 0 : -1;
}

int rl_pack_read( rl_stream_t* stream, void* buffer, unsigned* bytes )
{
  PHYSFS_sint64 num_read = PHYSFS_readBytes( (PHYSFS_File*)stream->opaque, buffer, *bytes );
  *bytes = num_read;
  return num_read != -1 ? 0 : -1;
}

int rl_pack_write( rl_stream_t* stream, const void* buffer, unsigned bytes )
{
  PHYSFS_sint64 num_written = PHYSFS_writeBytes( (PHYSFS_File*)stream->opaque, buffer, bytes );
  return num_written == bytes ? 0 : -1;
}

int rl_pack_seek( rl_stream_t* stream, unsigned offset )
{
  return PHYSFS_seek( (PHYSFS_File*)stream->opaque, offset ) != 0 ? 0 : -1;
}

int rl_pack_tell( rl_stream_t* stream, unsigned* pos )
{
  PHYSFS_sint64 p = PHYSFS_tell( (PHYSFS_File*)stream->opaque );
  *pos = p;
  return p != -1 ? 0 : -1;
}

int rl_pack_size( rl_stream_t* stream, unsigned* bytes )
{
  PHYSFS_sint64 size = PHYSFS_fileLength( (PHYSFS_File*)stream->opaque );
  *bytes = size;
  return size != -1 ? 0 : -1;
}

int rl_pack_eof( rl_stream_t* stream )
{
  return PHYSFS_eof( (PHYSFS_File*)stream->opaque );
}

void rl_pack_close( rl_stream_t* stream )
{
  PHYSFS_close( (PHYSFS_File*)stream->opaque );
}
