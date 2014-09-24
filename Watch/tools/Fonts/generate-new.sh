#!/bin/sh
rm -Rf *.c
#../grlib/ftrasterize/ftrasterize -f Red -s 13  "C&C Red Alert [LAN].ttf"
#../grlib/ftrasterize/ftrasterize -f Baby -s 16  babyblue.ttf
#../grlib/ftrasterize/ftrasterize -f Baby -s 12  babyblue.ttf
#../grlib/ftrasterize/ftrasterize -f Baby -s 12  babyblue.ttf
../grlib/ftrasterize/ftrasterize -f Gothic -s 14  RasterGothic14Cond.otf
#../grlib/ftrasterize/ftrasterize -f Gothic -s 14    "News Gothic Light.ttf"
#../grlib/ftrasterize/ftrasterize -f Gothic -s 14 -b "News Gothic Bold.ttf"
../grlib/ftrasterize/ftrasterize -f Gothic -s 18    "RasterGothic18Cond.otf"
../grlib/ftrasterize/ftrasterize -f Gothic -s 18 -b "RasterGothic18CondBold.otf"
../grlib/ftrasterize/ftrasterize -f RobotoCondensed -s 18 -b RobotoCondensed-Regular.ttf
#../grlib/ftrasterize/ftrasterize -f Gothic -s 24    "News Gothic Light.otf"
../grlib/ftrasterize/ftrasterize -f Gothic -s 24 -b "RasterGothic24CondBold.otf"
../grlib/ftrasterize/ftrasterize -f Gothic -s 28    "RasterGothic28Cond.otf"
../grlib/ftrasterize/ftrasterize -f Gothic -s 28 -b "RasterGothic28CondBold.otf"
../grlib/ftrasterize/ftrasterize -f Driod -s 28 -b DroidSerif-Bold.ttf

echo "Generate digit numbers"
../grlib/ftrasterize/ftrasterize -f Nimbus -s 30 -v "URW++ - NimbusSanNov-Hea.otf"
../grlib/ftrasterize/ftrasterize -f Nimbus -s 34 -v "URW++ - NimbusSanNov-Hea.otf"

../grlib/ftrasterize/ftrasterize -f Nimbus -s 38 -v -n -p 48 -e 58 "URW++ - NimbusSanNov-Hea.otf"
../grlib/ftrasterize/ftrasterize -f Nimbus -s 40 -v -n -p 48 -e 58 "URW++ - NimbusSanNov-Hea.otf"
../grlib/ftrasterize/ftrasterize -f Nimbus -s 46 -v -n -p 48 -e 58 "URW++ - NimbusSanNov-Hea.otf"
../grlib/ftrasterize/ftrasterize -f Nimbus -s 50 -v -n -p 48 -e 58 "URW++ - NimbusSanNov-Hea.otf"
../grlib/ftrasterize/ftrasterize -f Nimbus -s 52 -v -n -p 48 -e 58 "URW++ - NimbusSanNov-Hea.otf"
../grlib/ftrasterize/ftrasterize -f Nimbus -s 91 -v -n -p 48 -e 58 "URW++ - NimbusSanNov-Hea.otf"

../grlib/ftrasterize/ftrasterize -f Icon -s 16 -v icons_16x16_all.pbm
../grlib/ftrasterize/ftrasterize -f Icon -s 32 -v icons_32x32_all.pbm
../grlib/ftrasterize/ftrasterize -f Icon -s 48 -v icons_48x48_all.pbm


# generate digit clock fonts
#../grlib/ftrasterize/ftrasterize -f Digit -s 44 -v -n -p 48 -e 58 msgothic.ttc
#../grlib/ftrasterize/ftrasterize -f Digit -s 44 -v -n -b -p 48 -e 58 ProximaNova-Bold.otf
#../grlib/ftrasterize/ftrasterize -f Digit -s 52 -v -n -b -p 48 -e 58 ProximaNova-Bold.otf
#../grlib/ftrasterize/ftrasterize -f Digit -s 56 -v -n -p 48 -e 58 msgothic.ttc

../grlib/ftrasterize/ftrasterize -v -f unicode -s 18 -c unicode.txt -y -r -u wqy-microhei.ttc
