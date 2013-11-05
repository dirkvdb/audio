CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
ENABLE_TESTING()

ADD_DEFINITIONS("-D__STDC_CONSTANT_MACROS")

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PULSEAUDIO_PKG libpulse)
PKG_CHECK_MODULES(FLAC_PKG flac)
PKG_CHECK_MODULES(FLACCPP_PKG flac++)
PKG_CHECK_MODULES(MAD_PKG mad)
PKG_CHECK_MODULES(AVCODEC_PKG libavcodec)
PKG_CHECK_MODULES(AVFORMAT_PKG libavformat)
PKG_CHECK_MODULES(AVUTIL_PKG libavutil)
PKG_CHECK_MODULES(TAGLIB_PKG taglib)

FIND_PACKAGE(ALSA)
IF (ALSA_FOUND)
    OPTION(HAVE_ALSA "Alsa audio output support" ON)
ENDIF (ALSA_FOUND)

FIND_PACKAGE(OpenAL)
IF (OPENAL_FOUND)
    OPTION(HAVE_OPENAL "OpenAL audio output support" ON)
ENDIF (OPENAL_FOUND)

IF (PULSEAUDIO_PKG_FOUND)
    OPTION(HAVE_PULSE "Pulse audio output support" ON)
    
    FIND_PATH(PULSEAUDIO_INCLUDE_DIR
        NAMES pulse/pulseaudio.h
        PATHS ${PULSEAUDIO_PKG_INCLUDE_DIRS}
    )

    FIND_LIBRARY(PULSEAUDIO_LIBRARY
        NAMES ${PULSEAUDIO_PKG_LIBRARIES}
        PATHS ${PULSEAUDIO_PKG_LIBRARY_DIRS}
    )
ENDIF (PULSEAUDIO_PKG_FOUND)

IF (AVCODEC_PKG_FOUND AND AVFORMAT_PKG_FOUND AND AVUTIL_PKG_FOUND)
    OPTION(HAVE_FFMPEG "FFmpeg audio decoder support" ON)
    
    FIND_PATH(AVCODEC_INCLUDE_DIR
        NAMES libavcodec/avcodec.h
        PATHS ${AVCODEC_PKG_INCLUDE_DIRS}
    )

    FIND_PATH(AVFORMAT_INCLUDE_DIR
        NAMES libavcodec/avcodec.h
        PATHS ${AVFORMAT_PKG_INCLUDE_DIRS}
    )

    FIND_PATH(AVUTIL_INCLUDE_DIR
        NAMES libavutil/avutil.h
        PATHS ${AVUTIL_PKG_INCLUDE_DIRS}
    )
    
    FIND_LIBRARY(AVCODEC_LIBRARY
        NAMES ${AVCODEC_PKG_LIBRARIES}
        PATHS ${AVCODEC_PKG_LIBRARY_DIRS}
    )

    FIND_LIBRARY(AVFORMAT_LIBRARY
        NAMES ${AVFORMAT_PKG_LIBRARIES}
        PATHS ${AVFORMAT_PKG_LIBRARY_DIRS}
    )
    
    FIND_LIBRARY(AVUTIL_LIBRARY
        NAMES ${AVUTIL_PKG_LIBRARIES}
        PATHS ${AVUTIL_PKG_LIBRARY_DIRS}
    )

    SET(FFMPEG_INCLUDE_DIRS ${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR})
    SET(FFMPEG_LIBRARIES ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY})
ENDIF (AVCODEC_PKG_FOUND AND AVFORMAT_PKG_FOUND AND AVUTIL_PKG_FOUND)

IF (FLACCPP_PKG_FOUND)
    OPTION(HAVE_FLACCPP_PKG "Flac audio decoder support" ON)
    
    FIND_PATH(FLACCPP_INCLUDE_DIR
        NAMES FLAC++/decoder.h
        PATHS ${FLACCPP_PKG_INCLUDE_DIRS}
    )
    
    FIND_PATH(FLAC_INCLUDE_DIR
        NAMES FLAC/all.h
        PATHS ${FLACCPP_PKG_INCLUDE_DIRS}
    )
    
    FIND_LIBRARY(FLACCPP_LIBRARY
        NAMES ${FLACCPP_PKG_LIBRARIES}
        PATHS ${FLACCPP_PKG_LIBRARY_DIRS}
    )
    
    FIND_LIBRARY(FLAC_LIBRARY
        NAMES ${FLAC_PKG_LIBRARIES}
        PATHS ${FLAC_PKG_LIBRARY_DIRS}
    )
    
    SET(FLAC_INCLUDE_DIRS ${FLAC_INCLUDE_DIR} ${FLACCPP_INCLUDE_DIR})
    SET(FLAC_LIBRARIES ${FLAC_LIBRARY} ${FLACCPP_LIBRARY})
