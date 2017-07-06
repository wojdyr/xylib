
lib_LTLIBRARIES = libxy.la

libxy_la_LDFLAGS = -no-undefined -version-info 4:1:1
libxy_la_LIBADD = $(XYLIB_ADDLIB)

libxy_la_SOURCES = xylib.cpp cache.cpp bruker_raw.cpp bruker_spc.cpp \
		   pdcif.cpp philips_raw.cpp philips_udf.cpp \
		   xrdml.cpp rigaku_dat.cpp text.cpp csv.cpp \
		   uxd.cpp vamas.cpp winspec_spe.cpp cpi.cpp dbws.cpp \
		   canberra_mca.cpp canberra_cnf.cpp xfit_xdd.cpp riet7.cpp \
		   chiplot.cpp spectra.cpp specsxy.cpp util.cpp util.h

pkginclude_HEADERS = xylib.h cache.h bruker_raw.h bruker_spc.h\
  		     pdcif.h philips_raw.h philips_udf.h xrdml.h \
		     rigaku_dat.h text.h csv.h uxd.h vamas.h winspec_spe.h \
		     cpi.h dbws.h canberra_mca.h canberra_cnf.h \
		     xfit_xdd.h riet7.h chiplot.h spectra.h specsxy.h

