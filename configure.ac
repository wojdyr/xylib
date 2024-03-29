# Process this file with `autoreconf -i` to produce a configure script.
AC_INIT(xylib, 1.6)
AC_CONFIG_AUX_DIR(build-aux)
AC_CONFIG_MACRO_DIR(build-aux)
AC_CONFIG_SRCDIR(xylib/xylib.cpp)
AC_CONFIG_HEADER(config.h)

AM_INIT_AUTOMAKE([1.11 foreign silent-rules subdir-objects -Wall])

# optional zlib and bzlib libraries
AC_ARG_WITH(zlib,
 [  --without-zlib                disable zlib support (reading .gz files)])

AC_ARG_WITH(bzlib,
 [  --without-bzlib               disable bzlib support (reading .bz2 files)])

AC_ARG_WITH(gui,
 [  --without-gui                 do not build GUI (which requires wxWidgets)])

WX_CONFIG_OPTIONS

# Checks for programs.
AC_PROG_CXX
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT([disable-static win32-dll])

AM_MAINTAINER_MODE dnl disable (by default) maintainer mode

# Checks for libraries.
XYLIB_ADDLIB=
if test "x$with_zlib" != xno; then
  # if found defines HAVE_LIBZ
  AC_CHECK_LIB(z, gzopen, , AC_MSG_ERROR([
   zlib library was not found.
   Either use flag --without-zlib or install the library.]))
  XYLIB_ADDLIB="$XYLIB_ADDLIB -lz"
  AC_CHECK_HEADER([zlib.h], , [AC_MSG_ERROR([
   zlib.h header was not found. Either use flag --without-zlib or install
   zlib library (including header files).
 ])])
fi

if test "x$with_bzlib" != xno; then
  # if found defines HAVE_LIBBZ2
  AC_CHECK_LIB(bz2, BZ2_bzReadOpen, , AC_MSG_ERROR([
   bzlib library was not found.
   Either use flag --without-bzlib or install the library.]))
  XYLIB_ADDLIB="$XYLIB_ADDLIB -lbz2"
  AC_CHECK_HEADER([bzlib.h], , [AC_MSG_ERROR([
   bzlib.h header was not found. Either use flag --without-bzlib or install
   bzlib library (including header files).
 ])])
fi

if test "x$with_gui" != xno; then
    AM_PATH_WXCONFIG([3.0.0], [], [AC_MSG_ERROR([
              wxWidgets must be installed on your system
              but wx-config script could not be found.

              Please check that wx-config is in path, the directory
              where wxWidgets libraries are installed (returned by
              'wx-config --libs' command) is in LD_LIBRARY_PATH or
              equivalent variable and wxWindows version is 3.0.0 or above.])],
                     [adv,core,base])
fi

# Checks for header files.
AC_LANG([C++])
AC_CHECK_HEADER([climits], [],
                [AC_MSG_ERROR([Could not find necessary C++ libs headers])])
AC_CHECK_HEADER([boost/spirit/version.hpp], [],
                [AC_MSG_ERROR([Boost::Spirit headers were not found.])])
AC_CHECK_HEADER([boost/tokenizer.hpp], [],
                [AC_MSG_ERROR([Boost Tokenizer header not found.])])
AC_CHECK_HEADER([boost/property_tree/ptree.hpp], [],
                [AC_MSG_ERROR([Boost PropertyTree header not found.])])
AC_CHECK_HEADER([sys/types.h], [],
                [AC_MSG_ERROR([Header sys/types.h not found.
                 Please inform xylib maintainer about this problem,
                 including information about your compiler. ])])
AC_CHECK_HEADER([sys/stat.h], [],
                [AC_MSG_ERROR([Header sys/stat.h not found.
                 Please inform xylib maintainer about this problem,
                 including information about your compiler. ])])

AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
#ifdef _WIN32
choke me
#endif
]])],
[windows_host=no], [windows_host=yes])

if test "$windows_host" = "yes" && test "x$with_gui" != xno; then
    AC_CHECK_TOOL(WINDRES, windres)
    if test "x$WINDRES" = "x"; then
        AC_MSG_ERROR([Required resource tool 'windres' not found on PATH.])
    fi
fi

# this is used in Makefile.am to define XYLIB_DLL when building xyconv
AM_CONDITIONAL(USE_XYLIB_DLL, false)
case $host in
    *mingw*)
        AM_CONDITIONAL(USE_XYLIB_DLL, test "X$enable_static" = "Xno")
    ;;
esac

AM_CONDITIONAL([BUILD_GUI], [test "x$with_gui" != xno])
AM_CONDITIONAL([WINDOWS], [test "$windows_host" = yes])

AC_SUBST(XYLIB_ADDLIB)

AC_OUTPUT([
	   Makefile
	   xylib/Makefile
])

