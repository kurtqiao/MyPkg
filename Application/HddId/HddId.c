/*++

Copyright (c) 2014 Kurt Qiao

Module Name:

    HddId.c
    
Abstract:
    HddId app

Revision History

--*/


#include "HddId.h"
#include "Misc.h"
#include <Library/AppFrameWork.h>
#include <Library/AppCommon.h>

EFI_STATUS
InitializeFrameworkTable(
  IN UINTN Argc,
  IN CHAR16 **Argv
  );

EFI_APP_FRAMEWORK_TABLE gFT = {
  InitializeFrameworkTable,       //Init
  Print_Help,                     //AppPrintHelp
  {0},                            //Operations
  NULL,                           //PROTOCOL1  (save EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL)
  NULL,                           //PROTOCOL2  (save EFI_ATA_PASS_THRU_PROTOCOL)
  NULL                            //OP1 (not use)
};


EFI_STATUS
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
    EFI_STATUS  Status=EFI_SUCCESS;
    EFI_ATA_DEVICES *ATADevice=NULL;
    UINT8  tmpbuffer[512];
    UINT8  *bufferptr;

    //
    gFT.Init(Argc, Argv);
    //
    DetectATAController(ATADevice);

    ZeroMem(tmpbuffer, sizeof(UINT8)*512);
    bufferptr = tmpbuffer;

    if(gFT.Operations.APP_OPERATIONS_PRINT_TEXT){
      if (ATADevice->ATATypte == SATA_AHCI_1){
        //typically only one AHCI controller, so we not support more than one AHCI here
        AHCI_HDD_Identify(ATADevice->ATA_CONTROLLER_ADDRESS[0], bufferptr);
      }
      if (ATADevice->ATATypte == IDE_CONTROLLER){
        IDE_HDD_Identify(ATADevice, (UINT16 *)bufferptr);
      }

    }
    if(gFT.Operations.APP_OPERATIONS_PRINT_HELP){
      gFT.AppPrintHelp();
    }

    return Status;
}

EFI_STATUS
InitializeFrameworkTable(
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  EFI_STATUS Status;

  Status = ParseCommandLine(Argc, Argv, &gFT);
  if (EFI_ERROR(Status))
  {
    Print(L"Wrong parameters usage!\n");
    return Status;
  }
  //save protocol for future use, so we only locate it when program init
  Status = gBS->LocateProtocol(&gEfiPciRootBridgeIoProtocolGuid, 
                                NULL, 
                                (VOID**)&gFT.PROTOCOL1
                                );
  if(EFI_ERROR(Status)){
    Print(L"Locate PciRootBridgeIoProtocol Fail!\n");
    return Status;
  }

  Status = gBS->LocateProtocol(&gEfiAtaPassThruProtocolGuid, 
                                NULL, 
                                (VOID **) &gFT.PROTOCOL2 
                                );
  if (EFI_ERROR(Status)) {
    Print(L"Locate AtaPassThruProtocol Fail!\n");
    return Status;
  }

  return Status;
}

