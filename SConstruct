#!/usr/bin/env python
# Author: Bryan Cain
# Date: March 12, 2011

import os, sys, platform
from scons import wxconfig

def error(msg):
	sys.stderr.write('Error: %s\n' % msg)
	sys.exit(1)

env = Environment(ENV = os.environ)
conf = Configure(env, custom_tests = {'CheckWXConfig': wxconfig.CheckWXConfig})

# check for SDL
if not conf.CheckLib('SDL'):
	error('SDL not found')
if not conf.CheckLib('SDL_mixer'):
	error('SDL_mixer not found')
env.ParseConfig('sdl-config --cflags --libs')
env['LIBS'] += ['SDL_mixer']

# check for wxWidgets 2.8.9
if not conf.CheckWXConfig(2.89, ['adv', 'core', 'base']):
	error('wxWidgets (>= 2.8.9) not found')
wxconfig.ParseWXConfig(env)

# check for libgif
if not conf.CheckLib('gif'):
	error('libgif not found')
env['LIBS'] += ['gif']

# set compile flags
env['CCFLAGS'] += ['-g', '-O2', '-DPACKAGE_VERSION=\\"1.5\\"', '-DPACKAGE_STRING=\\"obeditor\\ 1.5\\"']
if env['PLATFORM'] == 'win32':
	env['CCFLAGS'] += ['-DOSWINDOW']
elif env['PLATFORM'] == 'posix':
	env['CCFLAGS'] += ['-DOSLINUX']
else:
	error('unknown platform')

# make the output prettier
env['CCCOMSTR'] = 'Compiling $SOURCE'
env['CXXCOMSTR'] = 'Compiling $SOURCE'
env['LINKCOMSTR'] = 'Linking $TARGET'

# build
env = conf.Finish()
Export('env')
SConscript('src/SConscript')

# move to "release" directory
os.system('rm -rf release')
os.mkdir('release')
env.Command('release/obeditor', 'src/obeditor', [Move('release/obeditor', 'src/obeditor')])
env.Command('release/resources', 'resources', [Copy('release/resources', 'resources')])


