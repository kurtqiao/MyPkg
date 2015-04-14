#include "Misc.h"
#include <Library/AppCommon.h>
#include <Library/AppFrameWork.h>

extern EFI_APP_FRAMEWORK_TABLE gFT;

EFI_STATUS
Print_Help(
    )
{
    Print(L"HDD/ODD Identify. Copyright@2014. Version: %s\n", APP_VERSION);
    Print(L"Print ATA/ATAPI devices identify data, support AHCI and IDE controller.\n");
    Print(L"Usage:\n");
    Print(L"-h/-H/-?  Print help\n");

    return EFI_SUCCESS;
}

VOID
PrintPCIAdress(UINT64 PCI_ADDRESS)
{
  UINT8 BUS, DEV, FUNC;
/*
PCI_LIB_ADDRESS(Bus,Device,Function,Register)   \
  (((Register) & 0xfff) | (((Function) & 0x07) << 12) | (((Device) & 0x1f) << 15) | (((Bus) & 0xff) << 20))
*/
  BUS = (UINT8)(PCI_ADDRESS >> 20)&0xFF;
  DEV = (UINT8)(PCI_ADDRESS >> 15)&0x1F;
  FUNC = (UINT8)(PCI_ADDRESS >> 12)&0x07;
  Print(L"BUS:%d DEV:%d FUNC:%d",BUS, DEV, FUNC);

}

EFI_STATUS 
hdd_wait_status(UINT32 idePort)
{
    EFI_STATUS Status;
    UINT8      atasts;
    int i;
  
    Status = EFI_SUCCESS;
  
    for (i = 0; i < 1000; i++) {
      atasts = IoRead8(idePort+ATA_REG_STATUS);
        if ((atasts & (ATA_SR_BSY | ATA_SR_DRQ)) == ATA_SR_DRQ)
            return EFI_SUCCESS;
        if (atasts & (ATA_SR_ERR | ATA_SR_DF))
            Status = EFI_DEVICE_ERROR;
        if (!atasts)
            Status = EFI_NOT_READY;

        gBS->Stall(100);
    }

    return Status;
}

