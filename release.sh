
VERSION=1.4

if [ $# -eq 0 ]; then
    echo Usage: $0 step
    echo Steps to prepare a new release:
    echo 0. check version numbers and soname
    echo c. Coverity scan
    echo 1. test compilation
    echo 2. make tarball
    echo 3. make Windows binaries
    echo 4. RPMs: https://build.opensuse.org/project/show?project=home%3Awojdyr
#    echo 5. prepare DEBs in Launchpad  /obsolete/
    echo "6. git tag -a v$version -m 'version $version'; git push --tags"
    echo "   (and upload tarball and binaries)"
    echo 7. update webpage
    echo 8. announce
    echo 9. make Windows static lib for linking with fityk
    exit
fi

arg=$1

# 0. check version numbers and soname
if [ $arg -eq 0 ]; then
    echo version: $VERSION
    echo configure.ac:
    grep AC_INIT configure.ac
    echo in xylib/xylib.h:
    grep 'define XYLIB_VERSION' xylib/xylib.h
    echo in README.rst:
    grep '.tar.bz2' README.rst
    grep '.zip' README.rst
    echo in xylib/Makefile.am:
    grep 'version-info' xylib/Makefile.am
    echo in xylib_capi.py:
    grep libxy.so xylib_capi.py
    echo in README.rst:
    grep "\* $VERSION" README.rst
fi

# c. Coverity scan
if [ $arg = c ]; then
    make clean
    rm -r cov-int/ xylib-cov.tgz
    cov-build --dir cov-int make
    tar czf xylib.tgz cov-int
    echo upload to https://scan.coverity.com/projects/1742/builds/new
fi

# 1. test compilation from git
if [ $arg -eq 1 ]; then
    rm -rf git_copy
    mkdir git_copy
    cd git_copy
    git clone ../../xylib
    cd xylib
    autoreconf -i || exit 1
    ./configure --disable-static && make distcheck || exit 1
    ./configure --prefix=`pwd`/install_dir --disable-shared --with-gui && \
        make install \
        || exit 1
    install_dir/bin/xyconv -v || exit 1
fi

# 2. make tarball
if [ $arg -eq 2 ]; then
    gunzip git_copy/xylib/xylib-*.tar.gz
    bzip2 git_copy/xylib/xylib-*.tar
    mv git_copy/xylib/xylib-*.tar.bz2 . && echo OK
    #make dist-bzip2
fi

# 3. make Windows binaries 
if [ $arg -eq 3 ]; then
    mkdir -p xylib_win-$VERSION/docs
    cp -r git_copy/xylib/install_dir/include/xylib/ xylib_win-$VERSION
    rm index.html
    #make index.html
    rst2html --stylesheet=web.css --template=web-tmpl.txt README.rst index.html
    cp index.html README.dev TODO COPYING sample-urls xylib_win-$VERSION/docs
    echo copy files xylib-*.dll and xyconv.exe to `pwd`/xylib_win-$VERSION
    echo and do:  zip -r xylib_win-$VERSION.zip xylib_win-$VERSION/
fi

# 5. prepare DEBs in Launchpad  /obsolete/
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

# 6. GitHub file release
if [ $arg -eq 6 ]; then
    echo "do: git tag -a v$version -m 'version $version'; git push --tags"
fi

# 7. update webpage
if [ $arg -eq 7 ]; then
    make index.html
    scp index.html `whoami`,xylib@web.sourceforge.net:htdocs/
fi

# 8. announce
if [ $arg -eq 8 ]; then
    grep -5 "\* $VERSION" README.rst | tail -6
    echo maybe post to http://fityk-announce.nieto.pl/
fi

# 9. make Windows static lib for linking with fityk
if [ $arg -eq 9 ]; then
    rm -rf mingw32-build
    mkdir mingw32-build
    cd mingw32-build
    MDIR=$HOME/local/mingw32
    ../configure --host=i686-w64-mingw32 --enable-static --enable-shared \
                 --with-pic --without-gui \
	         CPPFLAGS="-I$HOME/local/src/boost_1_50_0/ -I$MDIR/include/" \
		 CXXFLAGS="-O3" LDFLAGS="-s -L$MDIR/lib" --without-bzlib \
		 --prefix=$MDIR && \
    make install
fi

