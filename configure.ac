# Process this file with `autoreconf -i` to produce a configure script.
AC_INIT(xylib, 1.3)
AC_CONFIG_AUX_DIR(build-aux)
AC_CONFIG_MACRO_DIR(build-aux)
AC_CONFIG_SRCDIR(xylib/xylib.cpp)
AC_CONFIG_HEADER(config.h)

AM_INIT_AUTOMAKE([1.11 foreign silent-rules -Wall])

# optional zlib and bzlib libraries
AC_ARG_WITH(zlib,
 [  --without-zlib                disable zlib support (reading .gz files)])

AC_ARG_WITH(bzlib,
 [  --without-bzlib               disable bzlib support (reading .bz2 files)])

# Checks for programs.
AC_PROG_CXX
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT([disable-static win32-dll])

# Checks for libraries.
XYLIB_ADDLIB=
if test "x$with_zlib" != xno; then
  AC_CHECK_LIB(z, gzopen, , AC_MSG_ERROR([
   zlib library was not found.
   Either use flag --without-zlib or install the library.]))
  XYLIB_ADDLIB="$XYLIB_ADDLIB -lz"
  AC_CHECK_HEADER([zlib.h], [], [AC_MSG_ERROR([
   zlib.h header was not found. Either use flag --without-zlib or install
   zlib library (including header files).
 ])])
fi

if test "x$with_bzlib" != xno; then
  AC_CHECK_LIB(bz2, BZ2_bzReadOpen, , AC_MSG_ERROR([
   bzlib library was not found.
   Either use flag --without-bzlib or install the library.]))
  XYLIB_ADDLIB="$XYLIB_ADDLIB -lbz2"
  AC_CHECK_HEADER([bzlib.h], [], [AC_MSG_ERROR([
   bzlib.h header was not found. Either use flag --without-bzlib or install
   bzlib library (including header files).
 ])])
fi

# Checks for header files.
AC_LANG([C++])
AC_CHECK_HEADERS([climits vector], [],
                 [AC_MSG_ERROR([Could not find necessary C++ libs headers])])
AC_CHECK_HEADERS([boost/cstdint.hpp boost/detail/endian.hpp], [],
                 [AC_MSG_ERROR([Could not find necessary Boost headers])])
AC_CHECK_HEADERS([boost/spirit/version.hpp], [],
                 [AC_MSG_ERROR([Boost::Spirit headers were not found.])])
AC_CHECK_HEADERS([boost/tokenizer.hpp], [],
                 [AC_MSG_ERROR([Boost Tokenizer header not found.])])
AC_CHECK_HEADERS([sys/types.h sys/stat.h], [],
                 [AC_MSG_ERROR([Headers containg POSIX stat() are not found
		  Please inform xylib maintainer about this problem,
		  including information about your compiler. ])])
AC_CHECK_HEADERS([tr1/memory],
                 [AC_DEFINE([XYLIB_HAVE_TR1_MEMORY], [1],
	                            [Define if <tr1/memory> is available])])

# We change directly $srcdir/xylib/cache.h.
# Using cache.h.in would be more elegant, but would make it harder to
# to build without configure, for example in ad-hoc MS VC++ project.
# (Including config.h would make it even worse.)
uses_tr1_memory=`grep '#define XYLIB_USE_TR1_MEMORY ' $srcdir/xylib/cache.h | sed 's/.*\([01]\)/\1/g'`
if test "$ac_cv_header_tr1_memory" = "yes" && test $uses_tr1_memory != "1"; then
    sed -i.bak 's/\(#define XYLIB_USE_TR1_MEMORY\) 0/\1 1/' $srcdir/xylib/cache.h
elif test "$ac_cv_header_tr1_memory" != "yes" && test $uses_tr1_memory = "1"; then
    sed -i.bak 's/\(#define XYLIB_USE_TR1_MEMORY\) 1/\1 0/' $srcdir/xylib/cache.h
fi

# this is used in Makefile.am to define XYLIB_DLL when building xyconv
AM_CONDITIONAL(USE_XYLIB_DLL, false)
case $host in
    *mingw*)
        AM_CONDITIONAL(USE_XYLIB_DLL, test "X$enable_static" = "Xno")
    ;;
esac

AC_SUBST(XYLIB_ADDLIB)

AC_OUTPUT([
	   Makefile
	   xylib/Makefile
])

