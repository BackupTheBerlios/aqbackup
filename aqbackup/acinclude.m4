dnl as-scrub-include.m4 0.0.1
dnl autostars m4 macro for scrubbing CFLAGS of system include dirs
dnl because gcc 3.x complains about including system including dirs
dnl
dnl thomas@apestaart.org
dnl
dnl This macro uses output of cpp -v and expects it to contain text that 
dnl looks a little bit like this:
dnl #include <...> search starts here:
dnl  /usr/local/include
dnl  /usr/lib/gcc-lib/i386-redhat-linux/3.2/include
dnl  /usr/include
dnl End of search list.

dnl AS_SCRUB_INCLUDE(VAR)
dnl example
dnl AS_SCRUB_INCLUDE(CFLAGS)
dnl will remove all system include dirs from the given CFLAGS

AC_DEFUN(AS_SCRUB_INCLUDE,
[
  GIVEN_CFLAGS=$[$1]
  INCLUDE_DIRS=`echo | cpp -v 2>&1`

  dnl remove everything from this output between the "starts here" and "End of"
  dnl line
  INCLUDE_DIRS=`echo $INCLUDE_DIRS | sed -e 's/.*<...> search starts here://' | sed -e 's/End of search list.*//'`
  for dir in $INCLUDE_DIRS; do
    GIVEN_CFLAGS=$(echo $GIVEN_CFLAGS | sed -e "s;-I$dir ;;" | sed -e "s;-I$dir$;;")
  done
  [$1]=$GIVEN_CFLAGS
])
# $Id: acinclude.m4,v 1.1 2003/06/07 21:07:36 aquamaniac Exp $
# (c) 2002 Martin Preuss<martin@libchipcard.de>
# This function checks for the debugmode


AC_DEFUN(AQ_DEBUGMODE,[
dnl IN: 
dnl   nothing
dnl OUT:
dnl   Variables:
dnl     DEBUGMODE: number of the debug level (subst)
dnl   Defines:
dnl     DEBUGMODE: number of the debug level (subst)

dnl check for debugmode
AC_MSG_CHECKING(for debug mode)
AC_ARG_WITH(debug-mode,
  [  --with-debug-mode=MODE  debug mode],
  [DEBUGMODE="$withval"],
  [DEBUGMODE="0"])
AC_SUBST(DEBUGMODE)
AC_DEFINE_UNQUOTED(DEBUGMODE,$DEBUGMODE,[debug mode to be used])
AC_MSG_RESULT($DEBUGMODE)
])
# $Id: acinclude.m4,v 1.1 2003/06/07 21:07:36 aquamaniac Exp $
# (c) 2002 Martin Preuss<martin@libchipcard.de>
# This function checks for the output path for the documentation

AC_DEFUN(AQ_DOCPATH,[
dnl PREREQUISITES:
dnl   none
dnl IN: 
dnl   nothing
dnl OUT:
dnl   Variables:
dnl     docpath: Output path for the documentation (subst)
dnl   Defines:

# check for docpath
AC_MSG_CHECKING(for docpath)
AC_ARG_WITH(docpath, [  --with-docpath=DIR      where to store the apidoc],
  [docpath="$withval"],
  [docpath="apidoc"])
AC_SUBST(docpath)
AC_MSG_RESULT($docpath)
])
# $Id: acinclude.m4,v 1.1 2003/06/07 21:07:36 aquamaniac Exp $
# (c) 2002 Martin Preuss<martin@libchipcard.de>
# These functions search for KDE 2-3


