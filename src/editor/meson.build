deps = [ core, application, graphics, math, resource, audio, gameplay ]
src  = [ 'main.vala', 'EditorApp.vala', 'MainWindow.vala', 'CloudyGame.vapi' ]
res = gnome.compile_resources('resources2', 'editor.gresource.xml', source_dir : 'src')
exe  = executable('cloudy-editor', src + res,
                       include_directories : inc + [ include_directories('..') ],
                       dependencies : [ gtk, editor ] + deps,
                       link_with : [ e_lib ],
                       vala_args : ['--target-glib=2.38'],
                       link_args : args,
                       install : false)

run_command('ctags', '-R', '.')
