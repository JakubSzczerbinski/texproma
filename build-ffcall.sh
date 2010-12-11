#!/bin/sh

CC="gcc -O2 -fomit-frame-pointer -D__x86_64__"

cd ffcall
./configure --build=x86_64-apple-darwin13.4.0 --host=x86_64-apple-darwin13.4.0

cd avcall
$CC -S avcall-x86_64.c -o avcall-x86_64.s
cd -

cd vacall
$CC -DHAVE_LONG_LONG_INT -S vacall-x86_64.c -o vacall-x86_64.s
cd -

cd callback/vacall_r
cp ../../vacall/vacall.h.in .
$CC -DHAVE_LONG_LONG_INT -DREENTRANT -S vacall-x86_64.c -o vacall-x86_64.s 
cd -

make

cd -
