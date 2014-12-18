/***************************************************************************
*
* boot managerr EFI variables
*
* 
 ***************************************************************************/
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include "Library/UefiRuntimeServicesTableLib.h"
#include "Guid/GlobalVariable.h"
#include <Library/MemoryAllocationLib.h>


#define EFI_NULL_GUID \
  { 0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} }

//BOOLEAN SHOW_VERBOSE = FALSE;
//BOOLEAN SET_BOOTORDER = FALSE;
//UINT16  BOOTNUM;

typedef struct{
  UINT16 bootnum;
  UINTN show_verbose : 1;
  UINTN set_bootorder : 1;
  UINTN  : 1;
  UINTN usage : 1;
}BOOTMGROPTS;

BOOTMGROPTS opts={0};

BOOLEAN
EFIAPI
IsCharDigit (
  IN CHAR16  Char
  )
{
  return (BOOLEAN) (Char >= L'0' && Char <= L'9')||(Char >= L'A' && Char <= L'F');
}

//refer to IntelFrameworkPkg/Lib/GenericBdsLib/BdsMisc.c BdsLibGetVariableAndSize
VOID *
EFIAPI
mGetVariable(
  IN  CHAR16              *Name,
  IN  EFI_GUID            *VendorGuid,
  OUT UINTN               *VariableSize,
  OUT UINT32              *Attr
  )
{
  EFI_STATUS  Status;
  UINTN       BufferSize;
  VOID        *Buffer;

  Buffer = NULL;

  //
  // Pass in a zero size buffer to find the required buffer size.
  //
  BufferSize  = 0;
  Status      = gRT->GetVariable (Name, VendorGuid, Attr, &BufferSize, Buffer);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    //
    // Allocate the buffer to return
    //
    Buffer = AllocateZeroPool (BufferSize);
    if (Buffer == NULL) {
      *VariableSize = 0;
      return NULL;
    }
    //
    // Read variable into the allocated buffer.
    //
    Status = gRT->GetVariable (Name, VendorGuid, Attr, &BufferSize, Buffer);
    if (EFI_ERROR (Status)) {
      FreePool (Buffer);
      BufferSize = 0;
      Buffer     = NULL;
    }
  }

  *VariableSize = BufferSize;
  return Buffer;
}
VOID
ParseOpt(
	UINTN Argc, 
	CHAR16 **Argv
	)
{
  UINTN Index;

  for (Index = 1; Index < Argc; Index ++) {
    if ((Argv[Index][0] != L'-') || (Argv[Index][2] != L'\0')) {
      return ;
    }

    switch (Argv[Index][1]) {
    
    case L'v':
      opts.show_verbose = TRUE;
      break;
    case L's':
      opts.set_bootorder = TRUE;
      opts.bootnum = (UINT16)StrHexToUintn(Argv[Index+1]);
      break;
    case L'h':
    case L'H':
    case L'?':
      opts.usage = 1;
      break;

    default:
      break;
     // Status = EFI_INVALID_PARAMETER;
    }
  }
}

VOID
Usage()
{
	Print(L"bootmgr ver 0.1\n");
	Print(L"[-v]   verbose display\n");
	Print(L"[-s bootnum] set first boot\n" );
	Print(L"[-h] help\n");
}
EFI_STATUS
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  EFI_STATUS    Status;
  UINT16        *BootVariable;
  UINTN         BootVariableSize;
  UINT16        LDAttr;
  CHAR16        *Name;
  UINTN         NewNameSize;
  UINTN         NameSize;
  UINTN         i;
  UINT32        Attr;
  EFI_GUID      VarGuid=EFI_NULL_GUID;
  

  ParseOpt(Argc, Argv);

  if (opts.usage){
  	Usage();
  	return EFI_SUCCESS;
  }
  if (opts.set_bootorder){
  	BootVariable = mGetVariable(L"BootOrder", &gEfiGlobalVariableGuid, &BootVariableSize, &Attr);
    BootVariable[0]=opts.bootnum;
    Status = gRT->SetVariable(L"BootOrder", &gEfiGlobalVariableGuid, Attr, BootVariableSize, BootVariable);
    if (!EFI_ERROR(Status))
      Print(L"Set first boot to Boot%04x\n", opts.bootnum);
    return Status;
  }
  //get BootCurrent
  BootVariable = mGetVariable(L"BootCurrent", &gEfiGlobalVariableGuid, &BootVariableSize, NULL);
  if (BootVariable != NULL)
    Print(L"BootCurrent: %04x\n", *BootVariable);
  //get BootOrder
  BootVariable = mGetVariable(L"BootOrder", &gEfiGlobalVariableGuid, &BootVariableSize, &Attr);
  if (BootVariable != NULL){
  	Print(L"BootOrder:  ");
    for(i=0; i<(BootVariableSize/2); i++){
    	Print(L" %04x ",BootVariable[i]);
    }
    Print(L"\n");
  }

  //Print all BOOT#### Load Options
  NameSize = sizeof(CHAR16);
  Name     = AllocateZeroPool(NameSize);
  for (i=0; ;i++ ){
  	NewNameSize = NameSize;
  	//search all EFI variables
  	Status = gRT->GetNextVariableName (&NewNameSize, Name, &VarGuid);
  	  if (Status == EFI_BUFFER_TOO_SMALL) {
      Name = ReallocatePool (NameSize, NewNameSize, Name);
      Status = gRT->GetNextVariableName (&NewNameSize, Name, &VarGuid);
      NameSize = NewNameSize;
    }
    //
    if (Status == EFI_NOT_FOUND) {
      break;
    }
    //skip if not Global variable
    if (!CompareGuid(&VarGuid, &gEfiGlobalVariableGuid))
    	continue;
    //check BOOT#### variable
    if(!StrnCmp(Name, L"Boot", 4) &&
    	IsCharDigit(Name[4]) && IsCharDigit(Name[5]) &&
    	IsCharDigit(Name[6]) && IsCharDigit(Name[7]))
    {
    	Print(L"%s:", Name);
        //get BOOT####
        BootVariable = mGetVariable(Name, &gEfiGlobalVariableGuid, &BootVariableSize, NULL);
        //print attribute
        LDAttr = BootVariable[0];
        if (opts.show_verbose){
        	i = 6;   //for adjust display
          if (LDAttr == 0)
        	Print(L"CB*", i--);     //category boot
          if (LDAttr & 1)
           	Print(L"A* ", i--);      //active
          if (LDAttr & 2)
           	Print(L"FR*", i--);     //force reconnect
          if (LDAttr & 8)
           	Print(L"H* ", i--);      //hidden
          if (LDAttr & 0x100)
           	Print(L"CA*", i--);     //category app
           //Print(L"\n");
           while (i--){
           	Print(L"   ");
           }
        }
        //print EFI_LOAD_OPTION description
        Print(L"   %s",(CHAR16 *)(BootVariable+3));
        Print(L"\n");
        
    }
  }

  return Status;
}