AC_DEFUN(AQ_CHECK_KDE,[
dnl $1 = operating system name ("linux", "freebsd", "windows")
dnl $2 = subdirs to include when KDE is available
dnl searches a dir for some files
dnl You may preset the return variables.
dnl All variables which already have a value will not be altered
dnl returns some variables:
dnl  kde_generation either 1,2 or 3
dnl  kde_includes path to includes
dnl  kde_libs path to libraries
dnl  kde_install_dir

ops="$1"
lsd="$2"

dnl check if kde apps are desired
AC_MSG_CHECKING(if KDE applications should be compiled)
AC_ARG_ENABLE(kdeapps,
  [  --enable-kdeapps         enable compilation of kde applications (default=yes)],
  enable_kdeapps="$enableval",
  enable_kdeapps="yes")
AC_MSG_RESULT($enable_kdeapps)

if test "$enable_kdeapps" = "no"; then
   kde_libs=""
   kde_includes=""
   kde_app=""
else

dnl paths for kde
AC_ARG_WITH(kde-dir, 
  [  --with-kde-dir=DIR      uses kde from given dir],
  [local_kde_paths="$withval"],
  [local_kde_paths="\
  	$KDEDIR \
        /opt/kde3 \
      	/usr/local/lib/kde3 \
        /usr/lib/kde3 \
        /lib/kde3 \
        /opt/kde2 \
        /usr/local/lib/kde2 \
        /usr/lib/kde2 \
        /lib/kde2 \
        /opt/kde1 \
        /usr/local/lib/kde1 \
        /usr/lib/kde1 \
        /lib/kde1 \
        /opt/kde \
        /usr/local/lib/kde \
        /usr/lib/kde \
        /lib/kde \
        /usr/local \
        /usr \
        / \
        "])

dnl check for library
AC_MSG_CHECKING(for kde libraries)
dnl check for 3
if test -z "$kde_libs" ; then
	AQ_SEARCH_FOR_PATH([lib/libkdeui.so.4],[$local_kde_paths])
	if test -n "$found_dir" ; then
        	kde_dir="$found_dir"
    		kde_libs="-L$found_dir/lib"
                kde_generation="3"
	fi
fi

dnl check for 2
if test -z "$kde_libs"; then
	AQ_SEARCH_FOR_PATH([lib/libkdeui.so.3],[$local_kde_paths])
	if test -n "$found_dir" ; then
        	kde_dir="$found_dir"
    		kde_libs="-L$found_dir/lib"
                kde_generation="2"
	fi
fi

dnl check for 1
if test -z "$kde_libs"; then
	AQ_SEARCH_FOR_PATH([lib/libkdeui.so],[$local_kde_paths])
	if test -n "$found_dir" ; then
        	kde_dir="$found_dir"
    		kde_libs="-L$found_dir/lib"
                kde_generation="1"
	fi
fi
if test -z "$kde_libs"; then
	AC_MSG_RESULT(not found)
else
	AC_MSG_RESULT($kde_libs)
fi

dnl check for includes
AC_MSG_CHECKING(for kde includes)
if test -z "$kde_includes"; then
       	AQ_SEARCH_FOR_PATH([include/kpushbutton.h],[$kde_dir $local_kde_paths])
       	if test -n "$found_dir" ; then
       		kde_includes="-I$found_dir/include"
       	fi
fi
if test -z "$kde_includes"; then
       	AQ_SEARCH_FOR_PATH([include/kde3/kpushbutton.h],[$kde_dir $local_kde_paths])
       	if test -n "$found_dir" ; then
       		kde_includes="-I$found_dir/include/kde3"
       	fi
fi
if test -z "$kde_includes"; then
       	AQ_SEARCH_FOR_PATH([include/kde2/kpushbutton.h],[$kde_dir $local_kde_paths])
       	if test -n "$found_dir" ; then
       		kde_includes="-I$found_dir/include/kde2"
       	fi
fi
if test -z "$kde_includes"; then
       	AQ_SEARCH_FOR_PATH([include/kde1/kpushbutton.h],[$kde_dir $local_kde_paths])
       	if test -n "$found_dir" ; then
       		kde_includes="-I$found_dir/include/kde1"
       	fi
fi
if test -z "$kde_includes"; then
       	AQ_SEARCH_FOR_PATH([include/kde/kpushbutton.h],[$kde_dir $local_kde_paths])
       	if test -n "$found_dir" ; then
       		kde_includes="-I$found_dir/include/kde"
       	fi
fi
if test -z "$kde_includes"; then
	AC_MSG_RESULT(not found)
else
	AC_MSG_RESULT($kde_includes)
fi


# check if all necessary kde components where found
if test -n "$kde_includes" && \
   test -n "$kde_libs"; then
   kde_app="$lsd"
   AC_DEFINE_UNQUOTED(KDE_GENERATION,$kde_generation,[KDE generation])
   AC_MSG_CHECKING(for kde install prefix)
   AC_ARG_WITH(kde-prefix, 
  	[  --with-kde-prefix=DIR      install kde apps to prefix],
  	[kde_install_dir="$withval"],
  	[kde_install_dir="$kde_dir"])
   AC_MSG_RESULT($kde_install_dir)
else
   kde_libs=""
   kde_includes=""
   kde_app=""
   if test "$enable_kdeapps" = "yes"; then
        AC_MSG_ERROR([
 Compilation of KDE applications is enabled but I could not find some KDE
 components (see which are missing in messages above).
 If you don't want to compile KDE applications please use "--disable-kdeapps".
 ])
   fi
fi

dnl end of if "$enable_kdeapps"
fi

AC_SUBST(kde_dir)
AC_SUBST(kde_app)
AC_SUBST(kde_libs)
AC_SUBST(kde_includes)
AC_SUBST(kde_generation)
AC_SUBST(kde_install_dir)
])
# $Id: acinclude.m4,v 1.1 2003/06/07 21:07:36 aquamaniac Exp $
# (c) 2002 Martin Preuss<martin@libchipcard.de>
# These functions guess your operation system

