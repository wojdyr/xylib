#!/bin/sh

cd "$(dirname "$0")"
diff="diff -U2"

for file in uplot.h uplot.cpp xybrowser.h xybrowser.cpp; do
    $diff $file ../../fityk/wxgui/$file
done
