#! /bin/bash
# Create icon set for macOS, requires "brew install librsvg"
mkdir MyIcon.iconset
rsvg-convert -h 16 -d 72 -p 72 images/scalable/zint-qt.svg > MyIcon.iconset/icon_16x16.png
rsvg-convert -h 32 -d 144 -p 144 images/scalable/zint-qt.svg > MyIcon.iconset/icon_16x16@2x.png
rsvg-convert -h 32 -d 72 -p 72 images/scalable/zint-qt.svg > MyIcon.iconset/icon_32x32.png
rsvg-convert -h 64 -d 144 -p 144 images/scalable/zint-qt.svg > MyIcon.iconset/icon_32x32@2x.png
rsvg-convert -h 128 -d 72 -p 72 images/scalable/zint-qt.svg > MyIcon.iconset/icon_128x128.png
rsvg-convert -h 256 -d 144 -p 144 images/scalable/zint-qt.svg > MyIcon.iconset/icon_128x128@2x.png
rsvg-convert -h 256 -d 72 -p 72 images/scalable/zint-qt.svg > MyIcon.iconset/icon_256x256.png
rsvg-convert -h 512 -d 144 -p 144 images/scalable/zint-qt.svg > MyIcon.iconset/icon_256x256@2x.png
rsvg-convert -h 512 -d 72 -p 72 images/scalable/zint-qt.svg > MyIcon.iconset/icon_512x512.png
rsvg-convert -h 1024 -d 144 -p 144 images/scalable/zint-qt.svg > MyIcon.iconset/icon_512x512@2x.png
iconutil -c icns MyIcon.iconset
# rm -R MyIcon.iconset