AC_DEFUN(AQ_CHECK_OS,[
dnl IN: 
dnl   - AC_CANONICAL_SYSTEM muste be called before
dnl OUT:
dnl   Variables:
dnl     OSYSTEM: Short name of your system (subst)
dnl     OS_TYPE: either "posix" or "windows" (subst)
dnl     MAKE_DLL_TARGET: under windows this is set to "dll" (subst)
dnl     INSTALL_DLL_TARGET: under Windows this is set to "dll-install" (subst)
dnl   Defines:
dnl     OS_NAME: full name of your system
dnl     OS_SHORTNAME: short name of your system
dnl     Depending on your system one of the following is defined in addition:
dnl      OS_LINUX, OS_OPENBSD, OS_FREEBSD, OS_BEOS, OS_WIN32

# check for OS
AC_MSG_CHECKING([target system type])
OSYSTEM=""
OS_TYPE=""
MAKE_DLL_TARGET=""
INSTALL_DLL_TARGET=""
AC_DEFINE_UNQUOTED(OS_NAME,"$target", [target system])
case "$target" in
    *-linux*)
	OSYSTEM="linux"
	AC_DEFINE(OS_LINUX,1,[if linux is used])
	OS_TYPE="posix"
	;;
    *-openbsd*)
	OSYSTEM="openbsd"
	AC_DEFINE(OS_OPENBSD,1,[if OpenBSD is used])
	OS_TYPE="posix"
	;;
    *-freebsd*)
	OSYSTEM="freebsd"
	AC_DEFINE(OS_FREEBSD,1,[if FreeBSD is used])
	OS_TYPE="posix"
	;;
    *-beos*)
	OSYSTEM="beos"
	AC_DEFINE(OS_BEOS,1,[if BeOS is used])
	OS_TYPE="posix"
	;;
    *-win32*)
    	OSYSTEM="windows"
	AC_DEFINE(OS_WIN32,1,[if WIN32 is used])
	OS_TYPE="windows"
        AC_DEFINE_UNQUOTED(BUILDING_DLL,1,[if DLL is to be built])
	MAKE_DLL_TARGET="dll"
	INSTALL_DLL_TARGET="dll-install"
	;;
    *-mingw32*)
	OSYSTEM="windows"
	AC_DEFINE(OS_WIN32,1,[if WIN32 is used])
	OS_TYPE="windows"
        AC_DEFINE_UNQUOTED(BUILDING_DLL,1,[if DLL is to be built])
	MAKE_DLL_TARGET="dll"
	INSTALL_DLL_TARGET="dll-install"
	;;
    *)
	AC_MSG_WARN([Sorry, but target $target is not supported.
        Please report if it works anyway. We will assume that your system
        is a posix system and continue.])
	OSYSTEM="unknown"
	OS_TYPE="posix"
	;;
esac

AC_SUBST(OSYSTEM)
AC_DEFINE_UNQUOTED(OS_SHORTNAME,"$OSYSTEM",[target system])
AC_SUBST(OS_TYPE)
AC_DEFINE_UNQUOTED(OS_TYPE,"$OS_TYPE",[system type])
AC_SUBST(MAKE_DLL_TARGET)
AC_SUBST(INSTALL_DLL_TARGET)

AC_MSG_RESULT($OS_TYPE)
])


# $Id: acinclude.m4,v 1.1 2003/06/07 21:07:36 aquamaniac Exp $
# (c) 2002 Martin Preuss<martin@libchipcard.de>
# These functions search for QT 2-3


