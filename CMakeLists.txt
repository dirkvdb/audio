cmake_minimum_required(VERSION 3.1)
enable_testing()

option(STANDALONE "Not used as a submodule of another project" OFF)
option(TESTTOOLS "Build tools for testing" OFF)
option(ENABLE_TESTS "build unit tests" ON)

add_definitions("-D__STDC_CONSTANT_MACROS")

if (STANDALONE)
    add_subdirectory(${CMAKE_SOURCE_DIR}/../utils "${CMAKE_CURRENT_BINARY_DIR}/utils")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1")
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND APPLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif ()
endif ()

find_package(PkgConfig)
pkg_check_modules(PULSEAUDIO_PKG libpulse)
pkg_check_modules(FLAC_PKG flac)
pkg_check_modules(FLACCPP_PKG flac++)
pkg_check_modules(MAD_PKG mad)
pkg_check_modules(AVCODEC_PKG libavcodec)
pkg_check_modules(AVFORMAT_PKG libavformat)
pkg_check_modules(AVUTIL_PKG libavutil)
pkg_check_modules(SWRESAMPLE_PKG libswresample)
pkg_check_modules(TAGLIB_PKG taglib)

find_package(ALSA)
if (ALSA_FOUND)
    option(HAVE_ALSA "Alsa audio output support" ON)
endif ()

find_package(OpenAL)
if (OPENAL_FOUND)
    option(HAVE_OPENAL "OpenAL audio output support" ON)
endif ()

if (PULSEAUDIO_PKG_FOUND)
    option(HAVE_PULSE "Pulse audio output support" ON)

    find_path(PULSEAUDIO_INCLUDE_DIR
        NAMES pulse/pulseaudio.h
        PATHS ${PULSEAUDIO_PKG_INCLUDE_DIRS}
    )

    find_library(PULSEAUDIO_LIBRARY
        NAMES ${PULSEAUDIO_PKG_LIBRARIES}
        PATHS ${PULSEAUDIO_PKG_LIBRARY_DIRS}
    )
endif ()

if (AVCODEC_PKG_FOUND AND AVFORMAT_PKG_FOUND AND AVUTIL_PKG_FOUND AND SWRESAMPLE_PKG_FOUND)
    option(HAVE_FFMPEG "FFmpeg audio decoder support" ON)

    find_path(AVCODEC_INCLUDE_DIR
        NAMES libavcodec/avcodec.h
        PATHS ${AVCODEC_PKG_INCLUDE_DIRS}
    )

    find_path(AVFORMAT_INCLUDE_DIR
        NAMES libavcodec/avcodec.h
        PATHS ${AVFORMAT_PKG_INCLUDE_DIRS}
    )

    find_path(AVUTIL_INCLUDE_DIR
        NAMES libavutil/avutil.h
        PATHS ${AVUTIL_PKG_INCLUDE_DIRS}
    )

    find_path(SWRESAMPLE_INCLUDE_DIR
        NAMES libswresample/swresample.h
        PATHS ${SWRESAMPLE_PKG_INCLUDE_DIRS}
    )

    find_library(AVCODEC_LIBRARY
        NAMES ${AVCODEC_PKG_LIBRARIES}
        PATHS ${AVCODEC_PKG_LIBRARY_DIRS}
    )

    find_library(AVFORMAT_LIBRARY
        NAMES ${AVFORMAT_PKG_LIBRARIES}
        PATHS ${AVFORMAT_PKG_LIBRARY_DIRS}
    )

    find_library(AVUTIL_LIBRARY
        NAMES ${AVUTIL_PKG_LIBRARIES}
        PATHS ${AVUTIL_PKG_LIBRARY_DIRS}
    )

    find_library(SWRESAMPLE_LIBRARY
        NAMES ${SWRESAMPLE_PKG_LIBRARIES}
        PATHS ${SWRESAMPLE_PKG_LIBRARY_DIRS}
    )

    set(FFMPEG_INCLUDE_DIRS ${AVFORMAT_INCLUDE_DIR} ${AVCODEC_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${SWRESAMPLE_INCLUDE_DIR})
    set(FFMPEG_LIBRARIES ${AVFORMAT_LIBRARY} ${AVCODEC_LIBRARY} ${AVUTIL_LIBRARY} ${SWRESAMPLE_LIBRARY})
endif ()

if (FLACCPP_PKG_FOUND)
    option(HAVE_FLAC "Flac audio decoder support" ON)

    find_path(FLACCPP_INCLUDE_DIR
        NAMES FLAC++/decoder.h
        PATHS ${FLACCPP_PKG_INCLUDE_DIRS}
    )

    find_path(FLAC_INCLUDE_DIR
        NAMES FLAC/all.h
        PATHS ${FLACCPP_PKG_INCLUDE_DIRS}
    )

    find_library(FLACCPP_LIBRARY
        NAMES ${FLACCPP_PKG_LIBRARIES}
        PATHS ${FLACCPP_PKG_LIBRARY_DIRS}
    )

    find_library(FLAC_LIBRARY
        NAMES ${FLAC_PKG_LIBRARIES}
        PATHS ${FLAC_PKG_LIBRARY_DIRS}
    )

    set(FLAC_INCLUDE_DIRS ${FLAC_INCLUDE_DIR} ${FLACCPP_INCLUDE_DIR})
    set(FLAC_LIBRARIES ${FLAC_LIBRARY} ${FLACCPP_LIBRARY})
endif ()

if (MAD_PKG_FOUND)
    option(HAVE_MAD "Mad mp3 audio decoder support" ON)

    find_path(MAD_INCLUDE_DIR
        NAMES mad.h
        PATHS ${MAD_PKG_INCLUDE_DIRS}
    )

    find_library(MAD_LIBRARY
        NAMES ${MAD_PKG_LIBRARIES}
        PATHS ${MAD_PKG_LIBRARY_DIRS}
    )
endif ()

if (TAGLIB_PKG_FOUND)
    option(HAVE_TAGLIB "Taglib audio metadata support" ON)
    find_package(ZLIB REQUIRED)

    find_path(TAGLIB_INCLUDE_DIR
        NAMES taglib.h
        PATHS ${TAGLIB_PKG_INCLUDE_DIRS}
        PATH_SUFFIXES taglib
    )

    find_library(TAGLIB_LIBRARY
        NAMES ${TAGLIB_PKG_LIBRARIES}
        PATHS ${TAGLIB_PKG_LIBRARY_DIRS}
    )
endif ()

if (OPENAL_FOUND)
    list(APPEND AUDIO_INCLUDE_DIRS ${OPENAL_INCLUDE_DIR} ${OPENAL_INCLUDE_DIR}/AL)
    list(APPEND AUDIO_LIBRARIES ${OPENAL_LIBRARY})
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
        list(APPEND AUDIO_LIBRARIES OpenSLES)
    ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        list(APPEND AUDIO_LIBRARIES rt)
    endif ()

    if (MINGW)
        list(APPEND AUDIO_LIBRARIES Winmm)
    endif ()
endif ()

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

    .travis.yml
)

