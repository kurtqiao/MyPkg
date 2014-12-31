
#include <Uefi.h>
#include <Protocol/BlockIo.h>
#include "Library/UefiBootServicesTableLib.h"
#include <Library/AppFrameWork.h>
#include <Library/AppCommon.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <IndustryStandard/Mbr.h>



EFI_STATUS
DumpDevicePath();

EFI_STATUS
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{

  EFI_STATUS                  Status;
  EFI_BLOCK_IO_PROTOCOL       *BlkIo;
  EFI_HANDLE                  *BlkIoHandle;
  UINT32                      BlockSize;
  UINTN                       Index;
  UINTN                       NoBlkIoHandles;
  EFI_DEVICE_PATH_PROTOCOL    *DevPath;
  CHAR16                      *DevPathString;
  EFI_PARTITION_TABLE_HEADER  *PartHdr;
  MASTER_BOOT_RECORD          *PMBR;
  
  //
  // Locate Handles that support BlockIo protocol
  //
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiBlockIoProtocolGuid,
                  NULL,
                  &NoBlkIoHandles,
                  &BlkIoHandle
                  );
  if (EFI_ERROR (Status)) {
  	return Status;
  }

  for (Index = 0; Index < NoBlkIoHandles; Index++) {
    Status = gBS->HandleProtocol (
                    BlkIoHandle[Index],
                    &gEfiBlockIoProtocolGuid,
                    (VOID **) &BlkIo
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }
    if(BlkIo->Media->LogicalPartition){  //if partition skip
    	continue;
    }
    DevPath = DevicePathFromHandle(BlkIoHandle[Index]);
    if (DevPath == NULL){
    	continue;
    }
    DevPathString = ConvertDevicePathToText(DevPath, TRUE, FALSE);
    Print(L"%s \nMedia Id: %d, device type: %x, SubType: %x, logical: %x\n", \
           DevPathString, BlkIo->Media->MediaId, DevPath->Type, DevPath->SubType, \
           BlkIo->Media->LogicalPartition);

    BlockSize = BlkIo->Media->BlockSize;
    PartHdr   = AllocateZeroPool (BlockSize);
    PMBR      = AllocateZeroPool (BlockSize);
    //read LBA0
    Status = BlkIo->ReadBlocks(
                   BlkIo,
                   BlkIo->Media->MediaId,
                   (EFI_LBA)0,							//LBA 0, MBR/Protetive MBR
                   BlockSize,
                   PMBR
                   );
    //read LBA1
    Status = BlkIo->ReadBlocks(
                   BlkIo,
                   BlkIo->Media->MediaId,
                   (EFI_LBA)1,							//LBA 1, GPT
                   BlockSize,
                   PartHdr
                   );
    //check if GPT
    if (PartHdr->Header.Signature == EFI_PTAB_HEADER_ID){
    	
      if (PMBR->Signature == MBR_SIGNATURE){
      	Print(L"Found Protetive MBR.\n");
      }
      Print(L"LBA 1,");
      Print(L"GPT:\n");
      //
      //you can add some parse GPT data structure here
      //
      AppPrintBuffer((UINT16 *)PartHdr);
      Print(L"\n");
    }else{
      if (PMBR->Signature == MBR_SIGNATURE){
       	Print(L"LBA 0,");
       	Print(L"MBR:\n");
       	AppPrintBuffer((UINT16 *)PMBR);
       	Print(L"\n");
       }
     }

    FreePool(PartHdr);
    FreePool(PMBR);
  }

  //debug dump device path
  //DumpDevicePath();
  return Status;
}

EFI_STATUS
DumpDevicePath()
{
  EFI_STATUS                Status;
  UINTN        			    HandlesCount;
  UINTN                     Index;
  EFI_HANDLE                *DevPathHandle;
  EFI_DEVICE_PATH_PROTOCOL  *DevPath;

  //
  // Locate Handles that support BlockIo protocol
  //
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiDevicePathProtocolGuid,
                  NULL,
                  &HandlesCount,
                  &DevPathHandle
                  );
  if (!EFI_ERROR (Status)) {
     Print(L"Device Path Protocol Handles: %d\n", HandlesCount);
     Print(L"Type SubType Length Str\n");
    for (Index = 0; Index < HandlesCount; Index++) {
    	Status = gBS->HandleProtocol (
                      DevPathHandle[Index],
                      &gEfiDevicePathProtocolGuid,
                      (VOID **) &DevPath
                      );
    	Print(L"%x   %x      %x     %s\n",DevPath->Type, DevPath->SubType, DevPath->Length, ConvertDevicePathToText(DevPath, TRUE, FALSE));
    }
  }
	return EFI_SUCCESS;
}
