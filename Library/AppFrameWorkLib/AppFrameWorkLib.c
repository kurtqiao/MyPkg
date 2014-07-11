#include <AppFrameWorkLib.h>
#include <Library/AppFrameWork.h>

/*++
Routine Description:
  Parse the command line.

Returns:
  EFI_SUCCESS   - Successfully.
  OTHERS        - Something failed.
--*/
EFI_STATUS
EFIAPI
ParseCommandLine(
  IN UINTN Argc,
  IN CHAR16 **Argv,
  EFI_APP_FRAMEWORK_TABLE *FtHandle
  )
{
  EFI_STATUS  Status;
  UINTN       Index;

  Status = EFI_SUCCESS;
  if (Argc < 2) {  
    FtHandle->Operations.APP_OPERATIONS_PRINT_TEXT = 1;
    return Status;
  }
  //
  // Check all parameters
  //
  for (Index = 1; Index < Argc; Index ++) {
    if ((Argv[Index][0] != L'-') || (Argv[Index][2] != L'\0')) {
      return EFI_INVALID_PARAMETER;
    }

    
    switch (Argv[Index][1]) {
    
    case L'g':
    case L'G':
      //
      // Run in graphic menu mode
      FtHandle->Operations.APP_OPERATIONS_PRINT_MENU = 1;
      break;

    case L'h':
    case L'H':
    case L'?':
      FtHandle->Operations.APP_OPERATIONS_PRINT_HELP = 1;
      break;

    default:
      Status = EFI_INVALID_PARAMETER;
    }
  }

  //
  // Done
  //
  return Status;
}

