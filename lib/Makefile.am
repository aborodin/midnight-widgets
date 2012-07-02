SUBDIRS = widgets

if ENABLE_MCLIB
    LIB_VERSION=`echo $(LIBMC_VERSION) | \
	tr '.' ' '| \
	while read v1 v2 v3; do echo $$v2':'$$v3':'$$v1; done`

    libmc_la_LDFLAGS=-no-undefined -version-info $(LIB_VERSION) -release $(LIBMC_RELEASE)

    lib_LTLIBRARIES = libmc.la
else
    noinst_LTLIBRARIES = libmc.la
endif

libmc_la_SOURCES =

libmc_la_CFLAGS= $(GLIB_CFLAGS) -I$(top_srcdir)

libmc_la_LIBADD = \
	widgets/libmcwidgets.la

libmc_la_LIBADD += $(MCLIBS)
libmc_la_LIBADD += $(GLIB_LIBS)
