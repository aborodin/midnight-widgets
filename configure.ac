dnl
dnl Configure.in file for the Midnight Commander
dnl

AC_INIT([GNU Mignight Commander], [], [mc-devel@gnome.org])
AC_PREREQ(2.60)
AC_CONFIG_SRCDIR([lib/widgets/widget.c])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([mc], [0.0.1])

LIBMC_VERSION="0.0.1"
LIBMC_RELEASE="1"
AC_SUBST(LIBMC_VERSION)
AC_SUBST(LIBMC_RELEASE)

dnl Enable silent rules by default (if yes)
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE
AC_CANONICAL_HOST

AC_USE_SYSTEM_EXTENSIONS

AC_PROG_LIBTOOL
PKG_PROG_PKG_CONFIG

AC_ISC_POSIX

AC_PROG_CC_STDC
AM_PROG_CC_C_O

AC_CHECK_GLIB

AC_ARG_ENABLE([mclib],
        [AS_HELP_STRING([--enable-mclib], [Compile shared library libmc.so @<:@no@:>@])],
        [
            if test "x$enableval" = "xno" ; then
                enable_mclib=no
            else
                if test "x$enable_shared" = "xno" ; then
                    AC_MSG_WARN([Build of shared library is disabled. Specify --enable-shared first])
                    enable_mclib=no
                else
                    enable_mclib=yes
                fi
            fi
        ],
        [enable_mclib=no])

AM_CONDITIONAL([ENABLE_MCLIB], [test x$enable_mclib = xyes])

AC_HEADER_MAJOR
AC_C_CONST
dnl Check this before type sizeof checks
AC_SYS_LARGEFILE

AC_PROG_LN_S
AC_CHECK_TOOL(AR, ar, ar)

dnl
dnl OS specific flags.
dnl
case $host_os in
aux*)
    # A/UX
    LIBS="$LIBS -lposix"
    AC_DEFINE(_POSIX_SOURCE)
    ;;
esac

AC_ARG_ENABLE(
    [werror],
    AS_HELP_STRING([--enable-werror], [Handle all compiler warnings as errors])
)

if test "x$enable_werror" = xyes; then
    MC_CHECK_ONE_CFLAG([-Werror])
fi
CFLAGS="$mc_configured_cflags $CFLAGS_OPTS $CFLAGS"

MC_UNIT_TESTS

AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(LIBS)

dnl Libraries used only when building the mc binary
AC_SUBST(MCLIBS)

AC_CONFIG_FILES([
Makefile

lib/Makefile
lib/widgets/Makefile
])

if test x$enable_tests != xno; then
    AC_CONFIG_FILES([
tests/Makefile
tests/lib/Makefile
tests/lib/widgets/Makefile
])

fi


AC_OUTPUT

echo "
Configuration:

  Source code location:       ${srcdir}
  Compiler:                   ${CC}
  Compiler flags:             ${CFLAGS}
"