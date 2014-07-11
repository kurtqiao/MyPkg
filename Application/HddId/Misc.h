
#pragma once

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
//add below include for gST, gBS
#include "Library/UefiBootServicesTableLib.h"
//add for ZeroMem
#include <Library/BaseMemoryLib.h>
#include <Library/PciLib.h>
#include <IndustryStandard/Pci22.h>
#include <Protocol/PciIo.h>
#include <Library/IoLib.h>

#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/AtaPassThru.h>

#include "HddId.h"

EFI_STATUS
Print_Help(
    );

EFI_STATUS
DetectATAController(
  EFI_ATA_DEVICES *ATADevice
  );

EFI_STATUS
AHCI_HDD_Identify(
  UINT64 AHCI_PCI_ADDRESS,
  UINT8  *IdentyBuffer512B
  );

EFI_STATUS
IDE_HDD_Identify(
  EFI_ATA_DEVICES *ATA_Device,
  UINT16  *IdentyBufferWORD
  );