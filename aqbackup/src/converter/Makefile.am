INCLUDES=\
  @chameleon_includes@ \
  -I${top_srcdir} \
  -I${top_srcdir}/src/ \
  @all_includes@

noinst_LTLIBRARIES=libconverter.la
iheaderdir=${includedir}/converter
iheader_DATA=\
  converter.h \
  converter_rmd160.h converter_rmd160_p.h \
  converter_md5.h converter_md5_p.h \
  converter_zip.h converter_zip_p.h \
  converter_unzip.h converter_unzip_p.h \
  converter_filein.h converter_filein_p.h \
  converter_fileout.h converter_fileout_p.h \
  converter_dummy.h converter_dummy_p.h

libconverter_la_LIBADD=@chameleon_libs@ @all_libraries@
libconverter_la_SOURCES=\
  converter.c \
  converter_rmd160.c \
  converter_md5.c \
  converter_zip.c \
  converter_unzip.c \
  converter_filein.c converter_fileout.c \
  converter_dummy.c

