
#pragma once
#include <Library/AppCommonLib/AppCommonLib.h>

#define ATA_PASS_THRU_SECOND_UNIT   (1000 * 1000 * 10) // Timeout for one second.
#define ATA_PASS_THRU_TIMEOUT       (ATA_PASS_THRU_SECOND_UNIT * 10) // Timeout definition.
#define ATA_CMD_IDENTIFY          	0xEC
#define ATAPI_CMD_IDENTIFY        	0xA1

EFI_STATUS
AppPrintBuffer(
  UINT16  *Buffer
  );

BOOLEAN
EFIAPI
AppProbePCIByClassCode(
  UINT8   mBassCode,
  UINT8   mSubClass,
  UINT8   *mInterface OPTIONAL,
  UINT64  *mPCI_ADDRESS
  );

EFI_STATUS
AppAhciIdentify(
  EFI_ATA_PASS_THRU_PROTOCOL *ATApt,
  UINT16 Port,
  UINT16 PortMultiplier,
  UINT8  *Buffer512B
  );