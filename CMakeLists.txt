CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_DEFINITIONS("-D__STDC_CONSTANT_MACROS")

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PULSEAUDIO libpulse)
PKG_CHECK_MODULES(FLAC flac++)
PKG_CHECK_MODULES(MAD mad)
PKG_CHECK_MODULES(AVCODEC libavcodec)
PKG_CHECK_MODULES(AVFORMAT libavformat)
PKG_CHECK_MODULES(AVUTIL libavutil)
PKG_CHECK_MODULES(UPNP libupnp)

FIND_PACKAGE(ALSA)
IF (ALSA_FOUND)
    OPTION(HAVE_ALSA "Alsa audio output support" ON)
ENDIF (ALSA_FOUND)

FIND_PACKAGE(OpenAL)
IF (OPENAL_FOUND)
    OPTION(HAVE_OPENAL "OpenAL audio output support" ON)
ENDIF (OPENAL_FOUND)

IF (PULSEAUDIO_FOUND)
    OPTION(HAVE_PULSE "Pulse audio output support" ON)
ENDIF (PULSEAUDIO_FOUND)

IF (UPNP_FOUND)
    OPTION(HAVE_UPNP "UPnP support" ON)
ENDIF (UPNP_FOUND)

IF (AVCODEC_FOUND AND AVFORMAT_FOUND AND AVUTIL_FOUND)
    OPTION(HAVE_FFMPEG "FFmpeg audio decoder support" ON)
    SET(FFMPEG_INCLUDE_DIRS ${AVCODEC_INCLUDE_DIRS} ${AVFORMAT_INCLUDE_DIRS} ${AVUTIL_INCLUDE_DIRS})
    SET(FFMPEG_LIBRARY_DIRS ${AVCODEC_LIBRARY_DIRS} ${AVFORMAT_LIBRARY_DIRS} ${AVUTIL_LIBRARY_DIRS})
    SET(FFMPEG_LIBRARIES ${AVCODEC_LIBRARIES} ${AVFORMAT_LIBRARIES} ${AVUTIL_LIBRARIES})
ENDIF (AVCODEC_FOUND AND AVFORMAT_FOUND AND AVUTIL_FOUND)

IF (FLAC_FOUND)
    OPTION(HAVE_FLAC "Flac audio decoder support" ON)
ENDIF (FLAC_FOUND)

IF (MAD_FOUND)
    OPTION(HAVE_MAD "Mad mp3 audio decoder support" ON)
ENDIF (MAD_FOUND)

SET (AUDIO_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/"
    "${CMAKE_CURRENT_SOURCE_DIR}/../utils/inc/"
    ${OPENAL_INCLUDE_DIR}
    ${ALSA_INCLUDE_DIRS}
    ${PULSEAUDIO_INCLUDE_DIRS}
    ${MAD_INCLUDE_DIRS}
    ${FFMPEG_INCLUDE_DIRS}
    ${UPNP_INCLUDE_DIRS}
)
SET (AUDIO_LIBRARY_DIRS
    "${CMAKE_CURRENT_BINARY_DIR}"
)
SET (AUDIO_LIBRARIES audio
    ${OPENAL_LIBRARY}
    ${ALSA_LIBRARIES}
    ${PULSEAUDIO_LIBRARIES}
    ${FFMPEG_LIBRARIES}
    ${FLAC_LIBRARIES}
    ${MAD_LIBRARIES}
    ${UTILS_LIBRARIES}
    ${UPNP_LIBRARIES}
)

SET (AUDIO_SRC_LIST
    src/audiobuffer.h                   src/audiobuffer.h
    inc/audio/audiodecoder.h
    inc/audio/audiodecoderfactory.h     src/audiodecoderfactory.cpp
    inc/audio/audiofilereader.h         src/audiofilereader.cpp
    inc/audio/audioformat.h
    inc/audio/audioframe.h              src/audioframe.cpp
    inc/audio/audiompegutils.h          src/audiompegutils.cpp
    inc/audio/audioplaybackinterface.h
    inc/audio/audioplaybackfactory.h    src/audioplaybackfactory.cpp
    src/audioplayback.h                 src/audioplayback.cpp
    inc/audio/audioplaylistinterface.h
    inc/audio/audioreaderfactory.h      src/audioreaderfactory.cpp
    inc/audio/audiorenderer.h
    inc/audio/audiorendererfactory.h    src/audiorendererfactory.cpp
    src/audiobuffer.h                   src/audiobuffer.cpp
    inc/audio/audiom3uparser.h          src/audiom3uparser.cpp
)

IF (HAVE_ALSA)
    LIST(APPEND AUDIO_SRC_LIST src/audioalsarenderer.h src/audioalsarenderer.cpp)
ENDIF (HAVE_ALSA)

IF (HAVE_OPENAL)
    LIST(APPEND AUDIO_SRC_LIST src/audioopenalrenderer.h src/audioopenalrenderer.cpp)
ENDIF (HAVE_OPENAL)

IF (HAVE_PULSE)
    LIST(APPEND AUDIO_SRC_LIST src/audiopulserenderer.h src/audiopulserenderer.cpp)
ENDIF (HAVE_PULSE)

IF (HAVE_FFMPEG)
    LIST(APPEND AUDIO_SRC_LIST src/audioffmpegdecoder.h src/audioffmpegdecoder.cpp)
ENDIF (HAVE_FFMPEG)

IF (HAVE_FLAC)
    LIST(APPEND AUDIO_SRC_LIST src/audioflacdecoder.h src/audioflacdecoder.cpp)
ENDIF (HAVE_FLAC)

IF (HAVE_MAD)
    LIST(APPEND AUDIO_SRC_LIST src/audiomaddecoder.h src/audiomaddecoder.cpp)
ENDIF (HAVE_MAD)

IF (HAVE_UPNP)
    LIST(APPEND AUDIO_SRC_LIST src/audiohttpreader.h src/audiohttpreader.cpp src/audiohttpget.h src/audiohttpget.cpp)
ENDIF (HAVE_UPNP)

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR} ${AUDIO_INCLUDE_DIRS})
LINK_DIRECTORIES(${AUDIO_LIBRARY_DIRS})

ADD_LIBRARY(audio STATIC ${AUDIO_SRC_LIST})

ADD_DEFINITIONS("-std=c++11")
IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    SET(CMAKE_CXX_FLAGS "-stdlib=libc++")
ENDIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

IF (UNIX OR APPLE)
    ADD_DEFINITIONS("-DCONSOLE_SUPPORTS_COLOR")
ENDIF (UNIX OR APPLE)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/audioconfig.h.in ${CMAKE_BINARY_DIR}/audioconfig.h)
