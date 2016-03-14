audiotestfiles = files(
    'gmock-gtest-all.cpp',
    'main.cpp',
)

testinc = include_directories(meson.current_build_dir() + '/..')

audiotest = executable('audiotest',
                       audiotestfiles,
                       include_directories : [audioinc, testinc],
                       dependencies : audio_dep)

test('audio test', audiotest)