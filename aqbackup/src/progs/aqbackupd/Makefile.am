INCLUDES=\
  @chameleon_includes@ \
  -I${top_srcdir} \
  -I${top_srcdir}/src/ \
  @all_includes@
  
sbin_PROGRAMS=aqbackupd

aqbackupd_SOURCES=aqbackupd.c
aqbackupd_LDADD=../../backupservice/libbackupserver.la
