FLAGS_PHYSFS = \
  -DPHYSFS_SUPPORTS_ZIP=1 -DPHYSFS_SUPPORTS_7Z=0 -DPHYSFS_SUPPORTS_GRP=0 \
  -DPHYSFS_SUPPORTS_HOG=0 -DPHYSFS_SUPPORTS_MVL=0 -DPHYSFS_SUPPORTS_WAD=0 \
  -DPHYSFS_SUPPORTS_QPAK=0 -DPHYSFS_SUPPORTS_SLB=0 \
  -DPHYSFS_SUPPORTS_ISO9660=0 -DPHYSFS_SUPPORTS_VDF=0 -Iphysfs/src

FLAGS_SOLOUD = -DWITH_NULL -Isoloud/include

FLAGS_LIBOPENMPT = \
  -Iopenmpt/common -Iopenmpt -Iopenmpt/include/modplug/include \
  -Iopenmpt/include -DMPT_WITH_ZLIB -Ibuild/svn_version \
  -DMPT_SVNURL=\"https://source.openmpt.org/svn/openmpt/branches/OpenMPT-1.28\" \
  -DMPT_SVNVERSION=\"11826\" -DMPT_SVNDATE=\"2019-07-23T16:39:50Z\" \
  -DLIBOPENMPT_BUILD -Iopenmpt/build/svn_version

CFLAGS = -Wall -fPIC -I. $(FLAGS_PHYSFS) $(FLAGS_SOLOUD) $(FLAGS_LIBOPENMPT)

ifneq ($(DEBUG),)
  CFLAGS += -O0 -g
else
  CFLAGS += -O3 -DNDEBUG
endif

CXXFLAGS = $(CFLAGS)

