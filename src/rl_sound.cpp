#include <rl_sound.h>

#include <soloud.h>
#include <soloud_file.h>
#include <soloud_wav.h>
#include <soloud_wavstream.h>
//#include <soloud_openmpt.h>
#include <soloud_speech.h>
#include <soloud_sfxr.h>
#include <soloud_vizsn.h>

static SoLoud::Soloud soloud;
static int16_t audio_buffer[ RL_SAMPLES_PER_FRAME * 2 ];

class PhysicsFsFile : public SoLoud::File
{
public:
  PhysicsFsFile() = default;
  
  bool init( const char* path )
  {
    return inited = rl_pack_open( &stream, path, 0 ) == 0;
  }

  virtual ~PhysicsFsFile()
  {
    if ( inited )
    {
      rl_pack_close( &stream );
    }
  }

  virtual int eof() override
  {
    return rl_pack_eof( &stream );
  }

  virtual unsigned int read( unsigned char* aDst, unsigned int aBytes ) override
  {
    if ( rl_pack_read( &stream, aDst, &aBytes ) != 0 )
    {
      return 0;
    }

    return aBytes;
  }

  virtual unsigned int length() override
  {
    unsigned bytes;

    if ( rl_pack_size( &stream, &bytes ) != 0 )
    {
      return 0;
    }

    return bytes;
  }

  virtual void seek( int aOffset ) override
  {
    long offset = aOffset;

    if ( offset < 0 )
    {
      offset += length();
    }

    rl_pack_seek( &stream, offset );
  }

  virtual unsigned int pos() override
  {
    unsigned pos;

    if ( rl_pack_tell( &stream, &pos ) != 0 )
    {
      return 0;
    }

    return pos;
  }

private:
  bool inited;
  rl_stream_t stream;
};

void rl_sound_init( void )
{
  soloud.init( SoLoud::Soloud::CLIP_ROUNDOFF, SoLoud::Soloud::NULLDRIVER, RL_SAMPLE_RATE );
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

unsigned rl_sound_play( const rl_sound_t* sound, int repeat )
{
  auto source = (SoLoud::AudioSource*)sound->opaque1;
  unsigned voice = soloud.play( *source, 1.0f, 0.0f, true, 0 );

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
