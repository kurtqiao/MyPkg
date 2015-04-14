/*++

Copyright (c) 2014 Kurt Qiao

Module Name:

    AHCI.c
    
Abstract:
    a program help to understan AHCI operation.
simply get AHCI device identify data

Revision History

--*/


#include "AHCI.h"
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/AppCommon.h>
#include <Library/PciLib.h>
#include "Library/UefiBootServicesTableLib.h"

HBA_MEM  *abar;
BOOLEAN EjectODD = FALSE;
/*VOID
ResetAhciEngine(
    HBA_MEM  *abar
    );*/

EFI_STATUS
AhciIssueCmd(
    HBA_PORT *port,
    UINT32 cmdslot);

VOID
BuildH2DFis(
    FIS_REG_H2D *fish2d, 
    UINT8 cmd
    );

EFI_STATUS
ATAPIEjectODD(
   HBA_MEM  *abar,
   FIS_REG_H2D *fish2d,
   HBA_CMD_HEADER *cmdlist,
   HBA_CMD_TBL *cmdheader
    );

EFI_STATUS
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
	
    EFI_STATUS  Status=EFI_SUCCESS;
	UINT32      ip;
	UINTN       i, PortAddr;
	DATA_64     Data64;
    UINT64      *AhciPciAddr=NULL;

    HBA_PORT *port;
    UINT8 *tmpbuffer, classinter, cmd;                 
    HBA_CMD_HEADER *cmdlist;
    HBA_CMD_TBL *cmdheader;
    FIS_REG_H2D *fish2d;

    //use AHCI.efi -e to eject ODD
    if (Argv[1][1] == L'e'){
        EjectODD = TRUE;
    }
    ZeroMem(AhciPciAddr, sizeof(UINT64));
    //get AHCI controller
    classinter = 0x01;
    if (!AppProbePCIByClassCode(0x01, 0x06,&classinter, AhciPciAddr))
        return EFI_NOT_FOUND;

    //read AHCI bar
    abar = (HBA_MEM  *)(UINTN)(PciRead32((UINTN)(*AhciPciAddr+0x24)));
    
    //make sure AHCI enable
    if (!(abar->ghc & BIT31))
        abar->ghc |= BIT31;

    //allocate buffer for read return data
    tmpbuffer = AllocateZeroPool(sizeof(UINT8)*512);

    //allocate alignment memory for command table
    cmdheader = AllocatePages(sizeof(HBA_CMD_TBL));
    ZeroMem(cmdheader, sizeof(HBA_CMD_TBL));

    //allocate h2d FIS
    fish2d = AllocateZeroPool(sizeof(FIS_REG_H2D));

    //allocate align memory address for command list, PxCLB bit9:0 is reserved
    cmdlist = AllocatePages(sizeof(HBA_CMD_HEADER));
    ZeroMem(cmdlist, sizeof(HBA_CMD_HEADER));

    if (EjectODD){
        Status = ATAPIEjectODD(abar, fish2d, cmdlist, cmdheader);
        Print(L"SATA AHCI Mode ODD eject!\n");
        return Status;
    }

    Print(L"AHCI controller address: %x\n", *AhciPciAddr);
    Print(L"AHCI bar: %x\n", abar);
    Print(L"Host Capabilities: %x ; Global Host Control: %x\n", abar->cap, abar->ghc);
    Print(L"Port Implemented: %x \n", abar->pi);
    //scan port for ATA/ATAPI device and print identify data
    cmd = ATA_CMD_IDENTIFY;
    //check implemented port
    for (ip = abar->pi, i=0; ip>0;ip>>=1, i++){

    	if (ip&1){

    		PortAddr = (UINTN)abar + 0x100 + 0x80*i;
    		Print(L"\nport: %d, address: %x\n", i, PortAddr);
    		port = (HBA_PORT *)(UINTN) PortAddr;
            if (port->sig == ATADRIVE)
                cmd = ATA_CMD_IDENTIFY;
            if (port->sig == ATAPIDRIVE)
                cmd = ATAPI_CMD_IDENTIFY;
            
            BuildH2DFis(fish2d, cmd);
            CopyMem(cmdheader->cfis, fish2d, sizeof(FIS_REG_H2D));
            cmdheader->prdt_entry[0].dba = (UINT32)(UINTN) tmpbuffer;
            cmdheader->prdt_entry[0].dbc = 0x1ff;
            cmdheader->prdt_entry[0].i = 1;

            cmdlist->cfl = 5;
            cmdlist->prdtl = 1;
            cmdlist->ctba = (UINT32)(UINTN) cmdheader;

            Data64.Uint64 = (UINTN)cmdlist;
    		port->clb = Data64.Uint32.Lower32;
    		port->clbu = Data64.Uint32.Upper32;

            Status = AhciIssueCmd(port, BIT0);
            if (EFI_ERROR(Status)){
                Print(L"Issue cmd fail! Need to Kick AHCI engine\n");
                continue;
            }
            AppPrintBuffer((UINT16 *)tmpbuffer);
            Print(L"\n");
    	}
    }

    return Status;
}

