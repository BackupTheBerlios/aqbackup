# $Id: libz.m4,v 1.1 2003/06/07 21:07:46 aquamaniac Exp $
# (c) 2002 Martin Preuss<martin@libchipcard.de>
# This function check if SSL is wanted

AC_DEFUN(AQ_CHECK_LIBZ,[
dnl PREREQUISITES:
dnl   - AQ_CHECK_OS must becalled before
dnl IN: 
dnl   nothing
dnl OUT:
dnl   Variables:
dnl     libz_libraries: Path to the ZLIB libraries (subst)
dnl     libz_lib: libz libraries to link against (subst)
dnl     libz_includes: Path to the ZLIB includes (subst)
dnl     libz_available: if libz is available
dnl   Defines:

dnl check if libz is desired
AC_MSG_CHECKING(if libz should be used)
AC_ARG_ENABLE(libz,
  [  --enable-libz             enable encryption (default=yes)],
  enable_libz="$enableval",
  enable_libz="yes")
AC_MSG_RESULT($enable_libz)

if test "$enable_libz" != "no"; then

dnl ******* libz includes ***********
AC_MSG_CHECKING(for libz includes)
AC_ARG_WITH(libz-includes, [  --with-libz-includes=DIR adds libz include path],
  [libz_search_inc_dirs="$withval"],
  [libz_search_inc_dirs="/usr/include\
    		         /usr/local/include"])

dnl search for libz
AQ_SEARCH_FOR_PATH([zlib.h],[$libz_search_inc_dirs])
if test -z "$found_dir" ; then
    AC_MSG_ERROR(Please specify the location of your libz includes.)
fi
libz_includes="-I$found_dir"
AC_MSG_RESULT($libz_includes)


dnl ******* libz lib ***********
AC_MSG_CHECKING(for libz libs)
AC_ARG_WITH(libz-libname, [  --with-libz-libname=NAME  specify the name of the libz library],
  [libz_search_lib_names="$withval"],
  [libz_search_lib_names="libz.so \
	                 libz.so.* \
	                 libz.a"])

AC_ARG_WITH(libz-libs, [  --with-libz-libs=DIR  adds libz library path],
  [libz_search_lib_dirs="$withval"],
  [libz_search_lib_dirs="/usr/lib \
		       /usr/local/lib \
		       /lib"])

dnl search for libz libs
if test "$OSYSTEM" != "windows" ; then
   for d in $libz_search_lib_dirs; do
     AQ_SEARCH_FILES("$d",$libz_search_lib_names)
     if test -n "$found_file" ; then
        libz_libraries="-L$d"
        libz_lib="-l`echo $found_file | sed 's/lib//;s/\.so*//;s/\.a//'`"
        break
     fi
   done
   else
     libz_libraries="-L/c/windows"
     libz_lib="-llibz"
fi

if test -z "$libz_libraries" -o -z "$libz_lib" -o -z "$libz_includes"; then
    libz_available="no"
    AC_MSG_WARN(libz libraries not found.)
else
    libz_available="yes"
    AS_SCRUB_INCLUDE(libz_includes)
    AC_MSG_RESULT($libz_libraries ${libz_lib})
fi


# end of "if enable-libz"
else
  libz_available="no"
fi
AC_SUBST(libz_includes)
AC_SUBST(libz_libraries)
AC_SUBST(libz_lib)
])