AC_DEFUN(AQ_CHECK_QT,[
dnl $1 = operating system name ("linux", "freebsd", "windows")
dnl $2 = subdirs to include when QT is available
dnl searches a dir for some files
dnl You may preset the return variables.
dnl All variables which already have a value will not be altered
dnl returns some variables:
dnl  qt_generation either 1,2 or 3
dnl  qt_includes path to includes
dnl  qt_libs path to libraries
dnl  qt_moc path to moc
dnl  qt_uic path to uic

ops="$1"
lsd="$2"

dnl check if qt apps are desired
AC_MSG_CHECKING(if QT applications should be compiled)
AC_ARG_ENABLE(qtapps,
  [  --enable-qtapps         enable compilation of qt applications (default=yes)],
  enable_qtapps="$enableval",
  enable_qtapps="yes")
AC_MSG_RESULT($enable_qtapps)

if test "$enable_qtapps" = "no"; then
   qt_libs=""
   qt_includes=""
   qt_app=""
   qt_moc=""
   qt_uic=""
else

dnl paths for qt
AC_ARG_WITH(qt-dir, 
  [  --with-qt-dir=DIR      uses qt from given dir],
  [local_qt_paths="$withval"],
  [local_qt_paths="\
  	$QTDIR \
      	/usr/local/lib/qt3 \
        /usr/lib/qt3 \
        /lib/qt3 \
        /usr/local/lib/qt2 \
        /usr/lib/qt2 \
        /lib/qt2 \
        /usr/local/lib/qt1 \
        /usr/lib/qt1 \
        /lib/qt1 \
        /usr/local/lib/qt \
        /usr/lib/qt \
        /lib/qt \
        /usr/local \
        /usr \
        /usr/X11R6 \
        / \
        "])

AC_MSG_CHECKING(if threaded qt may be used)
AC_ARG_ENABLE(qt-threads,
  [  --enable-qt-threads         enable qt-mt library (default=yes)],
  enable_qt_threads="$enableval",
  enable_qt_threads="yes")
AC_MSG_RESULT($enable_qt_threads)

dnl check for library
AC_MSG_CHECKING(for qt libraries)
dnl check for 3
if test -z "$qt_libs" && test "$enable_qt_threads" != "no"; then
	AQ_SEARCH_FOR_PATH([lib/libqt-mt.so.3],[$local_qt_paths])
	if test -n "$found_dir" ; then
        	qt_dir="$found_dir"
    		qt_libs="-L$found_dir/lib -lqt-mt"
                if test "$ops" = "freebsd"; then
                	qt_libs="$qt_libs -lc_r"
                fi
                qt_generation="3"
	fi
fi
if test -z "$qt_libs"; then
	AQ_SEARCH_FOR_PATH([lib/libqt.so.3],[$local_qt_paths])
	if test -n "$found_dir" ; then
        	qt_dir="$found_dir"
    		qt_libs="-L$found_dir/lib -lqt"
                qt_generation="3"
	fi
fi

dnl check for 2
if test -z "$qt_libs" && test "$enable_qt_threads" != "no"; then
	AQ_SEARCH_FOR_PATH([lib/libqt-mt.so.2],[$local_qt_paths])
	if test -n "$found_dir" ; then
        	qt_dir="$found_dir"
    		qt_libs="-L$found_dir/lib -lqt-mt"
                qt_generation="2"
	fi
fi
if test -z "$qt_libs"; then
	AQ_SEARCH_FOR_PATH([lib/libqt.so.2],[$local_qt_paths])
	if test -n "$found_dir" ; then
        	qt_dir="$found_dir"
    		qt_libs="-L$found_dir/lib -lqt"
                qt_generation="2"
	fi
fi

dnl check for 1
if test -z "$qt_libs" && test "$enable_qt_threads" != "no"; then
	AQ_SEARCH_FOR_PATH([lib/libqt-mt.so.1],[$local_qt_paths])
	if test -n "$found_dir" ; then
        	qt_dir="$found_dir"
    		qt_libs="-L$found_dir/lib -lqt-mt"
                qt_generation="1"
	fi
fi
if test -z "$qt_libs"; then
	AQ_SEARCH_FOR_PATH([lib/libqt.so.1],[$local_qt_paths])
	if test -n "$found_dir" ; then
        	qt_dir="$found_dir"
    		qt_libs="-L$found_dir/lib -lqt"
                qt_generation="1"
	fi
fi
if test -z "$qt_libs"; then
	AC_MSG_RESULT(not found)
else
	AC_MSG_RESULT($qt_libs)
fi

dnl check for includes
AC_MSG_CHECKING(for qt includes)
if test -z "$qt_includes"; then
       	AQ_SEARCH_FOR_PATH([include/qt.h],[$qt_dir $local_qt_paths])
       	if test -n "$found_dir" ; then
       		qt_includes="-I$found_dir/include"
       	fi
fi
if test -z "$qt_includes"; then
       	AQ_SEARCH_FOR_PATH([include/qt3/qt.h],[$qt_dir $local_qt_paths])
       	if test -n "$found_dir" ; then
       		qt_includes="-I$found_dir/include/qt3"
       	fi
fi
if test -z "$qt_includes"; then
       	AQ_SEARCH_FOR_PATH([include/qt2/qt.h],[$qt_dir $local_qt_paths])
       	if test -n "$found_dir" ; then
       		qt_includes="-I$found_dir/include/qt2"
       	fi
fi
if test -z "$qt_includes"; then
       	AQ_SEARCH_FOR_PATH([include/qt1/qt.h],[$qt_dir $local_qt_paths])
       	if test -n "$found_dir" ; then
       		qt_includes="-I$found_dir/include/qt1"
       	fi
fi
if test -z "$qt_includes"; then
       	AQ_SEARCH_FOR_PATH([include/qt/qt.h],[$qt_dir $local_qt_paths])
       	if test -n "$found_dir" ; then
       		qt_includes="-I$found_dir/include/qt"
       	fi
fi
if test -z "$qt_includes"; then
	AC_MSG_RESULT(not found)
else
	AC_MSG_RESULT($qt_includes)
fi

dnl check for moc
AC_MSG_CHECKING(for qt moc)
if test -z "$qt_moc"; then
       	AQ_SEARCH_FOR_PATH([bin/moc],[$qt_dir $local_qt_paths])
       	if test -n "$found_dir" ; then
       		qt_moc="$found_dir/bin/moc"
       	fi
fi
if test -z "$qt_moc"; then
       	AQ_SEARCH_FOR_PATH([bin/moc2],[$qt_dir $local_qt_paths])
       	if test -n "$found_dir" ; then
       		qt_moc="$found_dir/bin/moc2"
       	fi
fi
if test -z "$qt_moc"; then
	AC_MSG_RESULT(not found)
else
	AC_MSG_RESULT($qt_moc)
fi

dnl check for uic
AC_MSG_CHECKING(for qt uic)
if test -z "$qt_uic"; then
       	AQ_SEARCH_FOR_PATH([bin/uic],[$qt_dir $local_qt_paths])
       	if test -n "$found_dir" ; then
       		qt_uic="$found_dir/bin/uic"
       	fi
fi
if test -z "$qt_uic"; then
	AC_MSG_RESULT(not found)
else
	AC_MSG_RESULT($qt_uic)
fi

# check if all necessary qt components where found
if test -n "$qt_includes" && \
   test -n "$qt_libs" && \
   test -n "$qt_moc" && \
   test -n "$qt_uic"; then
   qt_app="$lsd"
   AC_DEFINE_UNQUOTED(QT_GENERATION,$qt_generation, [QT generation])
else
   qt_libs=""
   qt_includes=""
   qt_app=""
   qt_moc=""
   qt_uic=""
   if test "$enable_qtapps" = "yes"; then
        AC_MSG_ERROR([
 Compilation of QT applications is enabled but I could not find some QT
 components (see which are missing in messages above).
 If you don't want to compile QT applications please use "--disable-qtapps".
 ])
   fi
fi

dnl end of if "$enable_qtapps"
fi

AC_SUBST(qt_dir)
AC_SUBST(qt_app)
AC_SUBST(qt_libs)
AC_SUBST(qt_includes)
AC_SUBST(qt_moc)
AC_SUBST(qt_uic)
AC_SUBST(qt_generation)
])