OBJS = \
  rl_backgrnd.o \
  rl_base64.o \
  rl_bdffont.o \
  rl_config.o \
  rl_image.o \
  rl_pixelsrc.o \
  rl_pack.o \
  rl_rand.o \
  rl_sound.o \
  rl_sprite.o \
  rl_utf8.o \
  rl_version.o \
  rl_xml.o \
  physfs/src/physfs_archiver_7z.o \
  physfs/src/physfs_archiver_dir.o \
  physfs/src/physfs_archiver_grp.o \
  physfs/src/physfs_archiver_hog.o \
  physfs/src/physfs_archiver_iso9660.o \
  physfs/src/physfs_archiver_mvl.o \
  physfs/src/physfs_archiver_qpak.o \
  physfs/src/physfs_archiver_slb.o \
  physfs/src/physfs_archiver_unpacked.o \
  physfs/src/physfs_archiver_vdf.o \
  physfs/src/physfs_archiver_wad.o \
  physfs/src/physfs_archiver_zip.o \
  physfs/src/physfs_byteorder.o \
  physfs/src/physfs.o \
  physfs/src/physfs_platform_os2.o \
  physfs/src/physfs_platform_posix.o \
  physfs/src/physfs_platform_qnx.o \
  physfs/src/physfs_platform_unix.o \
  physfs/src/physfs_platform_windows.o \
  physfs/src/physfs_unicode.o \
  soloud/src/audiosource/openmpt/soloud_openmpt.o \
  soloud/src/audiosource/sfxr/soloud_sfxr.o \
  soloud/src/audiosource/speech/soloud_speech.o \
  soloud/src/audiosource/speech/darray.o \
  soloud/src/audiosource/speech/klatt.o \
  soloud/src/audiosource/speech/resonator.o \
  soloud/src/audiosource/speech/tts.o \
  soloud/src/audiosource/vizsn/soloud_vizsn.o \
  soloud/src/audiosource/wav/soloud_wav.o \
  soloud/src/audiosource/wav/soloud_wavstream.o \
  soloud/src/audiosource/wav/stb_vorbis.o \
  soloud/src/audiosource/wav/dr_impl.o \
  soloud/src/backend/null/soloud_null.o \
  soloud/src/core/soloud_audiosource.o \
  soloud/src/core/soloud_bus.o \
  soloud/src/core/soloud_core_3d.o \
  soloud/src/core/soloud_core_basicops.o \
  soloud/src/core/soloud_core_faderops.o \
  soloud/src/core/soloud_core_filterops.o \
  soloud/src/core/soloud_core_getters.o \
  soloud/src/core/soloud_core_setters.o \
  soloud/src/core/soloud_core_voicegroup.o \
  soloud/src/core/soloud_core_voiceops.o \
  soloud/src/core/soloud.o soloud/src/core/soloud_fader.o \
  soloud/src/core/soloud_fft.o \
  soloud/src/core/soloud_fft_lut.o \
  soloud/src/core/soloud_file.o \
  soloud/src/core/soloud_filter.o \
  soloud/src/core/soloud_queue.o \
  soloud/src/core/soloud_thread.o \
  soloud/src/filter/soloud_bassboostfilter.o \
  soloud/src/filter/soloud_biquadresonantfilter.o \
  soloud/src/filter/soloud_dcremovalfilter.o \
  soloud/src/filter/soloud_echofilter.o \
  soloud/src/filter/soloud_fftfilter.o \
  soloud/src/filter/soloud_flangerfilter.o \
  soloud/src/filter/soloud_lofifilter.o \
  soloud/src/filter/soloud_robotizefilter.o \
  soloud/src/filter/soloud_waveshaperfilter.o \
  openmpt/common/ComponentManager.o \
  openmpt/common/FileReader.o \
  openmpt/common/Logging.o \
  openmpt/common/Profiler.o \
  openmpt/common/misc_util.o \
  openmpt/common/mptAlloc.o \
  openmpt/common/mptCPU.o \
  openmpt/common/mptFileIO.o \
  openmpt/common/mptIO.o \
  openmpt/common/mptLibrary.o \
  openmpt/common/mptOS.o \
  openmpt/common/mptPathString.o \
  openmpt/common/mptRandom.o \
  openmpt/common/mptString.o \
  openmpt/common/mptStringBuffer.o \
  openmpt/common/mptStringFormat.o \
  openmpt/common/mptStringParse.o \
  openmpt/common/mptTime.o \
  openmpt/common/mptUUID.o \
  openmpt/common/mptWine.o \
  openmpt/common/serialization_utils.o \
  openmpt/common/version.o \
  openmpt/soundlib/AudioCriticalSection.o \
  openmpt/soundlib/ContainerMMCMP.o \
  openmpt/soundlib/ContainerPP20.o \
  openmpt/soundlib/ContainerUMX.o \
  openmpt/soundlib/ContainerXPK.o \
  openmpt/soundlib/Dither.o \
  openmpt/soundlib/Dlsbank.o \
  openmpt/soundlib/Fastmix.o \
  openmpt/soundlib/ITCompression.o \
  openmpt/soundlib/ITTools.o \
  openmpt/soundlib/InstrumentExtensions.o \
  openmpt/soundlib/Load_669.o \
  openmpt/soundlib/Load_amf.o \
  openmpt/soundlib/Load_ams.o \
  openmpt/soundlib/Load_c67.o \
  openmpt/soundlib/Load_dbm.o \
  openmpt/soundlib/Load_digi.o \
  openmpt/soundlib/Load_dmf.o \
  openmpt/soundlib/Load_dsm.o \
  openmpt/soundlib/Load_dtm.o \
  openmpt/soundlib/Load_far.o \
  openmpt/soundlib/Load_gdm.o \
  openmpt/soundlib/Load_imf.o \
  openmpt/soundlib/Load_it.o \
  openmpt/soundlib/Load_itp.o \
  openmpt/soundlib/Load_mdl.o \
  openmpt/soundlib/Load_med.o \
  openmpt/soundlib/Load_mid.o \
  openmpt/soundlib/Load_mo3.o \
  openmpt/soundlib/Load_mod.o \
  openmpt/soundlib/Load_mt2.o \
  openmpt/soundlib/Load_mtm.o \
  openmpt/soundlib/Load_okt.o \
  openmpt/soundlib/Load_plm.o \
  openmpt/soundlib/Load_psm.o \
  openmpt/soundlib/Load_ptm.o \
  openmpt/soundlib/Load_s3m.o \
  openmpt/soundlib/Load_sfx.o \
  openmpt/soundlib/Load_stm.o \
  openmpt/soundlib/Load_stp.o \
  openmpt/soundlib/Load_uax.o \
  openmpt/soundlib/Load_ult.o \
  openmpt/soundlib/Load_wav.o \
  openmpt/soundlib/Load_xm.o \
  openmpt/soundlib/MIDIEvents.o \
  openmpt/soundlib/MIDIMacros.o \
  openmpt/soundlib/MPEGFrame.o \
  openmpt/soundlib/Message.o \
  openmpt/soundlib/MixFuncTable.o \
  openmpt/soundlib/MixerLoops.o \
  openmpt/soundlib/MixerSettings.o \
  openmpt/soundlib/ModChannel.o \
  openmpt/soundlib/ModInstrument.o \
  openmpt/soundlib/ModSample.o \
  openmpt/soundlib/ModSequence.o \
  openmpt/soundlib/OPL.o \
  openmpt/soundlib/OggStream.o \
  openmpt/soundlib/Paula.o \
  openmpt/soundlib/RowVisitor.o \
  openmpt/soundlib/S3MTools.o \
  openmpt/soundlib/SampleFormatFLAC.o \
  openmpt/soundlib/SampleFormatMP3.o \
  openmpt/soundlib/SampleFormatMediaFoundation.o \
  openmpt/soundlib/SampleFormatOpus.o \
  openmpt/soundlib/SampleFormatVorbis.o \
  openmpt/soundlib/SampleFormats.o \
  openmpt/soundlib/SampleIO.o \
  openmpt/soundlib/Snd_flt.o \
  openmpt/soundlib/Snd_fx.o \
  openmpt/soundlib/Sndfile.o \
  openmpt/soundlib/Sndmix.o \
  openmpt/soundlib/SoundFilePlayConfig.o \
  openmpt/soundlib/Tables.o \
  openmpt/soundlib/Tagging.o \
  openmpt/soundlib/UMXTools.o \
  openmpt/soundlib/UpgradeModule.o \
  openmpt/soundlib/WAVTools.o \
  openmpt/soundlib/WindowedFIR.o \
  openmpt/soundlib/XMTools.o \
  openmpt/soundlib/load_j2b.o \
  openmpt/soundlib/mod_specifications.o \
  openmpt/soundlib/modcommand.o \
  openmpt/soundlib/modsmp_ctrl.o \
  openmpt/soundlib/pattern.o \
  openmpt/soundlib/patternContainer.o \
  openmpt/soundlib/tuning.o \
  openmpt/soundlib/tuningCollection.o \
  openmpt/soundlib/tuningbase.o \
  openmpt/soundlib/plugins/DigiBoosterEcho.o \
  openmpt/soundlib/plugins/LFOPlugin.o \
  openmpt/soundlib/plugins/PlugInterface.o \
  openmpt/soundlib/plugins/PluginManager.o \
  openmpt/soundlib/plugins/dmo/Chorus.o \
  openmpt/soundlib/plugins/dmo/Compressor.o \
  openmpt/soundlib/plugins/dmo/DMOPlugin.o \
  openmpt/soundlib/plugins/dmo/Distortion.o \
  openmpt/soundlib/plugins/dmo/Echo.o \
  openmpt/soundlib/plugins/dmo/Flanger.o \
  openmpt/soundlib/plugins/dmo/Gargle.o \
  openmpt/soundlib/plugins/dmo/I3DL2Reverb.o \
  openmpt/soundlib/plugins/dmo/ParamEq.o \
  openmpt/soundlib/plugins/dmo/WavesReverb.o \
  openmpt/sounddsp/AGC.o \
  openmpt/sounddsp/DSP.o \
  openmpt/sounddsp/EQ.o \
  openmpt/sounddsp/Reverb.o \
  openmpt/libopenmpt/libopenmpt_c.o \
  openmpt/libopenmpt/libopenmpt_cxx.o \
  openmpt/libopenmpt/libopenmpt_impl.o \
  openmpt/libopenmpt/libopenmpt_ext_impl.o \
  openmpt/include/minimp3/minimp3.o

%.o: %.c
	gcc $(CFLAGS) -std=c99 -c $< -o $@

%.o: %.cpp
	g++ $(CXXFLAGS) -std=c++11 -c $< -o $@

all: libretroluxury.a

libretroluxury.a: $(OBJS)
	ar rcs $@ $+

rl_version.c: FORCE
	cat ../etc/version.c.templ | sed s/HASH/`git rev-parse HEAD | tr -d "\n"`/g > $@

clean:
	rm -f libretroluxury.a $(OBJS)

.PHONY: clean FORCE
