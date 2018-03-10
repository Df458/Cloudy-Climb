project('cloudy', 'c', 'vala',
        version : '0.0.1',
        license : 'MIT',
        default_options : ['c_std=gnu11'])

compiler = meson.get_compiler('c')

m = compiler.find_library('m', required : false)

gnome = import('gnome')

dfgame = subproject('dfgame')
glfw = subproject('glfw')
glew = subproject('glew')

core = dfgame.get_variable('core')
math = dfgame.get_variable('math')
graphics = dfgame.get_variable('graphics')
audio = dfgame.get_variable('audio')
application = dfgame.get_variable('application_game')
editorbak = dfgame.get_variable('application_editor_c')
editor = dfgame.get_variable('application_editor_vala')
gameplay = dfgame.get_variable('gameplay')
resource = dfgame.get_variable('resource')
#core = dependency('dfgame-core')
#math = dependency('dfgame-math')
#graphics = dependency('dfgame-graphics')
#audio = dependency('dfgame-audio')
#application = dependency('dfgame-application-game')
#editorbak = dependency('dfgame-application-editor-backend')
#editor = dependency('dfgame-application-editor')
#gameplay = dependency('dfgame-gameplay')
#resource = dependency('dfgame-resource')
gtk = dependency('gtk+-3.0')

args = [ '-Wl,-rpath', '.' ]
subdir('src')
subdir('src/editor')

run_command('ctags', '-R', '.')
run_command('ln', '-rs', 'assets', 'build/src')
run_command('ln', '-rs', 'assets', 'build/src/editor')