ENDIF (FLACCPP_PKG_FOUND)

IF (MAD_PKG_FOUND)
    OPTION(HAVE_MAD "Mad mp3 audio decoder support" ON)
    
    FIND_PATH(MAD_INCLUDE_DIR
        NAMES mad.h
        PATHS ${MAD_PKG_INCLUDE_DIRS}
    )

    FIND_LIBRARY(MAD_LIBRARY
        NAMES ${MAD_PKG_LIBRARIES}
        PATHS ${MAD_PKG_LIBRARY_DIRS}
    )
ENDIF (MAD_PKG_FOUND)

IF (TAGLIB_PKG_FOUND)
    OPTION(HAVE_TAGLIB "Taglib audio metadata support" ON)
    
    FIND_PATH(TAGLIB_INCLUDE_DIR
        NAMES taglib/taglib.h
        PATHS ${MAD_PKG_INCLUDE_DIRS}
    )

    FIND_LIBRARY(TAGLIB_LIBRARY
        NAMES ${TAGLIB_PKG_LIBRARIES}
        PATHS ${TAGLIB_PKG_LIBRARY_DIRS}
    )
ENDIF (TAGLIB_PKG_FOUND)

SET (AUDIO_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/"
    "${CMAKE_CURRENT_SOURCE_DIR}/../utils/inc/"
    ${ALSA_INCLUDE_DIRS}
    ${PULSEAUDIO_INCLUDE_DIRS}
    ${MAD_INCLUDE_DIR}
    ${FFMPEG_INCLUDE_DIRS}
    ${FLAC_INCLUDE_DIRS}
    ${TAGLIB_INCLUDE_DIR}
)

SET (AUDIO_LIBRARY_DIRS
    "${CMAKE_CURRENT_BINARY_DIR}"
)

SET (AUDIO_LIBRARIES audio
    ${ALSA_LIBRARIES}
    ${PULSEAUDIO_LIBRARY}
    ${FFMPEG_LIBRARIES}
    ${FLACCPP_LIBRARY}
    ${FLAC_LIBRARIES}
    ${MAD_LIBRARY}
    ${UTILS_LIBRARIES}
    ${TAGLIB_LIBRARY}
)

IF (HAVE_OPENAL)
    LIST(APPEND AUDIO_INCLUDE_DIRS ${OPENAL_INCLUDE_DIR})
    LIST(APPEND AUDIO_LIBRARIES ${OPENAL_LIBRARY})
ENDIF (HAVE_OPENAL)

SET (AUDIO_SRC_LIST
    src/audiobuffer.h                   src/audiobuffer.h
    inc/audio/audiodecoder.h
    inc/audio/audiodecoderfactory.h     src/audiodecoderfactory.cpp
    inc/audio/audioformat.h
    inc/audio/audioflacheaders.h
    inc/audio/audioframe.h              src/audioframe.cpp
    inc/audio/audiompegutils.h          src/audiompegutils.cpp
    inc/audio/audioplaybackinterface.h
    inc/audio/audioplaybackfactory.h    src/audioplaybackfactory.cpp
    src/audioplayback.h                 src/audioplayback.cpp
    inc/audio/audioplaylistinterface.h
    inc/audio/audiorenderer.h
    inc/audio/audiorendererfactory.h    src/audiorendererfactory.cpp
    inc/audio/audiotrackinterface.h
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

IF (HAVE_TAGLIB)
    LIST(APPEND AUDIO_SRC_LIST inc/audio/audiometadata.h src/audiometadata.cpp
                               src/audiotaglibiostream.h src/audiotaglibiostream.cpp)
ENDIF (HAVE_TAGLIB)

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR} ${AUDIO_INCLUDE_DIRS})
LINK_DIRECTORIES(${AUDIO_LIBRARY_DIRS})

ADD_LIBRARY(audio STATIC ${AUDIO_SRC_LIST})

ADD_DEFINITIONS("-std=c++11")
IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    SET(CMAKE_CXX_FLAGS "-stdlib=libc++")
ENDIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Gcc")
    SET(CMAKE_CXX_FLAGS_DEBUG "-rdynamic ")
    SET(CMAKE_CXX_FLAGS "-stdlib=libstdc++")
ENDIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Gcc")

SET (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -Wall -Wfatal-errors")

IF (UNIX OR APPLE)
    ADD_DEFINITIONS("-DCONSOLE_SUPPORTS_COLOR")
ENDIF (UNIX OR APPLE)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/audioconfig.h.in ${CMAKE_BINARY_DIR}/audioconfig.h)

ADD_SUBDIRECTORY(testtools)

ADD_SUBDIRECTORY(test)
