#!/bin/bash
rm -rf png;
mkdir png;

for f in *.svg ; do
    convert -background none -size 1024x1024 $f png/`basename $f .svg`.png
done