EFI_STATUS
GET_IDE_BaseAddress(
  EFI_ATA_DEVICES *IDE_Device
)
{
  EFI_STATUS Status;
  UINT8      ctlr;
  UINT8      tmpval;
  UINT8      Count = 0;
  UINT64     IDE_PCI_ADDRESS;
  
  Status = EFI_SUCCESS;
  //init
  ZeroMem(IDE_Device->IDE_DEVICES,sizeof(EFI_IDE_DEVICES)*MAX_SUPPORT_DRIVES);
  
  for (ctlr = 0; ctlr < MAX_SUPPORT_CONTROLLER; ctlr++){
    IDE_PCI_ADDRESS = IDE_Device->ATA_CONTROLLER_ADDRESS[ctlr];
    if ( IDE_PCI_ADDRESS!= 0) {
      //1. check PCI config offset 0x09 Programming Interface Register, Legacy mode 0 or Native mode 1
      //   bit0 for Primary mode native enable, bit2 for Secondary mode native enable
      //   Note: PNE and SNE must be set by simultaneously, also in theory these bits can be programmed separately but HW not support!!
      //   so we just need to check PNE here
      tmpval = PciRead8((UINTN)IDE_PCI_ADDRESS + 0x09);
      if (tmpval&BIT0){ 
        // if in native PCI mode, get base port and control port in regs offset 0x10~0x1F
        IDE_Device->IDE_DEVICES[Count].Channel = ATA_PRIMARY;
        //Primary cmd base address 0x10 and not 0x07
        IDE_Device->IDE_DEVICES[Count].BasePort = (PciRead32((UINTN)IDE_PCI_ADDRESS + PRIMARY_CMD_BASEADDR)&(~0x07));
        //Primary control base address 0x14 and not 0x03
        IDE_Device->IDE_DEVICES[Count].ContrlPort = (PciRead32((UINTN)IDE_PCI_ADDRESS + PRIMARY_CTRL_BASEADDR)&(~0x03));
        Count++;

        IDE_Device->IDE_DEVICES[Count].Channel = ATA_SECONDARY;
        //Sencondary cmd base address 0x18 and not 0x07
        IDE_Device->IDE_DEVICES[Count].BasePort = (PciRead32((UINTN)IDE_PCI_ADDRESS + SECONDARY_CMD_BASEADDR)&(~0x07));
        //Sencondary control base address 0x1C and not 0x03
        IDE_Device->IDE_DEVICES[Count].ContrlPort = (PciRead32((UINTN)IDE_PCI_ADDRESS + SECONDARY_CTRL_BASEADDR)&(~0x03));
        Count++;
      }
      else{
        // if in legacy PCI mode, get base port and control port is fixed
        // Primary 0x1F0~0x1F7, Primary ctrl 0x3F6; Secondary 0x170~0x176, Secondary ctrl 0x376
        IDE_Device->IDE_DEVICES[Count].Channel = ATA_PRIMARY;
        //Primary cmd base address 0x1F0
        IDE_Device->IDE_DEVICES[Count].BasePort = LEGACY_PRIMARY_CMD_BASEADDR;
        //Primary control base address 0x14 and not 0x03
        IDE_Device->IDE_DEVICES[Count].ContrlPort = LEGACY_PRIMARY_CTRL_BASEADDR;
        Count++;

        IDE_Device->IDE_DEVICES[Count].Channel = ATA_SECONDARY;
        //Sencondary cmd base address 0x18 and not 0x0E
        IDE_Device->IDE_DEVICES[Count].BasePort = LEGACY_SECONDARY_CMD_BASEADDR;
        //Sencondary control base address 0x1C and not 0x03
        IDE_Device->IDE_DEVICES[Count].ContrlPort = LEGACY_SECONDARY_CTRL_BASEADDR;
        Count++;
      }

    }
  }

  //debug
  /*  if (DEBUG_SWITCH==2){
  do{
    Print(L"\nIDE Device %d: base:%x Control:%x Channel:%x",Count, IDE_Device->IDE_DEVICES[Count].BasePort,
           IDE_Device->IDE_DEVICES[Count].ContrlPort,
           IDE_Device->IDE_DEVICES[Count].Channel);
    }while(Count--);
  }*/
  return Status;
}

