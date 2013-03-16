CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

FIND_PACKAGE(PkgConfig)

FIND_PACKAGE(ALSA)
IF (ALSA_FOUND)
    OPTION(HAVE_ALSA "Alsa audio output support" ON)
ENDIF (ALSA_FOUND)

FIND_PACKAGE(OpenAL)
IF (OPENAL_FOUND)
    OPTION(HAVE_OPENAL "OpenAL audio output support" ON)
ENDIF (OPENAL_FOUND)

pkg_check_modules(PULSEAUDIO libpulse)
IF (PULSEAUDIO_FOUND)
    OPTION(HAVE_PULSE "Pulse audio output support" ON)
ENDIF (PULSEAUDIO_FOUND)

SET (AUDIO_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/inc/"
    "${CMAKE_CURRENT_SOURCE_DIR}/../utils/inc/"
    ${OPENAL_INCLUDE_DIR} ${ALSA_INCLUDE_DIRS} ${PULSEAUDIO_INCLUDE_DIRS}
)
SET (AUDIO_LIBRARY_DIRS
    "${CMAKE_CURRENT_BINARY_DIR}"
)
SET (AUDIO_LIBRARIES audio ${OPENAL_LIBRARY} ${ALSA_LIBRARIES} ${PULSEAUDIO_LIBRARIES} ${UTILS_LIBRARIES})

SET (AUDIO_SRC_LIST
    inc/audio/audioformat.h
    inc/audio/audioframe.h              src/audioframe.cpp
    inc/audio/audiorendererfactory.h    src/audiorendererfactory.cpp
    src/audiobuffer.h                   src/audiobuffer.cpp
)

IF (HAVE_ALSA)
    LIST(APPEND AUDIO_SRC_LIST src/alsarenderer.h src/alsarenderer.cpp)
ENDIF (HAVE_ALSA)

IF (HAVE_OPENAL)
    LIST(APPEND AUDIO_SRC_LIST src/openalrenderer.h src/openalrenderer.cpp)
ENDIF (HAVE_OPENAL)

IF (HAVE_PULSE)
    LIST(APPEND AUDIO_SRC_LIST src/pulserenderer.h src/pulserenderer.cpp)
ENDIF (HAVE_PULSE)

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
