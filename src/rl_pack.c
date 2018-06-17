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
