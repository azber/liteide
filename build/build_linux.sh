#!/bin/sh

export BUILD_ROOT=$PWD

if [ -z $LITEIDE_ROOT ]; then
	export LITEIDE_ROOT=$PWD/../liteidex
fi

echo build liteide 
echo QTDIR=$QTDIR
echo GOROOT=$GOROOT
echo BUILD_ROOT=$BUILD_ROOT
echo LITEIDE_ROOT=$LITEIDE_ROOT
echo .

if [ -z $QTDIR ]; then
	echo 'error, QTDIR is null'
	exit 1
fi

export PATH=$QTDIR/bin:$PATH

echo qmake liteide ...
echo .
qmake $LITEIDE_ROOT -spec linux-g++ "CONFIG+=release"

if [ $? -ge 1 ]; then
	echo 'error, qmake fail'
	exit 1
fi

echo make liteide ...
echo .
make

if [ $? -ge 1 ]; then
	echo 'error, make fail'
	exit 1
fi

go version
if [ $? -ge 1 ]; then
	echo 'error, not find go in PATH'
	exit 1
fi

echo build liteide tools ...
cd $LITEIDE_ROOT
export GOPATH=$PWD

#go install -ldflags "-s -r ." -v liteidex
go install -ldflags "-s" -v tools/goastview
go install -ldflags "-s" -v tools/godocview
go install -ldflags "-s" -v tools/goexec
go install -ldflags "-s" -v tools/goapi

if [ $? -ge 1 ]; then
	echo 'error, go install fail'
	exit 1
fi

echo deploy ...

cd $BUILD_ROOT

rm -r liteide
mkdir -p liteide
mkdir -p liteide/bin
mkdir -p liteide/share/liteide
mkdir -p liteide/lib/liteide/plugins

cp -a -v $LITEIDE_ROOT/LICENSE.LGPL liteide
cp -a -v $LITEIDE_ROOT/LGPL_EXCEPTION.TXT liteide
cp -a -v $LITEIDE_ROOT/../README.md liteide

cp -a -v $LITEIDE_ROOT/liteide/bin/* liteide/bin
cp -a -v $LITEIDE_ROOT/bin/* liteide/bin
cp -a -v $LITEIDE_ROOT/liteide/lib/liteide/plugins/*.so liteide/lib/liteide/plugins

cp -r -v $LITEIDE_ROOT/deploy/* liteide/share/liteide/
cp -r -v $LITEIDE_ROOT/os_deploy/linux/* liteide/share/liteide/

cp -a -v $QTDIR/lib/libQtCore.so* liteide/lib/liteide
cp -a -v $QTDIR/lib/libQtGui.so* liteide/lib/liteide
cp -a -v $QTDIR/lib/libQtXml.so* liteide/lib/liteide
