# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(aqbackup, 0.1, martin@libchipcard.de)
AC_CANONICAL_TARGET([])
AM_INIT_AUTOMAKE(aqbackup, 0.1, martin@libchipcard.de)
AC_CONFIG_SRCDIR([src/backup/server.c])
AC_CONFIG_HEADER([config.h])

AM_MAINTAINER_MODE

AQBACKUP_VERSION_MAJOR="0"
AQBACKUP_VERSION_MINOR="1"
AQBACKUP_VERSION_PATCHLEVEL="0"
AQBACKUP_VERSION_BUILD="0"
AQBACKUP_VERSION_TAG="CVS"
AC_SUBST(AQBACKUP_VERSION_MAJOR)
AC_SUBST(AQBACKUP_VERSION_MINOR)
AC_SUBST(AQBACKUP_VERSION_PATCHLEVEL)
AC_SUBST(AQBACKUP_VERSION_BUILD)
AC_SUBST(AQBACKUP_VERSION_TAG)

dnl setup rpm 
rpm_build_root="/tmp/aqbackup-buildroot"
rpm_version="$AQBACKUP_VERSION_MAJOR.$AQBACKUP_VERSION_MINOR.$AQBACKUP_VERSION_PATCHLEVEL"
rpm_release="1"
AC_SUBST(rpm_version)
AC_SUBST(rpm_release)
AC_SUBST(rpm_build_root)



dnl -------------------------------------------------------- prefix handling

AC_PREFIX_DEFAULT(/usr)
if test "x$prefix" = "xNONE"; then
  prefix=$ac_default_prefix
  ac_configure_args="$ac_configure_args --prefix $prefix"
fi
AC_SUBST(prefix)



dnl ------------------------------------------------------------ locale path
I18N_PATH="`eval echo $prefix/share`/locale"
AC_DEFINE_UNQUOTED(I18N_PATH,"$I18N_PATH",[Path for locales])



# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h sys/time.h syslog.h unistd.h])
AC_CHECK_HEADERS([libintl.h],[have_libintl_h=yes],[have_libintl_h=no])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_MALLOC
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([gethostbyaddr gethostbyname inet_ntoa memmove memset])
AC_CHECK_FUNCS([select socket])
AC_CHECK_FUNCS([strcasecmp strdup strerror snprintf])

AQ_CHECK_OS
AQ_CHECK_LIBZ
if test "$libz_available" = "yes"; then
  all_includes="$all_includes $libz_includes"
  all_libraries="$all_libraries $libz_libraries $libz_lib"
fi

AQ_CHAMELEON_INCLUDED(src)
all_includes="$all_includes $chameleon_includes"

dnl system dependant settings
case "$OS_TYPE" in
    posix)
        AQBACKUPC_CFGFILE="`eval echo $prefix/etc`/aqbackupc.conf"
        AQBACKUPD_CFGFILE="`eval echo $prefix/etc`/aqbackupd.conf"
        AQBACKUP_REPOSITORY_DIR="`eval echo $prefix/share`/aqbackup"
	MAKE_DLL_TARGET=""
	INSTALL_DLL_TARGET=""
	;;
    windows)
        AQBACKUPC_CFGFILE="c:\\\\windows\\\\aqbackupc.conf"
        AQBACKUPD_CFGFILE="c:\\\\windows\\\\aqbackupd.conf"
        AQBACKUP_REPOSITORY_DIR="c:\\\\aqbackup"
        AC_DEFINE_UNQUOTED(BUILDING_AQBACKUP_DLL,1,[if building DLL])
	MAKE_DLL_TARGET="dll"
	INSTALL_DLL_TARGET="dll-install"
	;;
esac


dnl check for SSL
AQ_CHECK_SSL
AC_MSG_CHECKING(if encryption is available)
if test -z "$ssl_libraries" -o -z "$ssl_lib"; then
    AC_MSG_ERROR(Sorry, but OpenSSL is required now. Please get it from www.openssl.org.)
