#include <AppCommonLib.h>
#include <Library/AppFrameWork.h>
#include <Library/AppCommon.h>




EFI_STATUS
AppPrintBuffer(
  UINT16  *Buffer
  )
{
  UINTN   i;

  for (i = 0; i<=0xFF; i++){
    if((i%10)==0){
      if (i != 0);
        Print(L"\n");
      Print(L"%.3d: ", i);
    }

    Print(L"%.4X ", Buffer[i]);
  }
  return EFI_SUCCESS;
}

/*
Search a given PCI classcode and return it's PCI address
*/
BOOLEAN
EFIAPI
AppProbePCIByClassCode(
  UINT8   mBassCode,
  UINT8   mSubClass,
  UINT8   *mInterface OPTIONAL,
  UINT64  *mPCI_ADDRESS
  )
{
  EFI_STATUS  Status;
  BOOLEAN     Found;
  UINTN       HandleCount;
  UINTN       ThisBus;
  UINTN       Bus, Dev, Func;
  UINTN       Seg, Index;
  UINTN       Count;
  
  PCI_DEVICE_INDEPENDENT_REGION *Hdr;
  PCI_TYPE_GENERIC              PciHeader;
  EFI_PCI_IO_PROTOCOL           *Pci;
  EFI_HANDLE  *HandleBuffer;

  Found = FALSE;
  ZeroMem(mPCI_ADDRESS, sizeof(EFI_APP_FRAMEWORK_TABLE)*APP_MAX_CONTROLLER);
  //init
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiPciIoProtocolGuid, \
                                    NULL, &HandleCount, &HandleBuffer);
  if (EFI_ERROR(Status)){
    Print(L"No PCI devices found!!\n");
    return FALSE;
  }
  Count = 0;
  
  for (ThisBus = 0; ThisBus <= PCI_MAX_BUS; ThisBus++){
    for (Index = 0; Index < HandleCount; Index++){
      Status = gBS->HandleProtocol(HandleBuffer[Index], &gEfiPciIoProtocolGuid, (VOID **)&Pci);
      if(!EFI_ERROR(Status)){
        Pci->GetLocation(Pci, &Seg, &Bus, &Dev, &Func);
        if (ThisBus != Bus)
          continue;
        Status = Pci->Pci.Read(Pci, EfiPciWidthUint32, 0, sizeof(PciHeader)/sizeof(UINT32), &PciHeader);
        if (!EFI_ERROR(Status)){
          Hdr = &PciHeader.Bridge.Hdr;
          
          if ((Hdr->ClassCode[2] == mBassCode) && (Hdr->ClassCode[1] == mSubClass)){

              if (mInterface != NULL){
                if (Hdr->ClassCode[0] == *mInterface){
                  mPCI_ADDRESS[Count++] = PCI_LIB_ADDRESS(Bus, Dev, Func, 0);
                  Found = TRUE;
                }
              }else{
                  mPCI_ADDRESS[Count++] = PCI_LIB_ADDRESS(Bus, Dev, Func, 0);
                  Found = TRUE;

              }
          } //endif ((Hdr->ClassCode[2] == mBassCode)
          
        } //endif (!EFI_ERROR(Status))
      }  //endif (!EFI_ERROR(Status)) 1st
    }//for HandleCount
  }//for ThisBus
  
  return Found;
}


EFI_STATUS
AppAhciIdentify(
  EFI_ATA_PASS_THRU_PROTOCOL *ATApt,
  UINT16 Port,
  UINT16 PortMultiplier,
  UINT8  *Buffer512B
  )
{
  EFI_STATUS Status;
  EFI_ATA_PASS_THRU_COMMAND_PACKET Packet;
  
  
  EFI_ATA_STATUS_BLOCK Asb;
  EFI_ATA_COMMAND_BLOCK Acb;


  Status = EFI_SUCCESS;
  ZeroMem(Buffer512B, sizeof(512*sizeof(UINT8)));
  //
  //  Initialize Packet buffer.
  //
  ZeroMem ((char *)&Packet, sizeof(Packet));
  ZeroMem ((char *)&Acb, sizeof(Acb));
  ZeroMem ((char *)&Asb, sizeof(Asb));
  Packet.Asb = &Asb;
  Packet.Acb = &Acb;

  Packet.InDataBuffer = Buffer512B;
  Packet.InTransferLength = 512;
  Packet.Protocol = EFI_ATA_PASS_THRU_PROTOCOL_PIO_DATA_IN;
  Packet.Length = EFI_ATA_PASS_THRU_LENGTH_BYTES;
  Packet.Timeout = ATA_PASS_THRU_TIMEOUT;
  Acb.AtaCommand = ATA_CMD_IDENTIFY;

  Status = ATApt->PassThru(ATApt, Port, PortMultiplier, &Packet, NULL);
  if (EFI_ERROR(Status)){
    Acb.AtaCommand = ATAPI_CMD_IDENTIFY;    //for ATAPI Indentify(ODD)
    Status = ATApt->PassThru(ATApt, Port, PortMultiplier, &Packet, NULL);
  }

  return Status;
}
