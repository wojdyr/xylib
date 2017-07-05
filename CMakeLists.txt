cmake_minimum_required(VERSION 2.8)
project(xylib C CXX)

option(USE_ZLIB "Handle compressed GZ files - requires Zlib library" ON)
option(DOWNLOAD_ZLIB "Download and build the Zlib library" OFF)
option(USE_BZIP2 "Handle compressed BZ2 files - requires Bzip2 library" OFF)
option(GUI "Build xyConvert GUI - requires wxWidgets 3.0+" ON)
option(BUILD_SHARED_LIBS "Build as a shared library" ON)

if(NOT DEFINED LIB_INSTALL_DIR)
  set(LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib)
endif()

if (POLICY CMP0042)
  cmake_policy(SET CMP0042 OLD)
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

if (DOWNLOAD_ZLIB)
  include(ExternalProject)
  set(zlib_install "${CMAKE_INSTALL_PREFIX}")
  ExternalProject_Add(zlib
    URL "http://zlib.net/zlib-1.2.8.tar.gz"
    URL_MD5 44d667c142d7cda120332623eab69f40
    PREFIX zlib
    CMAKE_ARGS -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
               -DCMAKE_INSTALL_PREFIX:PATH=${zlib_install}
               -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
               -DSKIP_INSTALL_FILES=ON)

# the zlib library should be named zlib1.lib not zlib.lib
#    ExternalProject_Add_Step(zlib RenameLib
#    COMMAND ${CMAKE_COMMAND} -E copy "${zlib_install}/lib/zlib.lib"
#    "${zlib_install}/lib/zlib1.lib"
#    DEPENDEES install)
  set(ZLIB_INCLUDE_DIR "${zlib_install}/include")
  if (BUILD_SHARED_LIBS)
    if (WIN32)
      set(ZLIB_SUFFIX ${CMAKE_IMPORT_LIBRARY_SUFFIX})
    else()
      set(ZLIB_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif()
  else()
    set(ZLIB_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
  endif()
  if (NOT WIN32)
    set(ZLIB_NAME libz)
  elseif (BUILD_SHARED_LIBS)
    set(ZLIB_NAME zlib)
  else() # static windows
    set(ZLIB_NAME zlibstatic)
  endif()
  set(ZLIB_LIBRARIES "${zlib_install}/lib/${ZLIB_NAME}${ZLIB_SUFFIX}")
  include_directories(${ZLIB_INCLUDE_DIR})
  message(STATUS "Using local zlib: ${ZLIB_LIBRARIES}")

elseif (USE_ZLIB)
  find_package(ZLIB REQUIRED)
  include_directories(${ZLIB_INCLUDE_DIR})
endif()

if (DEFINED ZLIB_LIBRARIES)
  add_definitions(-DHAVE_LIBZ=1)
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

if (MSVC)
    # warning C4275: non dll-interface class 'std::runtime_error' used as base
    # for dll-interface class ...
    add_definitions( /wd4275 )
endif()

# show warnings by default
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-Wextra HAS_WEXTRA_FLAG)
if (HAS_WEXTRA_FLAG)
  set(EXTRA_CXX_FLAGS "-Wall -Wextra" CACHE STRING "")
endif()
set(EXTRA_CXX_FLAGS ${EXTRA_CXX_FLAGS} CACHE STRING "Flags for compiler" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXX_FLAGS}")


add_library(xy
            xylib/bruker_raw.cpp
            xylib/cache.cpp
            xylib/canberra_cnf.cpp
            xylib/canberra_mca.cpp
            xylib/chiplot.cpp
            xylib/cpi.cpp
            xylib/csv.cpp
            xylib/dbws.cpp
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
            xylib/xrdml.cpp
	    xylib/xsyg.cpp
            xylib/xylib.cpp)

if (DOWNLOAD_ZLIB)
  add_dependencies(xy zlib)
endif()
target_link_libraries(xy ${ZLIB_LIBRARIES} ${BZIP2_LIBRARIES})
set_target_properties(xy PROPERTIES SOVERSION 4 VERSION 4.1.1)

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
install(FILES xylib/xylib.h xylib/cache.h DESTINATION include/xylib)