else
    # copy necessary files to cryp dir
    cp ${srcdir}/src/chameleon/openssl/cryp.c ${srcdir}/src/chameleon/
    cp ${srcdir}/src/chameleon/openssl/cryp.h ${srcdir}/src/chameleon/
    all_libraries="$all_libraries $ssl_libraries $ssl_lib"
    all_includes="$all_includes $ssl_includes"
    cryp_c="cryp.c"
    cryp_h="cryp.h"
    AC_DEFINE(CT_USE_ENCRYPTION,1,[if encryption used])
    DEFINE_CT_USE_ENCRYPTION="#define CT_USE_ENCRYPTION"
    AC_MSG_RESULT(yes)
fi
AC_SUBST(DEFINE_CT_USE_ENCRYPTION)
AC_SUBST(cryp_c)
AC_SUBST(cryp_h)



dnl ----------------------------------------------------------------- I18N

dnl Search for gettext
LIBS=""
I18N_CATALOG=""
AC_CHECK_PROGS(XGETTEXT, xgettext, missing)
if test "$XGETTEXT" != "missing"; then
  AC_SEARCH_LIBS(gettext,intl c,[have_gettext_fn=yes], [have_gettext_fn=no])
  i18n_library=$LIBS
  LIBS=""
  AC_MSG_CHECKING(for working i18n environment)
  if test "$have_libintl_h" = "yes" -a "$have_gettext_fn" = "yes" -a "$XGETTEXT" != "missing"; then
    AC_DEFINE(HAVE_GETTEXT_ENVIRONMENT,1,[if gettext is available])
    I18N_CATALOG="catalog"
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
  fi
fi
AC_SUBST(I18N_CATALOG)


dnl setup name of global config files
AC_MSG_CHECKING(for client config file path and name)
AC_ARG_WITH(client-cfg-file, 
  [  --with-client-cfg-file=DIR     set the path and name of the client config file],
  [AQBACKUPC_CFGFILE="$withval"])
AC_DEFINE_UNQUOTED(AQBACKUPC_CFGFILE,"$AQBACKUPC_CFGFILE",[cfgfile name])
AC_SUBST(AQBACKUPC_CFGFILE)
AC_MSG_RESULT($AQBACKUPC_CFGFILE)

AC_MSG_CHECKING(for server config file path and name)
AC_ARG_WITH(server-cfg-file, 
  [  --with-server-cfg-file=DIR     set the path and name of the server config file],
  [AQBACKUPD_CFGFILE="$withval"])
AC_DEFINE_UNQUOTED(AQBACKUPD_CFGFILE,"$AQBACKUPD_CFGFILE",[cfgfile name])
AC_SUBST(AQBACKUPD_CFGFILE)
AC_MSG_RESULT($AQBACKUPD_CFGFILE)

AC_MSG_CHECKING(for default backup dir)
AC_ARG_WITH(bak-dir, 
  [  --with-bak-dir=DIR     set the path for the backup files],
  [AQBACKUP_REPOSITORY_DIR="$withval"])
AC_DEFINE_UNQUOTED(AQBACKUP_REPOSITORY_DIR,"$AQBACKUP_REPOSITORY_DIR",[repository dir])
AC_SUBST(AQBACKUP_REPOSITORY_DIR)
AC_MSG_RESULT($AQBACKUP_REPOSITORY_DIR)



dnl ----------------------------------------------------------------- summary

all_includes="$all_includes $chameleon_includes"
all_libraries="$all_libraries"
AC_SUBST(all_includes)
AC_SUBST(all_libraries)


AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/chameleon/Makefile
                 src/chameleon/openssl/Makefile
                 src/chameleon/posix/Makefile
                 src/chameleon/windows/Makefile
                 src/converter/Makefile
                 src/service/Makefile
                 src/backup/Makefile
                 src/backupservice/Makefile
                 src/progs/Makefile
                 src/progs/aqbackup/Makefile
                 ])
AC_OUTPUT
