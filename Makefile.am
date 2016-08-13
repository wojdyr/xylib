
ACLOCAL_AMFLAGS=-I build-aux
SUBDIRS = xylib

EXTRA_DIST = web.css sample-urls README.rst README.dev xylib.py \
	     gui/xyconvert.rc gui/xyconvert16.xpm gui/xyconvert48.xpm \
	     CMakeLists.txt

bin_PROGRAMS = xyconv

dist_man_MANS = xyconv.1

xyconv_SOURCES = xyconv.cpp

xyconv_LDADD = xylib/libxy.la -lm
if USE_XYLIB_DLL
xyconv_CPPFLAGS = -DXYLIB_DLL
endif

if BUILD_GUI
bin_PROGRAMS += gui/xyconvert

if WINDOWS
RC_FLAGS=`echo $(DEFS) $(CPPFLAGS) $(WX_CPPFLAGS) | sed -e 's/-mthreads//g'`
xyconvert_res = gui/xyconvertres.o
$(xyconvert_res): gui/xyconvert.rc gui/xyconvert.ico
	$(WINDRES) -o $@ $(RC_FLAGS) $<
endif

gui_xyconvert_SOURCES = gui/xyconvert.cpp gui/xybrowser.cpp gui/xybrowser.h \
			gui/uplot.cpp gui/uplot.h
gui_xyconvert_CPPFLAGS = $(WX_CPPFLAGS) -DXYCONVERT
gui_xyconvert_LDADD = $(xyconvert_res) $(WX_LIBS) xylib/libxy.la -lm
endif

index.html: README.rst web/web.css
	rst2html --stylesheet=$(srcdir)/web/web.css \
	         --template=$(srcdir)/web/web-tmpl.txt $< $@
