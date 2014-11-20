

#include <Library/UefiBootServicesTableLib.h>

#include <Library/HiiLib.h>

#include <Protocol/FormBrowser2.h>
#include <Protocol/HiiDatabase.h>

//below extern will be built out by VFR compiler.
extern unsigned char  HiiMenuStrings[];
extern UINT8  HiiForm1VfrBin[];