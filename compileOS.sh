dd if=/dev/zero of=floppya.img bs=512 count=2880

nasm bootload.asm

dd if=bootload of=floppya.img bs=512 count=1 conv=notrunc
dd if=map.img of=floppya.img bs=512 count=1 seek=256 conv=notrunc
dd if=files.img of=floppya.img bs=512 count=1 seek=258 conv=notrunc
dd if=sectors.img of=floppya.img bs=512 count=1 seek=259 conv=notrunc

bcc -ansi -c -o kernel.o kernel.c
as86 kernel.asm -o kernel_asm.o
ld86 -o kernel -d kernel.o kernel_asm.o
dd if=kernel of=floppya.img bs=512 conv=notrunc seek=1

as86 lib.asm -o lib_asm.o

prog=shell
bcc -ansi -c -o $prog.o $prog.c
ld86 -o $prog -d $prog.o lib_asm.o
./loadFile $prog

prog=echo
bcc -ansi -c -o $prog.o $prog.c
ld86 -o $prog -d $prog.o lib_asm.o
./loadFile $prog

prog=ls
bcc -ansi -c -o $prog.o $prog.c
ld86 -o $prog -d $prog.o lib_asm.o
./loadFile $prog

prog=rm
bcc -ansi -c -o $prog.o $prog.c
ld86 -o $prog -d $prog.o lib_asm.o
./loadFile $prog

prog=mkdir
bcc -ansi -c -o $prog.o $prog.c
ld86 -o $prog -d $prog.o lib_asm.o
./loadFile $prog

prog=pwd
bcc -ansi -c -o $prog.o $prog.c
ld86 -o $prog -d $prog.o lib_asm.o
./loadFile $prog

prog=cat
bcc -ansi -c -o $prog.o $prog.c
ld86 -o $prog -d $prog.o lib_asm.o
./loadFile $prog

./loadFile keyproc2
