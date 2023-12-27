project('fpsunlock', 'c', version: '1.0.2')

sources = [
    'src/main.c',
    'src/game.c',
    'src/msg.c',
    'src/bytes.c'
]

include_dir = include_directories('include')

conf_data = configuration_data()
conf_data.set('version', meson.project_version())

conf = configure_file(input: 'include/config.h.in', output: 'config.h', configuration: conf_data)

executable(
    'fpsunlock',
    sources,
    conf,
    include_directories: include_dir,
    win_subsystem: 'windows'
)
