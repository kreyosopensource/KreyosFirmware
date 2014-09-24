#!/bin/sh
../grlib/ftrasterize/ftrasterize -f Red -s 13  "C&C Red Alert [LAN].ttf"
../grlib/ftrasterize/ftrasterize -f Baby -s 16  babyblue.ttf
../grlib/ftrasterize/ftrasterize -f Baby -s 12  babyblue.ttf
../grlib/ftrasterize/ftrasterize -f Nova -s 13  ProximaNova-Regular.otf
../grlib/ftrasterize/ftrasterize -f Nova -s 16  ProximaNova-Regular.otf
../grlib/ftrasterize/ftrasterize -f Nova -s 38  ProximaNova-Regular.otf
../grlib/ftrasterize/ftrasterize -f Nova -s 12 -b  ProximaNova-Bold.otf
../grlib/ftrasterize/ftrasterize -f Nova -s 16 -b  ProximaNova-Bold.otf
../grlib/ftrasterize/ftrasterize -f Nova -s 38 -b -v ProximaNova-Bold.otf
../grlib/ftrasterize/ftrasterize -f Nova -s 50 -b  ProximaNova-Bold.otf
../grlib/ftrasterize/ftrasterize -f Icon -s 16 -v icons_16x16_all.pbm
../grlib/ftrasterize/ftrasterize -f Icon -s 32 -v icons_32x32_all.pbm
../grlib/ftrasterize/ftrasterize -f Icon -s 48 -v icons_48x48_all.pbm

../grlib/ftrasterize/ftrasterize -f Nova -s 28  ProximaNova-Regular.otf
../grlib/ftrasterize/ftrasterize -f Nova -s 28 -b  ProximaNova-Bold.otf

echo "Generate digit numbers"
# generate digit clock fonts
../grlib/ftrasterize/ftrasterize -f Digit -s 44 -v -n -p 48 -e 58 ProximaNova-Regular.otf
../grlib/ftrasterize/ftrasterize -f Digit -s 44 -v -n -b -p 48 -e 58 ProximaNova-Bold.otf
../grlib/ftrasterize/ftrasterize -f Digit -s 52 -v -n -b -p 48 -e 58 ProximaNova-Bold.otf
../grlib/ftrasterize/ftrasterize -f Digit -s 56 -v -n -p 48 -e 58 ProximaNova-Regular.otf

../grlib/ftrasterize/ftrasterize -v -f unicode -s 16 -c unicode.txt -y -r -u simhei.ttf gulim.ttc