# $Id: acinclude.m4,v 1.1 2003/06/07 21:07:36 aquamaniac Exp $
# (c) 2002 Martin Preuss<martin@libchipcard.de>
# These functions search for files


AC_DEFUN(AQ_SEARCH_FOR_PATH,[
dnl searches for a file in a path
dnl $1 = file to search
dnl $2 = paths to search in
dnl returns the directory where the file is found (found_dir)
found_dir=""
ls=$1
ld="$2"
for li in $ld; do
    if test -r "$li/$ls"; then
        found_dir="$li"
        break
    fi
done
])

AC_DEFUN(AQ_SEARCH_FILES,[
dnl searches a dir for some files
dnl $1 = path where to search
dnl $2 = files to find
dnl returns the name of the file found (found_file)
found_file=""
ls="$1"
ld="$2"
lf=""
for li in $ld; do
    lf2=`find "$ls" -maxdepth 1 -name "$li" 2>/dev/null`
    lf="$lf $lf2"
done
for li in $lf; do
    if test -r "$li"; then
	found_file=`basename "$li"`
	break
    fi
done
])
# $Id: acinclude.m4,v 1.1 2003/06/07 21:07:36 aquamaniac Exp $
# (c) 2002 Martin Preuss<martin@libchipcard.de>
# This function check if SSL is wanted