if (HAVE_ALSA)
    list(APPEND AUDIO_SRC_LIST src/audioalsarenderer.h src/audioalsarenderer.cpp)
endif ()

if (HAVE_OPENAL)
    list(APPEND AUDIO_SRC_LIST src/audioopenalrenderer.h src/audioopenalrenderer.cpp)
endif ()

if (HAVE_PULSE)
    list(APPEND AUDIO_SRC_LIST src/audiopulserenderer.h src/audiopulserenderer.cpp)
endif ()

if (HAVE_FFMPEG)
    list(APPEND AUDIO_SRC_LIST src/audioffmpegdecoder.h src/audioffmpegdecoder.cpp)
    list(APPEND AUDIO_SRC_LIST src/audioresampler.h src/audioresampler.cpp)
endif ()

if (HAVE_FLAC)
    list(APPEND AUDIO_SRC_LIST src/audioflacdecoder.h src/audioflacdecoder.cpp)
endif ()

if (HAVE_MAD)
    list(APPEND AUDIO_SRC_LIST src/audiomaddecoder.h src/audiomaddecoder.cpp)
endif ()

if (HAVE_TAGLIB)
    list(APPEND AUDIO_SRC_LIST inc/audio/audiometadata.h src/audiometadata.cpp
                               src/audiotaglibiostream.h src/audiotaglibiostream.cpp)
endif ()

link_directories(${AUDIO_LIBRARY_DIRS})

add_library(audio STATIC ${AUDIO_SRC_LIST})

target_include_directories(audio PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
    $<INSTALL_INTERFACE:include>
    PRIVATE ${CMAKE_BINARY_DIR}
)

target_include_directories(audio SYSTEM PRIVATE
    ${ALSA_INCLUDE_DIRS}
    ${PULSEAUDIO_INCLUDE_DIRS}
    ${AUDIO_INCLUDE_DIRS}
    ${MAD_INCLUDE_DIR}
    ${FFMPEG_INCLUDE_DIRS}
    ${FLAC_INCLUDE_DIRS}
    ${TAGLIB_INCLUDE_DIR}
)

target_link_libraries(audio
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

if (UNIX OR APPLE)
    add_definitions("-DCONSOLE_SUPPORTS_COLOR")
endif ()

if (MINGW OR WIN32)
    # needed to avoid __imp* linker errors when linking against static libraries
    add_definitions("-DTAGLIB_STATIC -DAL_LIBTYPE_STATIC")
    target_link_libraries(audio Mswsock Ws2_32)
endif ()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/audioconfig.h.in ${CMAKE_BINARY_DIR}/audioconfig.h)

if (TESTTOOLS)
    add_subdirectory(testtools)
endif ()

if (ENABLE_TESTS)
    add_subdirectory(test)
endif ()
