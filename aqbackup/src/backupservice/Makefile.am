INCLUDES=\
  @chameleon_includes@ \
  -I${top_srcdir} \
  -I${top_srcdir}/src/ \
  -I${top_srcdir}/src/service \
  @all_includes@

noinst_LTLIBRARIES=libbackupserver.la
libbackupserver_la_SOURCES=backupserver.c
libbackupserver_la_LIBADD=../backup/libaqbackup.la ../service/libserver.la
