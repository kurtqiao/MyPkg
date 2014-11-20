

#include "HiiMenu.h"
#include "HiiMenuGUIDs.h"

EFI_GUID  mHiiForm1Guid = HII_MENU_FORM1_GUID;

EFI_STATUS
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  EFI_STATUS                      Status;
  EFI_HII_HANDLE                  HiiHandle;
  EFI_HII_DATABASE_PROTOCOL       *HiiDatabase;
  EFI_FORM_BROWSER2_PROTOCOL      *FormBrowser2;

  //
  // Locate Hii Database protocol
  //
  Status = gBS->LocateProtocol (&gEfiHiiDatabaseProtocolGuid, NULL, (VOID **) &HiiDatabase);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Locate Formbrowser2 protocol
  //
  Status = gBS->LocateProtocol (&gEfiFormBrowser2ProtocolGuid, NULL, (VOID **) &FormBrowser2);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Publish our HII data
  //
  HiiHandle = HiiAddPackages (
                   &mHiiForm1Guid,
                   NULL,
                   HiiMenuStrings,
                   HiiForm1VfrBin,
                   NULL
                   );
  if (HiiHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //call SendForm to show
  Status = FormBrowser2->SendForm (
                           FormBrowser2,
                           &HiiHandle,
                           1,             //handle count
                           NULL,          //formsetguid optional
                           0,             //form id optional
                           NULL,          //screen optional
                           NULL           //action request optional
                           );
  HiiRemovePackages (HiiHandle);

  return Status;
}