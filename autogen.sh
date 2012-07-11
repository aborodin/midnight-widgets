#! /bin/sh
# Run this to generate all the initial makefiles, etc.

# Don't ignore errors.
set -e

# Make it possible to specify path in the environment
: ${AUTOCONF=autoconf}
: ${AUTOHEADER=autoheader}
: ${AUTOMAKE=automake}
: ${ACLOCAL=aclocal}
: ${AUTOPOINT=autopoint}
: ${LIBTOOLIZE=libtoolize}
: ${XGETTEXT=xgettext}

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.


(
# Some shells don't propagate "set -e" to subshells.
set -e

cd "$srcdir"

# The autoconf cache (version after 2.52) is not reliable yet.
rm -rf autom4te.cache src/vfs/smbfs/helpers/autom4te.cache

if test ! -d config; then
  mkdir config
fi

$LIBTOOLIZE

ACLOCAL_INCLUDES="-I m4"

# Some old version of GNU build tools fail to set error codes.
# Check that they generate some of the files they should.

$ACLOCAL $ACLOCAL_INCLUDES $ACLOCAL_FLAGS
test -f aclocal.m4 || \
  { echo "aclocal failed to generate aclocal.m4" >&2; exit 1; }

$AUTOHEADER || exit 1
test -f config.h.in || \
  { echo "autoheader failed to generate config.h.in" >&2; exit 1; }

$AUTOCONF || exit 1
test -f configure || \
  { echo "autoconf failed to generate configure" >&2; exit 1; }

# Workaround for Automake 1.5 to ensure that depcomp is distributed.
if test "`$AUTOMAKE --version|awk '{print $NF;exit}'`" = '1.5' ; then
    $AUTOMAKE -a src/Makefile
fi
$AUTOMAKE -a
test -f Makefile.in || \
  { echo "automake failed to generate Makefile.in" >&2; exit 1; }

) || exit 1

if test -x $srcdir/configure.mc; then
  $srcdir/configure.mc "$@"
fi
