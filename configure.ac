#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([obeditor], [1.5], [culbuto66@ymail.com])
AC_CONFIG_SRCDIR([src/])
# AC_CONFIG_HEADER([config.h])

AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_AWK
AC_PROG_MKDIR_P
if test "x$host_alias" != x; then
    cross_compiling=yes
fi
echo "cross compiling = $cross_compiling"


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

dnl--------------------------------------------------
dnl m4 stuffs to include
m4_include(aclocal/wxwin.m4)

#--------------------------------------------------
AC_ARG_ENABLE(debug,[
Specific arguments
  --enable-debug  turn on debugging flags]
            , OBDEBUG=yes, OBDEBUG=no )

#--------------------------------------------------
AC_ARG_ENABLE(mtrace,[--enable-mtrace  turn on tracing mem on flags]
            , MTRACE=yes, MTRACE=no )


#--------------------------------------------------
#vérifier le comportement de malloc (en particulier, taille nulle)
# AC_FUNC_MALLOC


#--------------------------------------------------
if test "$OBDEBUG" = "yes"; then
	if test "${cross_compiling}" == "no"; then
		AC_SUBST(DEBUGGS_FLAGS, " -DMYDEBUG -g -O0 " )
	else
		AC_SUBST(DEBUGGS_FLAGS, " -DMYDEBUG -g -O0 " )
	fi
	echo "debugging on"
else
	if test "${cross_compiling}" == "no"; then
		AC_SUBST(DEBUGGS_FLAGS, " -O3 " )
	else
		AC_SUBST(DEBUGGS_FLAGS, " -O2 " )
	fi
	echo "debugging off"
fi

#--------------------------------------------------
if test "$MTRACE" = "yes"; then
	AC_SUBST(MTRACE_C_FLAGS, " -DMTRACE " )
	AC_SUBST(MTRACE_LD_FLAGS, "  " )
fi

#--------------------------------------------------
# Checks for library SDL
m4_include(aclocal/sdl.m4)
AM_OPTIONSSDL
AM_PATH_SDL([1.2.0], [],
	AC_MSG_ERROR([[
		SDL must be installed on your system.
 		Please check that sdl-config is in path, the directory
		where sdl libraries are installed (returned by
		'sdl-config --libs' command)
		is in LD_LIBRARY_PATH or equivalent variable and
		wxWidgets version is $reqwx or above.
		]]) )

#--------------------------------------------------
# Checks for library SDL_mixer
AC_CHECK_LIB([SDL_mixer], [Mix_CloseAudio], [[echo SDL_mixer seems to be installed]]
		,AC_MSG_ERROR([[
			Unable to find the library SDL_mixer, the program
			cannot compile without.
			]],
			"-lSDL"))

AC_SUBST([SDL_MIXER_LIB], [-lSDL_mixer] )

#--------------------------------------------------
# Checks for library wxwidgets
AM_OPTIONS_WXCONFIG
reqwx=2.8.9
if test $OBDEBUG = "yes" && test ${cross_compiling} = "no"; then
	AC_SUBST( [AM_WXCONFIG_ARG_MORE],[" --debug=yes --static "] )
else
	AC_SUBST( [AM_WXCONFIG_ARG_MORE],[" --debug=no --static "] )
fi
	
AM_PATH_WXCONFIG($reqwx, wxWin=1, "", $AM_WXCONFIG_ARG_MORE)
if test "$wxWin" != 1; then
	AC_MSG_ERROR([[
		wxWidgets must be installed on your system.
 
		Please check that wx-config is in path, the directory
		where wxWidgets libraries are installed (returned by
		'wx-config --libs' or 'wx-config --static --libs' command)
		is in LD_LIBRARY_PATH or equivalent variable and
		wxWidgets version is $reqwx or above.
		]])
fi
 


#--------------------------------------------------
# CPPFLAGS="$DEBUGGS_FLAGS $MTRACE_C_FLAGS $WX_CPPFLAGS $SDL_CFLAGS "
# CFLAGS="$DEBUGGS_FLAGS $MTRACE_C_FLAGS $WX_CFLAGS_ONLY $SDL_CFLAGS"

echo 
echo "build OS = $build"
echo "host  OS = $host"
echo

case $host in
	*mingw32*)
		CFLAG_OS_SYS=" -DOSWINDOW "
		GIFLIB=" -lgif "
		echo "compilation for window"
		echo
	;;
	
	*linux*)
		CFLAG_OS_SYS="-DOSLINUX "
		GIFLIB=" -lgif "
		echo "compilation for linux"
		echo
	;;
	
	*)
		AC_MSG_ERROR([[
		Unknow host system, cannot build on it.
		]])
	;;
esac
		
echo "prefix = ${prefix}"
CFLAGS=" -Wall $CFLAGS $CFLAG_OS_SYS $DEBUGGS_FLAGS $MTRACE_C_FLAGS $WX_CFLAGS $SDL_CFLAGS"
CXXFLAGS=" -Wall $CXXFLAGS $CFLAG_OS_SYS $DEBUGGS_FLAGS $MTRACE_C_FLAGS $WX_CXXFLAGS $SDL_CFLAGS"
LIBS=" -Wall $LIBS  $GIFLIB $WX_LIBS $SDL_LIBS $SDL_MIXER_LIB $MTRACE_LD_FLAGS"
# AC_MSG_WARN( [CXXFLAGS = $CXXFLAGS] )
# AC_MSG_WARN( [LIBS = $LIBS] )
AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(LIBS)

AC_CONFIG_COMMANDS(myinfo, [echo $LIBS] )



#--------------------------------------------------
AC_CONFIG_FILES([Makefile
                 src/Makefile
                 ressources/Makefile])
AC_OUTPUT
