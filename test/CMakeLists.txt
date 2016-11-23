find_package(Threads)

add_executable(audiotest
    gmock-gtest-all.cpp
    main.cpp
)

target_include_directories(audiotest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(audiotest
    utils
    audio
    ${CMAKE_THREAD_LIBS_INIT}
)

add_test(NAME AudioTests COMMAND audiotest)
