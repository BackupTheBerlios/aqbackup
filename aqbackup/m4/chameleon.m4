# $Id: chameleon.m4,v 1.1 2003/06/07 21:07:46 aquamaniac Exp $
# (c) 2002 Martin Preuss<martin@libchipcard.de>
# This function checks for libchameleon


AC_DEFUN(AQ_CHAMELEON_INCLUDED,[
dnl PREREQUISITES:
dnl   - AQ_CHECK_OS must already have been called
dnl   - AC_CHECK_HEADER(syslog.h) must have been called
dnl IN: 
dnl   - path and name of Chameleon (without trailing "/chameleon")
dnl     (e.g. src/libchipcard)
dnl OUT:
dnl   Variables:
dnl     chameleon_needed_libs: libraries libchameleon needs (subst)
dnl     chameleon_includes:    CFLAGS for chameleon includes (subst)
dnl     chameleon_libs:        LDADD/LIBADD instructions for chameleon (subst)

AC_MSG_CHECKING(for included chameleon)
case "$OS_TYPE" in
 windows)
  chameleon_needed_libs="-L/c/windows -lwsock32"
  cp ${srcdir}/$1/chameleon/windows/*.c ${srcdir}/$1/chameleon/
  cp ${srcdir}/$1/chameleon/windows/*.h ${srcdir}/$1/chameleon/
  ;;
 posix)
  cp ${srcdir}/$1/chameleon/posix/*.c ${srcdir}/$1/chameleon/
  cp ${srcdir}/$1/chameleon/posix/*.h ${srcdir}/$1/chameleon/
  ;;
esac
AC_SUBST(chameleon_needed_libs)
chameleon_includes="-I\${top_srcdir}/$1"
chameleon_libs="\${top_builddir}/$1/chameleon/libchameleon.la"
AS_SCRUB_INCLUDE(chameleon_includes)
AC_SUBST(chameleon_includes)
AC_SUBST(chameleon_libs)

AC_MSG_RESULT([ok (includes=$chameleon_includes)])
])

