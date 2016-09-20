#!python

import os

env = Environment(ENV = os.environ)
env.Append(CCFLAGS = '-g -O2 -Wall')
env.Append(LINKFLAGS = '-g')

env.ParseConfig('pkg-config --cflags --libs sdl2')
env.ParseConfig('pkg-config --cflags --libs libedit')

env.Append(LIBS = ['texproma'])
env.Append(LIBPATH = ['libtexproma'])
env.Append(CPPPATH = ['libtexproma'])

env.Append(LIBS = ['avcall'])
env.Append(LIBPATH = ['libffcall/avcall/.libs'])
env.Append(CPPPATH = ['libffcall/avcall'])

SConscript('libtexproma/SConscript')

sources = ['main.c', 'cell.c', 'dict.c', 'gui.c', 'interp.c']
t = env.Program(target='texproma', source=sources)
Clean(t, 'texproma.dSYM')
