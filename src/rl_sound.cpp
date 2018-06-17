#include <rl_sound.h>

#include <soloud.h>
#include <soloud_file.h>
#include <soloud_wav.h>
#include <soloud_wavstream.h>
//#include <soloud_openmpt.h>
#include <soloud_speech.h>
#include <soloud_sfxr.h>
#include <soloud_vizsn.h>

#include <physfs.h>

static SoLoud::Soloud soloud;
static int16_t audio_buffer[ RL_SAMPLES_PER_FRAME * 2 ];

class PhysicsFsFile : public SoLoud::File
{
public:
  PhysicsFsFile() = default;
  
  bool init( const char* path )
  {
    _file = PHYSFS_openRead( path );
    return _file != NULL;
  }

  virtual ~PhysicsFsFile()
  {
    if ( _file != NULL )
    {
      PHYSFS_close( _file );
      _file = NULL;
    }
  }

  virtual int eof() override
  {
    return PHYSFS_eof( _file );
  }

  virtual unsigned int read( unsigned char* aDst, unsigned int aBytes ) override
  {
    PHYSFS_sint64 num_read = PHYSFS_readBytes( _file, aDst, aBytes );
    return num_read != -1 ? num_read : 0;
  }

  virtual unsigned int length() override
  {
    PHYSFS_sint64 size = PHYSFS_fileLength( _file );
    return size != -1 ? size : 0;
  }

  virtual void seek( int aOffset ) override
  {
    if ( aOffset < 0 )
    {
      aOffset += length();
    }

    PHYSFS_seek( _file, aOffset );
  }

  virtual unsigned int pos() override
  {
    PHYSFS_sint64 pos = PHYSFS_tell( _file );
    return pos != -1 ? pos : 0;
  }

private:
  PHYSFS_File* _file;
};

void rl_sound_init( void )
{
  soloud.init( SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::NULLDRIVER, RL_SAMPLE_RATE, sizeof( audio_buffer ) );
}

void rl_sound_done( void )
{
  soloud.deinit();
}

/* Load an WAV, OGG, or an Open ModPlug Tracker file. */
int rl_sound_wav( rl_sound_t* sound, const char* path )
{
  PhysicsFsFile file;

  if ( !file.init( path ) )
  {
    return -1;
  }

  auto source = new SoLoud::Wav;
  
  if ( source->loadFile( &file ) != 0 )
  {
    delete source;
    return -1;
  }

  sound->opaque1 = source;
  sound->opaque2 = NULL;
  return 0;
}

int rl_sound_ogg( rl_sound_t* sound, const char* path )
{
  auto file = new PhysicsFsFile;

  if ( !file->init( path ) )
  {
    delete file;
    return -1;
  }

  auto source = new SoLoud::WavStream;

  if ( source->loadFile( file ) != 0 )
  {
    delete source;
    delete file;
    return -1;
  }

  sound->opaque1 = source;
  sound->opaque2 = file;
  return 0;
}

int rl_sound_mod( rl_sound_t* sound, const char* path )
{
  /*PhysicsFsFile file;

  if ( !file.init( path ) )
  {
    return -1;
  }

  auto source = new SoLoud::Openmpt;

  if ( source->loadFile( &file ) != 0 )
  {
    delete source;
    return -1;
  }

  sound->opaque1 = source;
  sound->opaque2 = NULL;
  return 0;*/
  return -1;
}

int rl_sound_speech( rl_sound_t* sound, const char* text )
{
  rl_sound_speech_t params;
  params.base_frequency = 1330;
  params.base_speed = 10.0f;
  params.base_declination = 0.5f;
  params.waveform =  RL_SOUND_SPEECH_TRIANGLE;

  return rl_sound_speech_params( sound, text, &params );
}

int rl_sound_speech_params( rl_sound_t* sound, const char* text, const rl_sound_speech_t* params )
{
  auto source = new SoLoud::Speech;

  if ( source->setParams( params->base_frequency, params->base_speed, params->base_declination, params->waveform ) != 0 )
  {
    delete source;
    return -1;
  }

  if ( source->setText( text ) != 0 )
  {
    delete source;
    return -1;
  }

  sound->opaque1 = source;
  sound->opaque2 = NULL;
  return 0;
}

int rl_sound_sfxr_load( rl_sound_t* sound, const char* path )
{
  PhysicsFsFile file;

  if ( !file.init( path ) )
  {
    return -1;
  }

  auto source = new SoLoud::Sfxr;

  if ( source->loadParamsFile( &file ) != 0 )
  {
    delete source;
    return -1;
  }

  sound->opaque1 = source;
  sound->opaque2 = NULL;
  return 0;
}

int rl_sound_sfxr( rl_sound_t* sound, int type, int seed )
{
  auto source = new SoLoud::Sfxr;

  if ( source->loadPreset( type, seed ) != 0 )
  {
    delete source;
    return -1;
  }

  sound->opaque1 = source;
  sound->opaque2 = NULL;
  return 0;
}

int rl_sound_vizsn( rl_sound_t* sound, const char* text )
{
  auto source = new SoLoud::Vizsn;
  source->setText( (char*)text );
  sound->opaque1 = source;
  sound->opaque2 = NULL;
  return 0;
}

void rl_sound_destroy( const rl_sound_t* sound )
{
  auto source = (SoLoud::AudioSource*)sound->opaque1;
  source->stop();
  delete source;

  if ( sound->opaque2 != NULL )
  {
    auto file = (SoLoud::File*)sound->opaque2;
    delete file;
  }
}

unsigned rl_sound_play( const rl_sound_t* sound, float volume, int repeat )
{
  auto source = (SoLoud::AudioSource*)sound->opaque1;
  unsigned voice = soloud.play( *source, volume, 0.0f, true, 0 );

  if ( repeat )
  {
    soloud.setLooping( voice, true );
  }

  soloud.setPause( voice, false );
  return voice;
}

void rl_sound_stop( unsigned voice )
{
  soloud.stop( voice );
}

void rl_sound_stop_all( void )
{
  soloud.stopAll();
}

const int16_t* rl_sound_mix( void )
{
  soloud.mixSigned16( audio_buffer, RL_SAMPLES_PER_FRAME );
  return audio_buffer;
}
