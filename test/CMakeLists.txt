INCLUDE_DIRECTORIES(
    ${UTILS_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

LINK_DIRECTORIES(
    ${AUDIO_LIBRARY_DIRS}
)

ADD_EXECUTABLE(audiotest
    gmock-gtest-all.cpp
    main.cpp
)

TARGET_LINK_LIBRARIES(audiotest
    -lpthread
    utils
    ${AUDIO_LIBRARIES}
)

ADD_TEST(NAME AudioTests COMMAND audiotest)
