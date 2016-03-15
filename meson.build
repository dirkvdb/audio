project('audio', 'cpp', default_options : ['cpp_std=c++14'])

utilssub = subproject('utils')

incs = ['inc']
libs = []

pulse_dep = dependency('libpulse', required : false)
alsa_dep = dependency('alsalib', required : false)
flac_dep = dependency('flac', required : false)
flacpp_dep = dependency('flac++', required : false)
mad_dep = dependency('mad', required : false)
avcodec_dep = dependency('libavcodec', required : false)
avformat_dep = dependency('libavformat', required : false)
avutil_dep = dependency('libavutil', required : false)
swresample_dep = dependency('libswresample', required : false)
taglib_dep = dependency('taglib', required : false)
openal_dep = dependency('openal', required : false)

# try to find openal on OSX (no pkg-config support)
if not openal_dep.found() and host_machine.system() == 'darwin'
    message('Searching for openal-soft on OSX')
    openal_dep = find_library('openal', dirs : '/usr/local/opt/openal-soft/lib', required : false)
    if openal_dep.found()
        incs += '/usr/local/opt/openal-soft/include/AL'
    endif
endif

audiofiles = files(
    'src/audiobuffer.h',                   'src/audiobuffer.h',
    'inc/audio/audiodecoder.h',
    'inc/audio/audiodecoderfactory.h',     'src/audiodecoderfactory.cpp',
    'inc/audio/audioformat.h',
    'inc/audio/audioframe.h',              'src/audioframe.cpp',
    'inc/audio/audiompegutils.h',          'src/audiompegutils.cpp',
    'inc/audio/audioplaybackinterface.h',
    'inc/audio/audioplaybackfactory.h',    'src/audioplaybackfactory.cpp',
    'src/audioplayback.h',                 'src/audioplayback.cpp',
    'inc/audio/audioplaylistinterface.h',
    'inc/audio/audiorenderer.h',
    'inc/audio/audiorendererfactory.h',    'src/audiorendererfactory.cpp',
    'inc/audio/audiotrackinterface.h',
    'src/audiobuffer.h',                   'src/audiobuffer.cpp',
    'inc/audio/audiom3uparser.h',          'src/audiom3uparser.cpp'
)

config = configuration_data()
config.set10('HAVE_OPENAL', openal_dep.found())
config.set10('HAVE_PULSE', pulse_dep.found())
config.set10('HAVE_ALSA', alsa_dep.found())
config.set10('HAVE_MAD', mad_dep.found())
config.set10('HAVE_FLAC', flac_dep.found() and flacpp_dep.found())
config.set10('HAVE_FFMPEG', avcodec_dep.found() and avformat_dep.found() and avutil_dep.found() and swresample_dep.found())
config.set10('HAVE_TAGLIB', taglib_dep.found())

if openal_dep.found()
    audiofiles += files('src/audioopenalrenderer.h', 'src/audioopenalrenderer.cpp')
endif

if pulse_dep.found()
    audiofiles += files('src/audiopulserenderer.h', 'src/audiopulserenderer.cpp')
endif

if alsa_dep.found()
    audiofiles += files('src/audioalsarenderer.h', 'src/audioalsarenderer.cpp')
endif

if mad_dep.found()
    audiofiles += files('src/audiomaddecoder.h', 'src/audiomaddecoder.cpp')
endif

if flac_dep.found() and flacpp_dep.found()
    config.set('HAVE_FLAC', true)
    audiofiles += files('src/audioflacdecoder.h', 'src/audioflacdecoder.cpp')
endif

if avcodec_dep.found() and avformat_dep.found() and avutil_dep.found() and swresample_dep.found()
    audiofiles += files('src/audioffmpegdecoder.h', 'src/audioffmpegdecoder.cpp',
                        'src/audioresampler.h', 'src/audioresampler.cpp')
endif

if taglib_dep.found()
    audiofiles += files('inc/audio/audiometadata.h', 'src/audiometadata.cpp',
                        'src/audiotaglibiostream.h', 'src/audiotaglibiostream.cpp')
endif

configure_file(input : 'audioconfigmeson.h.in', output : 'audioconfig.h', configuration : config)
audioinc = include_directories(incs)

audiolib = static_library('audio',
                          audiofiles,
                          include_directories : audioinc,
                          link_with : libs,
                          dependencies : [openal_dep, alsa_dep, pulse_dep, flac_dep, flacpp_dep, mad_dep, avcodec_dep, avformat_dep, swresample_dep, avutil_dep, taglib_dep, utilssub.get_variable('utils_dep')])

audio_dep = declare_dependency(link_with : audiolib, include_directories : audioinc)

subdir('test')
