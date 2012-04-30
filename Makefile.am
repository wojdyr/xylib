
ACLOCAL_AMFLAGS=-I build-aux
SUBDIRS = xylib

EXTRA_DIST = web.css sample-urls README.dev xylib_capi.py

bin_PROGRAMS = xyconv

dist_man_MANS = xyconv.1

xyconv_SOURCES = xyconv.cpp

xyconv_LDADD = xylib/libxy.la -lm
if USE_XYLIB_DLL
xyconv_CPPFLAGS = -DXYLIB_DLL
endif

index.html: README web.css
	rst2html --stylesheet-path=web.css README index.html
