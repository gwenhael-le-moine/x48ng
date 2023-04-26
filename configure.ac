m4_include([version.m4])
AC_INIT([x48],[VERSION_NUMBER],[x48-bugs@gam3.net])

AC_COPYRIGHT([Copyright (c) 2006-2010 G. Allen Morris III])
AC_CANONICAL_TARGET

AC_PREREQ([2.71])

AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_CPP

AC_PATH_XTRA
if test x"$no_x" = xyes ; then
  AC_MSG_ERROR([X development libraries not found])
fi
X_LIBS="$X_LIBS -lX11"
AC_SUBST(X_LIBS)

PKG_PROG_PKG_CONFIG([0.20])

# ********* readline library
AC_ARG_ENABLE(readline,
  [  --disable-readline      disable readline support],
  [ if test x"$enableval" = xyes; then
    with_readline="yes, check"
  else
    with_readline="no"
    problem_readline=": Explicitly disabled"
  fi ],
  [ with_readline="not specified, check" ]
)

AH_TEMPLATE([HAVE_READLINE],[Define if Readline is used.])
if test x"$with_readline" != xno; then
  AC_CHECK_LIB(readline, readline,
               with_readline=yes;
               LIBS="$LIBS -lreadline";
               AC_DEFINE(HAVE_READLINE),
               with_readline=no;
               problem_readline=": Can't detect readline library.",
               [])
  AC_CHECK_LIB(history, add_history)
  AC_CHECK_HEADERS([readline/readline.h])
  AC_CHECK_HEADERS([readline/history.h])
fi

# ********* MIT Shared Memory Extension
AC_ARG_ENABLE(shm,
  [  --disable-shm           disable MIT Shared Memory Extension],
  [ if test x"$enableval" = xyes; then
    with_shm="yes, check"
  else
    with_shm="no"
    problem_shm=": Explicitly disabled"
  fi ],
  [ with_shm="not specified, check" ]
)

AH_TEMPLATE([HAVE_XSHM],[Define if MIT Shared Memory extension is used.])
if test x"$with_shm" != xno; then
AC_CHECK_HEADER(X11/extensions/XShm.h, [
  AC_CHECK_LIB(Xext, XShmQueryExtension,
               with_shm=yes; AC_DEFINE(HAVE_XSHM),
               with_shm=no;
               problem_shm=": Can't detect MIT Shared Memory ext.",
               [$X_LIBS $X_PRE_LIBS -lX11 $X_EXTRA_LIBS])
 ],problem_shm=": Can't detect MIT Shared Memory headers.",[#include <X11/Xlib.h>])
fi
if test x"$with_shm" = xyes ; then
X_LIBS="$X_LIBS -lXext"
fi

# Checks for libraries.
AC_CHECK_LIB([GL], [XCloseDisplay])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h memory.h stdlib.h string.h sys/ioctl.h sys/time.h termios.h unistd.h stdint.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

AC_STRUCT_TM

AC_DEFINE([VERSION_MAJOR], _VERSION_MAJOR, [The major version number])
AC_DEFINE([VERSION_MINOR], _VERSION_MINOR, [The minor version number])
AC_DEFINE([PATCHLEVEL], _PATCHLEVEL, [The patch level])


AC_DEFINE_UNQUOTED([COMPILE_TIME], "`date`", [Date program was compiled])
AC_DEFINE([COMPILE_VERSION], [0], [description])

# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STAT
AC_CHECK_FUNCS([bzero gethostname gettimeofday memset mkdir select strdup strrchr uname])

PKG_CHECK_MODULES(APPDEFS, xt)
AC_ARG_WITH([appdefaultdir],
    [AS_HELP_STRING([--with-appdefaultdir=DIR],[Xt app-defaults directory (default: from pkg-config)])],
     [appdefaultdir="$withval"],
     [appdefaultdir=`$PKG_CONFIG --variable=appdefaultdir xt`])
AC_SUBST(appdefaultdir)

AM_INIT_AUTOMAKE

AC_CONFIG_FILES([Makefile
                 romdump/Makefile
                 src/x48.man
                 src/Makefile])
AC_OUTPUT

echo "
X48 Configuration:

  Version:     $VERSION$VERSIONINFO

  Executables: $bindir
  Man pages:   $mandir

  With ReadLine sup. in Debuger?  $with_readline$problem_readline
  With Shared memory for XImage?  $with_shm$problem_shm

# See INSTALL for the description of what this may mean."
