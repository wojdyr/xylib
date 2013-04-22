
VERSION=1.1

if [ $# -eq 0 ]; then
    echo Usage: $0 step
    echo Steps to prepare a new release:
    echo 0. check version numbers and soname
    echo 1. test compilation
    echo 2. make tarball
    echo 3. make Windows binaries
    echo 4. RPMs: https://build.opensuse.org/project/show?project=home%3Awojdyr
    echo 5. prepare DEBs in Launchpad
    echo 6. SourceForge file release
    echo 7. update webpage
    echo 8. announce at FreshMeat
    echo 9. make Windows static lib for linking with fityk
    exit
fi

arg=$1

# 0. check version numbers and soname
if [ $arg -eq 0 ]; then
    echo version: $VERSION
    echo in xylib/xylib.h:
    grep 'define XYLIB_VERSION' xylib/xylib.h
    echo in README.rst:
    grep '.tar.bz2' README.rst
    grep '.zip' README.rst
    echo in xylib/Makefile.am:
    grep 'version-info' xylib/Makefile.am
    echo in xylib_capi.py:
    grep LoadLibrary xylib_capi.py
    echo in README.rst:
    grep "\* $VERSION" README.rst
    echo svnversion: `svnversion`
fi

# 1. test compilation from svn
if [ $arg -eq 1 ]; then
    rm -rf svn_copy
    mkdir svn_copy
    cd svn_copy
    svn co https://xylib.svn.sourceforge.net/svnroot/xylib/trunk xylib
    cd xylib
    autoreconf -i || exit 1
    ./configure --disable-static && make distcheck || exit 1
    ./configure --prefix=`pwd`/install_dir --disable-shared && make install \
        || exit 1
    install_dir/bin/xyconv -v || exit 1
fi

# 2. make tarball
if [ $arg -eq 2 ]; then
    gunzip svn_copy/xylib/xylib-*.tar.gz
    bzip2 svn_copy/xylib/xylib-*.tar
    mv svn_copy/xylib/xylib-*.tar.bz2 . && echo OK
    #make dist-bzip2
fi

# 3. make Windows binaries 
if [ $arg -eq 3 ]; then
    mkdir -p xylib_win-$VERSION/docs
    cp -r svn_copy/xylib/install_dir/include/xylib/ xylib_win-$VERSION
    rm index.html
    #make index.html
    rst2html --stylesheet-path=web.css README.rst index.html
    cp index.html README.dev TODO COPYING sample-urls xylib_win-$VERSION/docs
    echo copy files xylib.dll and xyconv.exe to `pwd`/xylib_win-$VERSION
    echo and do:  zip -r xylib_win-$VERSION.zip xylib_win-$VERSION/
fi

# 5. prepare DEBs in Launchpad
if [ $arg -eq 5 ]; then
    echo go to directory with old debs, or: apt-get source xylib
    echo cd xylib-oldversion
    echo uupdate `pwd`/xylib-$VERSION.tar.bz2
    echo cd ../xylib-$VERSION
    echo sed -i 's/0ubuntu1/1~lucid1/' debian/changelog
    echo debuild # test building
    echo debuild -S -sa # build source package
    echo dput ppa:`whoami`/fityk ../xylib_$VERSION-1~lucid1_source.changes
    echo sed -i 's/lucid/maverick/' debian/changelog
    echo etc.
fi

# 6. SourceForge file release
if [ $arg -eq 6 ]; then
scp xylib-$VERSION.tar.bz2 xylib_win-$VERSION.zip `whoami`,xylib@frs.sourceforge.net:/home/frs/project/x/xy/xylib/
echo verify the release: https://sourceforge.net/downloads/xylib/
fi

# 7. update webpage
if [ $arg -eq 7 ]; then
    make index.html
    scp index.html `whoami`,xylib@web.sourceforge.net:htdocs/
fi

# 8. announce at FreshMeat
if [ $arg -eq 8 ]; then
    grep -5 "\* $VERSION" README.rst | tail -6
    echo
    echo http://freshmeat.net/projects/xylib/releases/new
fi

# 9. make Windows static lib for linking with fityk
if [ $arg -eq 9 ]; then
    rm -rf mingw32
    mkdir mingw32
    cd mingw32
    MDIR=$HOME/local/mingw32msvc
    ../configure --host=i586-mingw32msvc --enable-static --disable-shared \
	         CPPFLAGS="-I$HOME/local/src/boost_1_42_0/ -I$MDIR/include/" \
		 CXXFLAGS="-O3" LDFLAGS="-s -L$MDIR/lib" --without-bzlib \
		 --prefix=$MDIR && \
    make install
fi

