deps = [ core, graphics, math, resource, audio, gameplay ]
src  = [ 
  'audiomanager.c',
  'bonus.c',
  'cloud.c', 
  'entity.c', 
  'game_controls.c', 
  'gameloop.c',
  'goal.c',
  'io.c',
  'menu.c',
  'player.c', 
  'rail.c',
  'renderer.c',
  'spike.c',
]
inc  = []
e_lib  = static_library('cloudy-ed-bak', src + [ 'editor/editor.c' ],
                       include_directories : inc,
                       dependencies : deps + [ editorbak ],
                       c_args : [ '-D BUILD_EDITOR' ],
                       link_args : args,
                       install : false)
g_lib  = static_library('cloudy-bak', src,
                       include_directories : inc,
                       dependencies : deps + [ application ],
                       c_args : [ '-D BUILD_GAME' ],
                       link_args : args,
                       install : false)

exe  = executable('cloudy', 'main.c',
                       include_directories : inc,
                       dependencies : deps + [ application ],
                       link_with : g_lib,
                       link_args : args,
                       install : false)

run_command('ctags', '-R', '.')
