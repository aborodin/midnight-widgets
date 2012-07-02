AM_CFLAGS = -I$(top_srcdir)/lib/widgets $(GLIB_CFLAGS) -I$(top_srcdir) @CHECK_CFLAGS@
LIBS=@CHECK_LIBS@  $(top_builddir)/lib/libmc.la

TESTS = \
	widget_group

check_PROGRAMS = $(TESTS)

widget_group_SOURCES = \
	widget_group.c
