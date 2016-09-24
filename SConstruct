#!python

import platform
import SCons.Builder
import SCons.Action


def config_h_build(target, source, env):
    config_h_defines = {}

    for key, value in env.Dictionary().items():
        if key.startswith('HAVE_'):
            config_h_defines[key] = value

    for a_target, a_source in zip(target, source):
        with open(str(a_source), 'r') as c_in:
            with open(str(a_target), 'w') as c_out:
                for key, value in config_h_defines.items():
                    print >>c_out, ['#undef', '#define'][int(value)], key
                print >>c_out, ''
                c_out.write(c_in.read())


env = Environment()

if platform.system() == 'Darwin':
    env['ENV']['PATH'] = ['/opt/local/bin', '/usr/bin', '/bin']

env['BUILDERS']['ctags'] = \
    SCons.Builder.Builder(action='ctags --tag-relative=yes -f $TARGET $SOURCES')

env.Append(CCFLAGS = '-g -O2 -Wall')
env.Append(LINKFLAGS = '-g')

sources = ['main.c', 'cell.c', 'dict.c', 'gui.c', 'interp.c']

conf = Configure(env)

env['HAVE_STRNDUP'] = conf.CheckFunc('strndup')
env['HAVE_RANDOM'] = conf.CheckFunc('random')

dependencies = [('m', 'math.h', None),
                ('SDL2', 'SDL.h', 'sdl2'),
                ('SDL2_ttf', 'SDL_ttf.h', 'SDL2_ttf'),
                ('ffi', 'ffi.h', 'libffi'),
                ('png', 'png.h', 'libpng'),
                ('edit', 'histedit.h', 'libedit')]

conf.CheckProg('pkg-config')

for lib, header, pkg in dependencies:
    if pkg:
        env.ParseConfig('pkg-config --cflags --libs ' + pkg)

    if not conf.CheckLib(lib) or not conf.CheckCHeader(header):
        Exit(1)

env = conf.Finish()

env.AlwaysBuild(env.Command('config.h', 'config.h.in', config_h_build))

env.Append(LIBS = ['texproma'])
env.Append(LIBPATH = ['libtexproma'])
env.Append(CPPPATH = ['libtexproma', 'linenoise'])

if not env['HAVE_STRNDUP']:
    sources += ['extra/strndup.c']
if not env['HAVE_RANDOM']:
    sources += ['extra/random.c']

SConscript('libtexproma/SConscript')

ctag_sources = [Glob('*.[ch]'), Glob('libtexproma/*.[ch]')]
sources += ['linenoise/linenoise.c']

Alias('tags', env.ctags(source=ctag_sources, target='tags'))

prog = env.Program(target='texproma', source=sources)
Clean(prog, ['texproma.dSYM', Glob('*~')])