/*VOID
ResetAhciEngine(
    HBA_MEM  *abar
    )
{
    UINTN i;
    Print(L"\n!!kick the engine!!\n");
    abar->ghc |= BIT0;
    //wait cmd
    for(i=0; i<100; i++){
        gBS->Stall(500);
        if (!(abar->ghc & BIT0))
            break;
    }
    abar->ghc |= BIT31;
    gBS->Stall(500);
}*/

EFI_STATUS
AhciIssueCmd(
    HBA_PORT *port,
    UINT32 cmdslot
    )
{   
    UINTN i;
    EFI_STATUS Status = EFI_SUCCESS;

    //make sure bit0 Start set to enable process cmd list
    if(!(port->cmd & 1))
        port->cmd |= BIT0|BIT4;     
    //clear status
    port->serr = port->serr;
    port->is = port->is;

    //if err status or busy, reset PxCMD.
    if ((port->is &(BIT30|BIT29|BIT28|BIT27|BIT26|BIT24|BIT23)) || \
         (port->tfd &(BIT0|BIT7))){
        port->cmd &= ~BIT0;
        //Start|Command List Override|FIS Receive Enable
        port->cmd |= BIT0|BIT3|BIT4;
        port->is = port->is;
    }

    //if (port->tfd & BIT0)
    //    return EFI_DEVICE_ERROR;
    //issue cmd
    port->ci = cmdslot;

    //wait cmd
    for(i=0; i<100; i++){
        gBS->Stall(500);
        if (!(port->tfd & BIT7))
            break;
    }

    if (port->tfd & BIT0)
        Status = EFI_DEVICE_ERROR;

    return Status;
}

VOID
BuildH2DFis(
    FIS_REG_H2D *fish2d, 
    UINT8 cmd
    )
{
    //initial H2D FIS (Serial ATA Revision 2.6 specification) 
//  +-+-------------+--------------+----------------+---------------+
//  |0| Features    |   Command    |0|C|R|R|R|PmPort| FIS Type (27h)|
//  +-+-------------+--------------+----------------+---------------+
//  |1|   Device    |   LBA High   |    LBA Mid     |  LBA Low      |
//  +-+-------------+--------------+----------------+---------------+
//  |2|Features(exp)|LBA High (exp)| LBA Mid (exp)  | LBA Low (exp) |
//  +-+-------------+--------------+----------------+---------------+
//  |3|   Control   | Reserved (0) |SectorCount(exp)|  Sector Count |
//  +-+-------------+--------------+----------------+---------------+
//  |4| Reserved (0)| Reserved (0) |  Reserved (0)  |  Reserved (0) |
//  +-+-------------+--------------+----------------+---------------+
    fish2d->fis_type = FIS_TYPE_REG_H2D;
    fish2d->pmport = 0;
    fish2d->c = 1;
    fish2d->command = cmd;
    fish2d->featurel = 1;

}

EFI_STATUS
ATAPIEjectODD(
   HBA_MEM  *abar,
   FIS_REG_H2D *fish2d,
   HBA_CMD_HEADER *cmdlist,
   HBA_CMD_TBL *cmdheader
    )
{
    EFI_STATUS  Status = EFI_NOT_FOUND;
    UINT32      ip;   //implemented port
    UINTN       i, PortAddr;
    HBA_PORT    *port;
    DATA_64     Data64;

    //scan implemented port
    for (ip = abar->pi, i=0; ip>0;ip>>=1, i++){
        if (ip&1){
            PortAddr = (UINTN)abar + 0x100 + 0x80*i;
            port = (HBA_PORT *)(UINTN) PortAddr;
            //found ODD and issue eject command
            if (port->sig == ATAPIDRIVE){
                //setting ATAPI cmd enable FIS
                BuildH2DFis(fish2d, 0xA0);
                //setting command header
                CopyMem(cmdheader->cfis, fish2d, sizeof(FIS_REG_H2D));

                cmdheader->prdt_entry[0].i = 1;

                cmdheader->acmd[0] = 0x1B;  //START STOP UNIT Command
                cmdheader->acmd[4] = 0x02;  //LOEJ bit

                cmdlist->cfl = 5;
                cmdlist->prdtl = 1;
                cmdlist->a = 1; //enable ATAPI cmd
                cmdlist->ctba = (UINT32)(UINTN) cmdheader;

                Data64.Uint64 = (UINTN)cmdlist;
                port->clb = Data64.Uint32.Lower32;
                port->clbu = Data64.Uint32.Upper32;

                Status = AhciIssueCmd(port, BIT0);
                break;
            }
        }
    }

    return Status;
}