EFI_STATUS
DETECT_IDE_DEVICES(
  EFI_ATA_DEVICES *IDE_Device
)
{
  EFI_STATUS Status;
  UINT8 i;
  UINT32  basePort;

  for (i = 0; i<= MAX_SUPPORT_DRIVES; i++){
    basePort = IDE_Device->IDE_DEVICES[i].BasePort;
    if( basePort == 0)
      continue;
      //init detect as no devices
      IDE_Device->IDE_DEVICES[i].SelDrive = 0;
      IDE_Device->IDE_DEVICES[i].Present = FALSE;
    
      //write offset 0x06 with value 0xA0 and 0xB0 to detect devices
      //check offset 0x07 not 0 may need to modify, i found the value should be 0x50 if devices attached.(which means DSC&DRDY)
      IoWrite8(basePort + ATA_REG_HDDEVSEL, ATA_MASTER);
      Status = hdd_wait_status(basePort);
      if (!EFI_ERROR(Status)){
        if (IoRead8(basePort + ATA_REG_HDDEVSEL) == ATA_MASTER)
        {
        IDE_Device->IDE_DEVICES[i].SelDrive |= (ATA_MASTER << 8);
        IDE_Device->IDE_DEVICES[i].Present = TRUE;
         }
      }
      //check Slave channel
      IoWrite8(basePort + ATA_REG_HDDEVSEL, ATA_SLAVE);
      Status = hdd_wait_status(basePort);
      if (!EFI_ERROR(Status)){
        if (IoRead8(basePort + ATA_REG_HDDEVSEL) == ATA_SLAVE)
        {
        IDE_Device->IDE_DEVICES[i].SelDrive |= ATA_SLAVE;
        IDE_Device->IDE_DEVICES[i].Present = TRUE;
        }
      }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DetectATAController(
  EFI_ATA_DEVICES *ATADevice
  )
{
       UINT8 tmpinterface;
       EFI_STATUS  Status = EFI_SUCCESS;
       BOOLEAN    Found;

       ATADevice->ATATypte = UNKNOWN_TYPE;
       ZeroMem(ATADevice->ATA_CONTROLLER_ADDRESS, sizeof(UINT64)*MAX_SUPPORT_CONTROLLER);

       //detect AHCI controller
       tmpinterface = 0x01;
       Found = AppProbePCIByClassCode(0x01, 0x06, &tmpinterface, ATADevice->ATA_CONTROLLER_ADDRESS);
       
       if (!Found){
         //detect IDE controller
         Found = AppProbePCIByClassCode(0x01,0x01, 0x00, ATADevice->ATA_CONTROLLER_ADDRESS);
         if (Found){
            ATADevice->ATATypte = IDE_CONTROLLER;
           GET_IDE_BaseAddress(ATADevice);
           DETECT_IDE_DEVICES(ATADevice);
         }
       }else{
            ATADevice->ATATypte = SATA_AHCI_1;
       }
       
       return Status;
}


EFI_STATUS
HDD_Reset(EFI_IDE_DEVICES IDE_Device)
{
    EFI_STATUS Status;
    //ide reset
    IoWrite8(IDE_Device.ContrlPort + ATA_REG_CONTROL, 0x04);
    gBS->Stall(500);
    IoWrite8(IDE_Device.ContrlPort + ATA_REG_CONTROL, 0x02);
    gBS->Stall(2000);
    Status = hdd_wait_status(IDE_Device.BasePort);
    
    return Status;

}

EFI_STATUS
IDE_Identify_Cmd(
  EFI_IDE_DEVICES IDE_Device,
  UINT8   Drive,
  UINT8   ATA_CMD
)
{
   EFI_STATUS Status;
   
   Status = HDD_Reset(IDE_Device);
   if(EFI_ERROR(Status)){
    return Status;
    }
   //hdd select
   IoWrite8(IDE_Device.BasePort + ATA_REG_HDDEVSEL, Drive);
  
   gBS->Stall(500);
   //clear 2~5
   IoWrite8(IDE_Device.BasePort + ATA_REG_SECCOUNT, 0);
   gBS->Stall(500);
   IoWrite8(IDE_Device.BasePort + ATA_REG_LBA_LOW, 0);
   gBS->Stall(500);
   IoWrite8(IDE_Device.BasePort + ATA_REG_LBA_MED, 0);
   gBS->Stall(500);
   IoWrite8(IDE_Device.BasePort + ATA_REG_LBA_HIGH, 0);
   gBS->Stall(500);
   
   //send identify cmd
   IoWrite8(IDE_Device.BasePort + ATA_REG_COMMAND, ATA_CMD);
   gBS->Stall(1000);
   if (ATA_CMD == ATAPI_CMD_IDENTIFY)
    gBS->Stall(1000);
   
   Status = hdd_wait_status(IDE_Device.BasePort);

   return Status;

}

EFI_STATUS
SENT_IDE_Identify(
  EFI_IDE_DEVICES  IDE_Device,
  UINT8   Drive,    //Master or Slave
  UINT16  *IdentyBufferWORD
)
{
      EFI_STATUS Status;
      UINTN i;
      Status = EFI_SUCCESS;
      ZeroMem(IdentyBufferWORD, 256*sizeof(UINT16));
      
      Status = IDE_Identify_Cmd(IDE_Device,Drive,ATA_CMD_IDENTIFY);
      
      //check device error is kind of important here, because of found some AMD controller accept BYTE read data
      //if not check device error, may miss a WORD of identify data when read HDD on AMD controller
      if (Status == EFI_DEVICE_ERROR){
        if (IoRead8(IDE_Device.BasePort) == 0x14){
          Status = IDE_Identify_Cmd(IDE_Device,Drive,ATAPI_CMD_IDENTIFY);
        }
      }
      
      if ( !EFI_ERROR(Status)){
        for (i = 0; i<=0xFF; i++){
          IdentyBufferWORD[i] = IoRead16(IDE_Device.BasePort);
        }  
      }
      return Status;

}

EFI_STATUS
IDE_HDD_Identify(
  EFI_ATA_DEVICES *ATA_Device,
  UINT16  *IdentyBufferWORD
  )
{
  EFI_STATUS Status;
  UINT8    i;
  UINT64  IDE_PCI_ADDRESS;
  Status = EFI_SUCCESS;
        
  //print IDE controller address:
  for (i = 0; i < MAX_SUPPORT_CONTROLLER; i++){
    IDE_PCI_ADDRESS = ATA_Device->ATA_CONTROLLER_ADDRESS[i];
    if (IDE_PCI_ADDRESS != 0){
      Print(L"\nIDE controller  ");
      PrintPCIAdress(IDE_PCI_ADDRESS);
    }
  }
  Print(L"\n");
  for  (i = 0; i< MAX_SUPPORT_DRIVES; i++){
    if (ATA_Device->IDE_DEVICES[i].Present){
      if ((ATA_Device->IDE_DEVICES[i].SelDrive & 0xFF00) == 0xA000){ 
        Status = SENT_IDE_Identify(ATA_Device->IDE_DEVICES[i], 0xA0, IdentyBufferWORD);
        if (!EFI_ERROR(Status)){
          (ATA_Device->IDE_DEVICES[i].Channel == ATA_PRIMARY)?Print(L"\nPrimary "):Print(L"\nSecondary ");
          Print(L"Master");
          AppPrintBuffer(IdentyBufferWORD);
        }
      }
      if ((ATA_Device->IDE_DEVICES[i].SelDrive & 0x00FF) == 0x00B0){ 
        Status = SENT_IDE_Identify(ATA_Device->IDE_DEVICES[i], 0xB0, IdentyBufferWORD);
        if (!EFI_ERROR(Status)){
          (ATA_Device->IDE_DEVICES[i].Channel == ATA_PRIMARY)?Print(L"\nPrimary "):Print(L"\nSecondary ");
          Print(L"Slave");
          AppPrintBuffer(IdentyBufferWORD);
        }
      }
    }
  }//end for
        
        return Status;
}

EFI_STATUS
AHCI_HDD_Identify(
  UINT64 AHCI_PCI_ADDRESS,
  UINT8  *IdentyBuffer512B
  )
{
  EFI_STATUS Status=EFI_SUCCESS;
  UINT8      PortImplement;
  UINT16     Port;
  UINT32     AHCIBAR;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *PCIrb;
  EFI_ATA_PASS_THRU_PROTOCOL      *ATApt;

  PCIrb = (EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *)gFT.PROTOCOL1;
  ATApt = (EFI_ATA_PASS_THRU_PROTOCOL      *)gFT.PROTOCOL2;

  //get AHCI bar in offset 24h~27h
  AHCIBAR = PciRead32 ((UINTN)AHCI_PCI_ADDRESS+0x24);
  Print(L"AHCI controller 1.0 at ");
  PrintPCIAdress(AHCI_PCI_ADDRESS);

  Print(L"\nAHCI bar = %x \n", AHCIBAR);

  Status = PCIrb->Mem.Read(PCIrb, EfiPciWidthUint8, \
                          (UINT64)(UINTN)(AHCIBAR+0x0C), 1, &PortImplement);

  for (Port = 0; Port <= MAX_SUPPORT_PORTS; Port++){
    if (PortImplement&1){
      
      ZeroMem(IdentyBuffer512B, sizeof(512*sizeof(UINT8)));
      
      AppAhciIdentify(ATApt, Port, 0, IdentyBuffer512B);

      if (!EFI_ERROR(Status)){
      Print(L"\n\nPort %d Identify data:\n", Port);
      AppPrintBuffer((UINT16 *)IdentyBuffer512B);
      }

    }
    PortImplement >>= 1;
  }
  return Status;
}