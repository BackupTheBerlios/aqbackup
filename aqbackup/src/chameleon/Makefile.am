SUBDIRS=posix windows openssl
INCLUDES=-I.. @chameleon_includes@  @all_includes@

noinst_LTLIBRARIES=libchameleon.la
iheaderdir=${includedir}/chameleon
iheader_DATA=\
 debug.h \
 error.h \
 inetaddr.h \
 inetsocket.h \
 chameleon.h \
 ipcmessage.h \
 ipctransportlayer.h \
 ipcmessagelayer.h \
 ipcservicelayer.h \
 cryp.h \
 libloader.h \
 conf.h \
 directory.h \
 stringlist.h \
 logger.h

libchameleon_la_SOURCES=\
 error.c \
 inetaddr.c \
 inetsocket.c \
 chameleon.c \
 ipcmessage.c \
 ipctransportlayer.c \
 ipcmessagelayer.c \
 ipcservicelayer.c \
 cryp.c \
 libloader.c \
 conf.c \
 directory.c \
 stringlist.c \
 logger.c
libchameleon_la_LIBADD=@chameleon_needed_libs@

distclean-local:
	rm -f inetaddr.{c,h} inetsocket.{c,h} cryp.{c,h} directory.{c,h}
