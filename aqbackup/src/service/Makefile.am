INCLUDES=@chameleon_includes@ \
  -I${top_srcdir} \
  -I${top_srcdir}/chipcard \
  -I${top_builddir}/src/libchipcard \
  -I${top_srcdir}/src/libchipcard/engine \
  -I${top_srcdir}/src/libchipcard/engine/reader \
  -I${top_srcdir}/src/libchipcard/engine/service @all_includes@

noinst_LTLIBRARIES=libserver.la libclient.la
noinst_HEADERS=ctservice.h ctserver.h ctclient.h

libserver_la_SOURCES=ctservice.c ctserver.c

libclient_la_SOURCES=ctservice.c ctclient.c

