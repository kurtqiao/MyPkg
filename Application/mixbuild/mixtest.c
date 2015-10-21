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

  support getopt(), refer to: edk2\AppPkg\Applications\OrderedCollectionTest
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
#include <unistd.h> // getopt()

#ifdef DOS_APP
#include <libgen.h>  //basename()

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


typedef struct _action
{
  unsigned char help :1;
  unsigned char shutdown :1;
  unsigned char reboot :1;
  unsigned char wakeup :1;
}ACTION;

int
print_usage(char *prog_name, FILE *channel, int ret_val)
{
  fprintf(channel, "\nusage: %s [-x | -x xxx | -x xxx]\n", prog_name);
  fprintf(channel, "Options\n");
  fprintf(channel, "-x      xx1\n");
  fprintf(channel, "-x xxx  parameter1 x xxx\n");
  fprintf(channel, "-x xxx  parameter2 x xxx\n");
  return ret_val;
}

unsigned char read_cmos(unsigned char index)
{
  unsigned char data;
  out_byte(0x70, index);
  data = in_byte(0x71);
  return data;

}

static int
parsing_commands (
  int  argc,
  char **argv,
  ACTION *action
  )
{
  int option;

  while ((option = getopt (argc, argv, ":hst:w:")) !=-1)
  {
   switch(option)
   {
    default:
//      fprintf(stderr, "internal error: unknown case %c\n", optopt);
      return EXIT_FAILURE;

    #ifdef DOS_APP
      // Remove below when in edk2 build
      // The current "compatibility" getopt() implementation doesn't support optopt
    case '?':
      fprintf(stderr, "error: unknown option: -%c\n", optopt);
      break;
    case ':':
      fprintf(stderr, "error: missing argument for option -%c\n", optopt);
      break;
    #endif

    case 'h':
      action->help = 1;
      break;
    }
    }

  return 0;
}

int
main(int argc, char * argv[])
{

  ACTION action = {0};


  parsing_commands(argc, argv, &action);

  if (action.help)
    print_usage(basename(argv[0]), stderr, EXIT_SUCCESS);

  printf("read cmos value = %x\n", read_cmos(0) );

  return 0;
}
