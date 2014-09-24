#!/bin/sh
#pawncc script1.p -i:pawnscript/include  -O3 -S64 -C32 -v2 -d0
pawncc script1.p -i:pawnscript/include  -O1 -S256 -C16 -v2 -d0
tools/bin2c -o script1.h script1.amx