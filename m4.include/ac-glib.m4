
AC_DEFUN([AC_CHECK_GLIB], [
    dnl
    dnl First try glib 2.x.
    dnl Keep this check close to the beginning, so that the users
    dnl without any glib won't have their time wasted by other checks.
    dnl

    AC_ARG_WITH([glib_static],
        AS_HELP_STRING([--with-glib-static], [Link glib statically @<:@no@:>@]))

    glib_found=no
    PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.8 gobject-2.0], [glib_found=yes], [:])
    if test x"$glib_found" = xno; then
        AC_MSG_ERROR([glib-2.0 not found or version too old (must be >= 2.8)])
    fi
])

