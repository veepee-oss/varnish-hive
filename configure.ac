AC_INIT(src/varnishhive.c)
AC_CONFIG_HEADERS([config.h])

PACKAGE=varnishhive
VERSION=0.5

AM_INIT_AUTOMAKE($PACKAGE, $VERSION)

AC_SUBST(INTI_CFLAGS)
AC_SUBST(INTI_LIBS)

AC_PROG_CC
AC_PROG_CC_STDC

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_TIME
AC_CHECK_HEADERS([sys/param.h])
AC_CHECK_HEADERS([sys/types.h])
AC_CHECK_HEADERS([sys/endian.h])
AC_CHECK_HEADERS([sys/filio.h])
AC_CHECK_HEADERS([sys/mount.h], [], [], [#include <sys/param.h>])
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([sys/statvfs.h])
AC_CHECK_HEADERS([sys/vfs.h])
AC_CHECK_HEADERS([endian.h])
AC_CHECK_HEADERS([execinfo.h])
AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([pthread_np.h], [], [], [#include <pthread.h>])
AC_CHECK_HEADERS([stddef.h])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([priv.h])

AC_OUTPUT([
	Makefile
	src/Makefile
])
