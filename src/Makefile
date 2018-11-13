INCLUDES = -I. -Iphysfs/src -Isoloud/include

DEFINES = -DPHYSFS_SUPPORTS_ZIP=1 -DPHYSFS_SUPPORTS_7Z=0 -DPHYSFS_SUPPORTS_GRP=0 \
          -DPHYSFS_SUPPORTS_HOG=0 -DPHYSFS_SUPPORTS_MVL=0 -DPHYSFS_SUPPORTS_WAD=0 \
          -DPHYSFS_SUPPORTS_QPAK=0 -DPHYSFS_SUPPORTS_SLB=0 -DPHYSFS_SUPPORTS_ISO9660=0 \
          -DPHYSFS_SUPPORTS_VDF=0 \
          -DWITH_NULL

CFLAGS = -Wall -fPIC $(INCLUDES) $(DEFINES)

ifneq ($(DEBUG),)
  CFLAGS += -O0 -g
else
  CFLAGS += -O3 -DNDEBUG
endif

CXXFLAGS = $(CFLAGS)

OBJS = rl_backgrnd.o rl_base64.o rl_bdffont.o rl_config.o rl_image.o rl_pixelsrc.o rl_pack.o rl_rand.o \
       rl_sound.o rl_sprite.o rl_utf8.o rl_version.o rl_xml.o \
       physfs/src/physfs_archiver_7z.o physfs/src/physfs_archiver_dir.o physfs/src/physfs_archiver_grp.o \
       physfs/src/physfs_archiver_hog.o physfs/src/physfs_archiver_iso9660.o physfs/src/physfs_archiver_mvl.o \
       physfs/src/physfs_archiver_qpak.o physfs/src/physfs_archiver_slb.o physfs/src/physfs_archiver_unpacked.o \
       physfs/src/physfs_archiver_vdf.o physfs/src/physfs_archiver_wad.o physfs/src/physfs_archiver_zip.o \
       physfs/src/physfs_byteorder.o physfs/src/physfs.o physfs/src/physfs_platform_os2.o \
       physfs/src/physfs_platform_posix.o physfs/src/physfs_platform_qnx.o physfs/src/physfs_platform_unix.o \
       physfs/src/physfs_platform_windows.o physfs/src/physfs_unicode.o \
       soloud/src/audiosource/sfxr/soloud_sfxr.o soloud/src/audiosource/speech/soloud_speech.o \
       soloud/src/audiosource/speech/darray.o soloud/src/audiosource/speech/klatt.o \
       soloud/src/audiosource/speech/resonator.o soloud/src/audiosource/speech/tts.o \
       soloud/src/audiosource/vizsn/soloud_vizsn.o soloud/src/audiosource/wav/soloud_wav.o \
       soloud/src/audiosource/wav/soloud_wavstream.o soloud/src/audiosource/wav/stb_vorbis.o \
       soloud/src/audiosource/wav/dr_impl.o soloud/src/backend/null/soloud_null.o \
       soloud/src/core/soloud_audiosource.o soloud/src/core/soloud_bus.o soloud/src/core/soloud_core_3d.o \
       soloud/src/core/soloud_core_basicops.o soloud/src/core/soloud_core_faderops.o \
       soloud/src/core/soloud_core_filterops.o soloud/src/core/soloud_core_getters.o \
       soloud/src/core/soloud_core_setters.o soloud/src/core/soloud_core_voicegroup.o \
       soloud/src/core/soloud_core_voiceops.o soloud/src/core/soloud.o soloud/src/core/soloud_fader.o \
       soloud/src/core/soloud_fft.o soloud/src/core/soloud_fft_lut.o soloud/src/core/soloud_file.o \
       soloud/src/core/soloud_filter.o soloud/src/core/soloud_queue.o soloud/src/core/soloud_thread.o \
       soloud/src/filter/soloud_bassboostfilter.o soloud/src/filter/soloud_biquadresonantfilter.o \
       soloud/src/filter/soloud_dcremovalfilter.o soloud/src/filter/soloud_echofilter.o \
       soloud/src/filter/soloud_fftfilter.o soloud/src/filter/soloud_flangerfilter.o \
       soloud/src/filter/soloud_lofifilter.o soloud/src/filter/soloud_robotizefilter.o \
       soloud/src/filter/soloud_waveshaperfilter.o

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
