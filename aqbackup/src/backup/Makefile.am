INCLUDES=\
  @chameleon_includes@ \
  -I${top_srcdir} \
  -I${top_srcdir}/src/ \
  @all_includes@

noinst_LTLIBRARIES=libaqbackup.la
noinst_PROGRAMS=aqbtest
iheaderdir=${includedir}/aqbackup
iheader_DATA=\
 entry.h \
 misc.h \
 server.h \
 client.h client_direct.h client_direct_p.h \
 aqbackup.h aqbackup_p.h \
 interactor.h interactor_p.h

libaqbackup_la_SOURCES=\
 misc.c \
 entry.c \
 server.c \
 client.c client_direct.c \
 aqbackup.c \
 interactor.c

libaqbackup_la_LIBADD=../converter/libconverter.la @chameleon_libs@ @all_libraries@

aqbtest_SOURCES=test.c
#aqbtest_LDADD=-L. -laqbackup @libz_libraries@ @libz_lib@
aqbtest_LDADD=libaqbackup.la 


