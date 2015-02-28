cmake_minimum_required(VERSION 2.8)
project(xylib C CXX)

option(USE_ZLIB "Handle compressed GZ files - requires Zlib library" ON)
option(USE_BZIP2 "Handle compressed BZ2 files - requires Bzip2 library" ON)
option(GUI "Build xyConvert GUI - requires wxWidgets 3.0+" ON)
option(BUILD_SHARED_LIBS "Build as a shared library" ON)

if(NOT DEFINED LIB_INSTALL_DIR)
  set(LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib)
endif()

# if we build static library we likely prefer 3rd-party libraries also static
if(NOT BUILD_SHARED_LIBS)
  set(CMAKE_FIND_LIBRARY_SUFFIXES
      ${CMAKE_STATIC_LIBRARY_SUFFIX} ${CMAKE_FIND_LIBRARY_SUFFIXES})
endif()
include_directories(${CMAKE_SOURCE_DIR})

find_package(Boost REQUIRED)
message(STATUS "Boost headers in: ${Boost_INCLUDE_DIR}")
include_directories(${Boost_INCLUDE_DIR})
if (BUILD_SHARED_LIBS AND WIN32)
  add_definitions(-DXYLIB_DLL=1)
endif()

if (USE_ZLIB)
  find_package(ZLIB REQUIRED)
  add_definitions(-DHAVE_LIBZ=1)
  include_directories(${ZLIB_INCLUDE_DIR})
endif()
if (USE_BZIP2)
  find_package(BZip2 REQUIRED)
  add_definitions(-DHAVE_LIBBZ2=1)
  include_directories(${Bzip2_INCLUDE_DIR})
endif()
if (GUI)
  set(wxWidgets_wxrc_EXECUTABLE no_thanks)
  find_package(wxWidgets REQUIRED adv core base)
  include(${wxWidgets_USE_FILE})
endif()

# configure.ac changes XYLIB_USE_TR1_MEMORY in cache.h (which is a bit tricky
# and may not be a good idea). Here we leave it as it is.

add_library(xy
            xylib/brucker_raw.cpp
            xylib/cache.cpp
            xylib/canberra_cnf.cpp
            xylib/canberra_mca.cpp
            xylib/chiplot.cpp
            xylib/cpi.cpp
            xylib/csv.cpp
            xylib/dbws.cpp
            xylib/gsas.cpp
            xylib/pdcif.cpp
            xylib/philips_raw.cpp
            xylib/philips_udf.cpp
            xylib/riet7.cpp
            xylib/rigaku_dat.cpp
            xylib/specsxy.cpp
            xylib/spectra.cpp
            xylib/text.cpp
            xylib/util.cpp
            xylib/uxd.cpp
            xylib/vamas.cpp
            xylib/winspec_spe.cpp
            xylib/xfit_xdd.cpp
            xylib/xylib.cpp)

target_link_libraries(xy ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES})
set_target_properties(xy PROPERTIES SOVERSION 3 VERSION "3.3.0")

add_executable(xyconv xyconv.cpp)
target_link_libraries(xyconv xy ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES})

if (GUI)
  if (WIN32)
    set(RCFILE gui/xyconvert.rc)
  endif()
  add_executable(xyconvert WIN32
                 gui/xyconvert.cpp
                 gui/xybrowser.cpp
                 gui/uplot.cpp
                 ${RCFILE})
  set_property(TARGET xyconvert
               APPEND PROPERTY COMPILE_DEFINITIONS "XYCONVERT")
  target_link_libraries(xyconvert xy ${wxWidgets_LIBRARIES}
                        ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES})
  install(TARGETS xyconvert DESTINATION bin)
endif()

install(TARGETS xyconv DESTINATION bin)
install(TARGETS xy
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
        LIBRARY DESTINATION "${LIB_INSTALL_DIR}")