AC_DEFUN(AQ_CHECK_SSL,[
dnl PREREQUISITES:
dnl   - AQ_CHECK_OS must becalled before
dnl IN: 
dnl   nothing
dnl OUT:
dnl   Variables:
dnl     ssl_libraries: Path to the SSL libraries (subst)
dnl     ssl_lib: SSL libraries to link against (subst)
dnl     ssl_includes: Path to the SSL includes (subst)
dnl   Defines:

dnl check if ssl is desired
AC_MSG_CHECKING(if OpenSSL should be used)
AC_ARG_ENABLE(ssl,
  [  --enable-ssl             enable encryption (default=yes)],
  enable_ssl="$enableval",
  enable_ssl="yes")
AC_MSG_RESULT($enable_ssl)

if test "$enable_ssl" != "no"; then

dnl ******* openssl includes ***********
AC_MSG_CHECKING(for openssl includes)
AC_ARG_WITH(openssl-includes, [  --with-openssl-includes=DIR adds openssl include path],
  [ssl_search_inc_dirs="$withval"],
  [ssl_search_inc_dirs="/usr/include\
    		       /usr/local/include\
		       /usr/local/ssl/include\
  		       /usr/ssl/include"])

dnl search for ssl
AQ_SEARCH_FOR_PATH([openssl/des.h],[$ssl_search_inc_dirs])
if test -z "$found_dir" ; then
    AC_MSG_ERROR(Please specify the location of your openssl includes.)
fi
ssl_includes="-I$found_dir"
AC_MSG_RESULT($ssl_includes)


dnl ******* openssl lib ***********
AC_MSG_CHECKING(for openssl libs)
AC_ARG_WITH(openssl-libname, [  --with-openssl-libname=NAME  specify the name of the openssl library],
  [ssl_search_lib_names="$withval"],
  [ssl_search_lib_names="libcrypto.so \
	                 libcrypto.so.* \
	                 libcrypto.a"])

AC_ARG_WITH(openssl-libs, [  --with-openssl-libs=DIR  adds openssl library path],
  [ssl_search_lib_dirs="$withval"],
  [ssl_search_lib_dirs="/usr/lib \
		       /usr/local/lib \
		       /usr/lib/ssl/lib \
		       /usr/lib/openssl/lib \
		       /usr/local/ssl/lib \
		       /usr/local/openssl/lib \
		       /lib"])

dnl search for openssl libs
if test "$OSYSTEM" != "windows" ; then
   for d in $ssl_search_lib_dirs; do
     AQ_SEARCH_FILES("$d",$ssl_search_lib_names)
     if test -n "$found_file" ; then
        ssl_libraries="-L$d"
        ssl_lib="-l`echo $found_file | sed 's/lib//;s/\.so*//;s/\.a//'`"
        break
     fi
   done
   else
     ssl_libraries="-L/c/windows"
     ssl_lib="-llibeay32 -llibssl32"
fi

if test -z "$ssl_libraries" -o -z "$ssl_lib" -o -z "$ssl_includes"; then
    AC_MSG_WARN(ssl libraries not found.)
else
    AC_MSG_RESULT($ssl_libraries ${ssl_lib})
fi


# end of "if enable-ssl"
fi
AC_SUBST(ssl_includes)
AC_SUBST(ssl_libraries)
AC_SUBST(ssl_lib)
])

