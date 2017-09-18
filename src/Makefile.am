AM_CFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_builddir)/include \
	-I/usr/include/varnish/ \
	-I./

varnishhive_LDADD = \
	-lm \
	-lvarnishapi

bin_PROGRAMS = varnishhive

varnishhive_SOURCES = varnishhive.c \
	varnishhive_options.c \
	varnishhive_parser.c \
	varnishhive_network.c \
	varnishhive_log.c \
	dictionary.c \
	iniparser.c \
	$(top_srcdir)/lib/libvarnishtools/vut.c \
	$(top_srcdir)/lib/libvarnish/vas.c \
	$(top_srcdir)/lib/libvarnish/flopen.c \
	$(top_srcdir)/lib/libvarnish/version.c \
	$(top_srcdir)/lib/libvarnish/vpf.c \
	$(top_srcdir)/lib/libvarnish/vtim.c \
	$(top_srcdir)/lib/libvarnish/vsb.c \
	$(top_srcdir)/lib/libvarnish/vnum.c \
	$(top_srcdir)/lib/libvarnishcompat/daemon.c \
	$(top_srcdir)/lib/libvarnishcompat/execinfo.c
