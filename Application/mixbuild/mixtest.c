/*
  mixtest
  demo how to create a common program, which could be bulit as dos app by
  open watcom c and shell app by uefi edk2 build env
  
  1.DOS 16bit application:
  wasm -ms iolib.asm
  wcl -bt=dos mixtest.c iolib.obj

  2. 64bit UEFI application:
  add mixtest.inf into MyPkg.dsc
  build -a X64 -p MyPkg\MyPkg.dsc -t VS2013x86
*/


#ifdef EFI_SPECIFICATION_VERSION
#undef DOS_APP
#pragma message ( "define uefi" ) 
#else
#define DOS_APP
#pragma message ( "define dos" ) 
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef DOS_APP
extern int cdecl ADD_VAL(int, int);
extern unsigned char cdecl in_byte(int);
extern void cdecl out_byte(int, int);
#else
#include <Library/IoLib.h>

unsigned char
in_byte(int index)
{
  return IoRead8(index);
}

void
out_byte(UINT8 port, UINT8 data)
{
  IoWrite8(port, data);
}
#endif

unsigned char read_cmos(unsigned char index)
{
  unsigned char data;
  out_byte(0x70, index);
  data = in_byte(0x71);
  return data;

}


int
main(int argc, char * argv[])
{

  printf("read cmos value = %x\n", read_cmos(0) );

  return 0;
}
