CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
ENABLE_TESTING()

OPTION(STANDALONE "Not used as a submodule of another project" OFF)
OPTION(TESTTOOLS "Build tools for testing" OFF)
OPTION(ENABLE_TESTS "build unit tests" ON)

ADD_DEFINITIONS("-D__STDC_CONSTANT_MACROS")

IF (STANDALONE)
    ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/../utils "${CMAKE_CURRENT_BINARY_DIR}/utils")
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    SET (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1")
    ADD_DEFINITIONS("-std=c++1y")
    IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND APPLE)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    ENDIF ()
ENDIF ()

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PULSEAUDIO_PKG libpulse)
PKG_CHECK_MODULES(FLAC_PKG flac)
PKG_CHECK_MODULES(FLACCPP_PKG flac++)
PKG_CHECK_MODULES(MAD_PKG mad)
PKG_CHECK_MODULES(AVCODEC_PKG libavcodec)
PKG_CHECK_MODULES(AVFORMAT_PKG libavformat)
PKG_CHECK_MODULES(AVUTIL_PKG libavutil)
PKG_CHECK_MODULES(SWRESAMPLE_PKG libswresample)
PKG_CHECK_MODULES(TAGLIB_PKG taglib)

FIND_PACKAGE(ALSA)
IF (ALSA_FOUND)
    OPTION(HAVE_ALSA "Alsa audio output support" ON)
ENDIF ()

FIND_PACKAGE(OpenAL)
IF (OPENAL_FOUND)
    OPTION(HAVE_OPENAL "OpenAL audio output support" ON)
ENDIF ()

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
ENDIF ()

IF (AVCODEC_PKG_FOUND AND AVFORMAT_PKG_FOUND AND AVUTIL_PKG_FOUND AND SWRESAMPLE_PKG_FOUND)
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

    FIND_PATH(SWRESAMPLE_INCLUDE_DIR
        NAMES libswresample/swresample.h
        PATHS ${SWRESAMPLE_PKG_INCLUDE_DIRS}
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

    FIND_LIBRARY(SWRESAMPLE_LIBRARY
        NAMES ${SWRESAMPLE_PKG_LIBRARIES}
        PATHS ${SWRESAMPLE_PKG_LIBRARY_DIRS}
    )

    SET(FFMPEG_INCLUDE_DIRS ${AVFORMAT_INCLUDE_DIR} ${AVCODEC_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${SWRESAMPLE_INCLUDE_DIR})
    SET(FFMPEG_LIBRARIES ${AVFORMAT_LIBRARY} ${AVCODEC_LIBRARY} ${AVUTIL_LIBRARY} ${SWRESAMPLE_LIBRARY})
ENDIF ()

IF (FLACCPP_PKG_FOUND)
    OPTION(HAVE_FLAC "Flac audio decoder support" ON)

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
ENDIF ()

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
ENDIF ()

IF (TAGLIB_PKG_FOUND)
    OPTION(HAVE_TAGLIB "Taglib audio metadata support" ON)
    FIND_PACKAGE(ZLIB REQUIRED)

    FIND_PATH(TAGLIB_INCLUDE_DIR
        NAMES taglib.h
        PATHS ${TAGLIB_PKG_INCLUDE_DIRS}
    )

    FIND_LIBRARY(TAGLIB_LIBRARY
        NAMES ${TAGLIB_PKG_LIBRARIES}
        PATHS ${TAGLIB_PKG_LIBRARY_DIRS}
    )
ENDIF ()

SET (AUDIO_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils/inc/
)

SET (AUDIO_SYS_INCLUDE_DIRS
    SYSTEM ${ALSA_INCLUDE_DIRS}
    SYSTEM ${PULSEAUDIO_INCLUDE_DIRS}
    SYSTEM ${MAD_INCLUDE_DIR}
    SYSTEM ${FFMPEG_INCLUDE_DIRS}
    SYSTEM ${FLAC_INCLUDE_DIRS}
    SYSTEM ${TAGLIB_INCLUDE_DIR}
)

IF (OPENAL_FOUND)
    LIST(APPEND AUDIO_INCLUDE_DIRS ${OPENAL_INCLUDE_DIR})
    LIST(APPEND AUDIO_LIBRARIES ${OPENAL_LIBRARY})
    IF (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
        LIST(APPEND AUDIO_LIBRARIES -lOpenSLES)
    ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        LIST(APPEND AUDIO_LIBRARIES -lrt)
    ENDIF ()

    IF (MINGW)
        LIST(APPEND AUDIO_LIBRARIES -lWinmm)
    ENDIF ()
ENDIF ()

SET (AUDIO_SRC_LIST
    src/audiobuffer.h                   src/audiobuffer.h
    inc/audio/audiodecoder.h
    inc/audio/audiodecoderfactory.h     src/audiodecoderfactory.cpp
    inc/audio/audioformat.h
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
ENDIF ()

IF (HAVE_OPENAL)
    LIST(APPEND AUDIO_SRC_LIST src/audioopenalrenderer.h src/audioopenalrenderer.cpp)
ENDIF ()

IF (HAVE_PULSE)
    LIST(APPEND AUDIO_SRC_LIST src/audiopulserenderer.h src/audiopulserenderer.cpp)
ENDIF ()

IF (HAVE_FFMPEG)
    LIST(APPEND AUDIO_SRC_LIST src/audioffmpegdecoder.h src/audioffmpegdecoder.cpp)
    LIST(APPEND AUDIO_SRC_LIST src/audioresampler.h src/audioresampler.cpp)
ENDIF ()

IF (HAVE_FLAC)
    LIST(APPEND AUDIO_SRC_LIST src/audioflacdecoder.h src/audioflacdecoder.cpp)
ENDIF ()

IF (HAVE_MAD)
    LIST(APPEND AUDIO_SRC_LIST src/audiomaddecoder.h src/audiomaddecoder.cpp)
ENDIF ()

IF (HAVE_TAGLIB)
    LIST(APPEND AUDIO_SRC_LIST inc/audio/audiometadata.h src/audiometadata.cpp
                               src/audiotaglibiostream.h src/audiotaglibiostream.cpp)
ENDIF ()

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR} ${AUDIO_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(SYSTEM ${AUDIO_SYS_INCLUDE_DIRS})
LINK_DIRECTORIES(${AUDIO_LIBRARY_DIRS})

ADD_LIBRARY(audio STATIC ${AUDIO_SRC_LIST})

TARGET_LINK_LIBRARIES(audio
    ${AUDIO_LIBRARIES}
    ${TAGLIB_LIBRARY}
    ${ALSA_LIBRARIES}
    ${PULSEAUDIO_LIBRARY}
    ${FFMPEG_LIBRARIES}
    ${FLACCPP_LIBRARY}
    ${FLAC_LIBRARIES}
    ${MAD_LIBRARY}
    ${ZLIB_LIBRARIES}
    utils
)

IF (UNIX OR APPLE)
    ADD_DEFINITIONS("-DCONSOLE_SUPPORTS_COLOR")
ENDIF ()

IF (MINGW OR WIN32)
    # needed to avoid __imp* linker errors when linking against static libraries
    ADD_DEFINITIONS("-DTAGLIB_STATIC -DAL_LIBTYPE_STATIC")
    TARGET_LINK_LIBRARIES(audio Mswsock Ws2_32)
ENDIF ()

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/audioconfig.h.in ${CMAKE_BINARY_DIR}/audioconfig.h)

IF (TESTTOOLS)
    ADD_SUBDIRECTORY(testtools)
ENDIF ()

IF (ENABLE_TESTS)
    ADD_SUBDIRECTORY(test)
ENDIF ()
