#!python

import SCons.Builder
import SCons.Action

env = Environment()
env['ENV']['PATH'] = ['/opt/local/bin', '/usr/bin', '/bin']

env['BUILDERS']['ctags'] = \
    SCons.Builder.Builder(action='ctags --tag-relative=yes -f $TARGET $SOURCES')

env.Append(CCFLAGS = '-g -O2 -Wall')
env.Append(LINKFLAGS = '-g')

env.ParseConfig('pkg-config --cflags --libs sdl2 SDL2_ttf')
env.ParseConfig('pkg-config --cflags --libs libedit')

env.Append(LIBS = ['texproma'])
env.Append(LIBPATH = ['libtexproma'])
env.Append(CPPPATH = ['libtexproma'])

env.Append(LIBS = ['avcall'])
env.Append(LIBPATH = ['libffcall/avcall/.libs'])
env.Append(CPPPATH = ['libffcall/avcall'])

SConscript('libtexproma/SConscript')

ctag_sources = [Glob('*.[ch]'), Glob('libtexproma/*.[ch]')]
sources = ['main.c', 'cell.c', 'dict.c', 'gui.c', 'interp.c']

Alias('tags', env.ctags(source=ctag_sources, target='tags'))

prog = env.Program(target='texproma', source=sources)
Clean(prog, ['texproma.dSYM', Glob('*~')])
