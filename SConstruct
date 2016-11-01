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
    env.Replace(CC = 'clang')

env['BUILDERS']['ctags'] = \
    SCons.Builder.Builder(action='ctags --tag-relative=yes -f $TARGET $SOURCES')

# --=[ Configuration ]=--------------------------------------------------------

env.Append(CCFLAGS = '-g -O2 -Wall -Wextra')
env.Append(LINKFLAGS = '-g')

conf = Configure(env)

env['HAVE_STRNDUP'] = conf.CheckFunc('strndup')
env['HAVE_STRLCPY'] = conf.CheckFunc('strlcpy')
env['HAVE_RANDOM'] = conf.CheckFunc('random')

dependencies = [('m', 'math.h', None),
                ('SDL2', 'SDL.h', 'sdl2'),
                ('SDL2_ttf', 'SDL_ttf.h', 'SDL2_ttf'),
                ('ffi', 'ffi.h', 'libffi'),
                ('png', 'png.h', 'libpng')]

if platform.system().startswith('MINGW'):
    dependencies += [('edit', 'editline/readline.h', None)]
else:
    dependencies += [('edit', 'editline/readline.h', 'libedit')]

conf.CheckProg('pkg-config')

for lib, header, pkg in dependencies:
    if pkg:
        env.ParseConfig('pkg-config --cflags ' + pkg)
        env.ParseConfig('pkg-config --libs ' + pkg)

    if not conf.CheckLib(lib) or not conf.CheckCHeader(header):
        Exit(1)

env = conf.Finish()

# --=[ Compilation ]=----------------------------------------------------------

env.AlwaysBuild(env.Command('config.h', 'config.h.in', config_h_build))

SConscript('libtexproma/SConscript', exports='env')

ctag_sources = [Glob('*.[ch]'), Glob('libtexproma/*.[ch]')]
Alias('tags', env.ctags(source=ctag_sources, target='tags'))

env.Prepend(LIBS = ['texproma'])
env.Prepend(LIBPATH = ['libtexproma'])
env.Prepend(CPPPATH = ['libtexproma'])

sources = ['main.c', 'cell.c', 'fn.c', 'word.c', 'dict.c', 'array.c',
           'builtin.c', 'gui.c', 'interp.c']

if not env['HAVE_STRNDUP']:
    sources += ['extra/strndup.c']
if not env['HAVE_RANDOM']:
    sources += ['extra/random.c']

prog = env.Program(target='texproma', source=sources)
Clean(prog, ['texproma.dSYM', Glob('*~')])
