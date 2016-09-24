#!python

import SCons.Builder
import SCons.Action

env = Environment()
env['ENV']['PATH'] = ['/opt/local/bin', '/usr/bin', '/bin']

env['BUILDERS']['ctags'] = \
    SCons.Builder.Builder(action='ctags --tag-relative=yes -f $TARGET $SOURCES')

env.Append(CCFLAGS = '-g -O2 -Wall')
env.Append(LINKFLAGS = '-g')

for lib in ['sdl2', 'SDL2_ttf', 'libffi', 'libpng']:
    env.ParseConfig('pkg-config --cflags --libs ' + lib)

conf = Configure(env)
if not conf.CheckCHeader('SDL.h'):
    print 'SDL2 must be installed!'
    Exit(1)
if not conf.CheckCHeader('SDL_ttf.h'):
    print 'SDL2_ttf must be installed!'
    Exit(1)
if not conf.CheckCHeader('png.h'):
    print 'libpng must be installed!'
    Exit(1)
if not conf.CheckCHeader('ffi.h'):
    print 'libpng must be installed!'
    Exit(1)
env = conf.Finish()

env.Append(LIBS = ['texproma'])
env.Append(LIBPATH = ['libtexproma'])
env.Append(CPPPATH = ['libtexproma', 'linenoise'])

SConscript('libtexproma/SConscript')

ctag_sources = [Glob('*.[ch]'), Glob('libtexproma/*.[ch]')]
sources = ['main.c', 'cell.c', 'dict.c', 'gui.c', 'interp.c',
           'linenoise/linenoise.c']

Alias('tags', env.ctags(source=ctag_sources, target='tags'))

prog = env.Program(target='texproma', source=sources)
Clean(prog, ['texproma.dSYM', Glob('*~')])
