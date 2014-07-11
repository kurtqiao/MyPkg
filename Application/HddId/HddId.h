
#pragma once

#define APP_VERSION L"v0.1"

#define MAX_SUPPORT_CONTROLLER    2
#define MAX_SUPPORT_DRIVES        6

#define MAX_SUPPORT_PORTS         10    //support 10 ports implemented


/*base addres offsets*/
#define PRIMARY_CMD_BASEADDR      0x10
#define PRIMARY_CTRL_BASEADDR     0x14
#define SECONDARY_CMD_BASEADDR      0x18
#define SECONDARY_CTRL_BASEADDR     0x1C
/*Legacy mode base addresses*/
#define LEGACY_PRIMARY_CMD_BASEADDR     0x1F0
#define LEGACY_PRIMARY_CTRL_BASEADDR    0x3F4 //0x3F6 - 2 for compatible with native mode base address
#define LEGACY_SECONDARY_CMD_BASEADDR   0x170
#define LEGACY_SECONDARY_CTRL_BASEADDR  0x374 //0x376 - 2

/* port offsets */
#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT   0x02
#define ATA_REG_LBA_LOW    0x03
#define ATA_REG_LBA_MED    0x04
#define ATA_REG_LBA_HIGH   0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_LBA3       0x03
#define ATA_REG_LBA4       0x04
#define ATA_REG_LBA5       0x05

#define ATA_REG_CONTROL    0x02
#define ATA_REG_ALTSTATUS  0x02
#define ATA_REG_DEVADDRESS 0x0D

#define ATA_REG_BMICOM     0x00
#define ATA_REG_BMISTA     0x02
#define ATA_REG_BMIDTP     0x04

/* status */
#define ATA_SR_BSY     0x80
#define ATA_SR_DRDY    0x40
#define ATA_SR_DF      0x20
#define ATA_SR_DSC     0x10
#define ATA_SR_DRQ     0x08
#define ATA_SR_CORR    0x04
#define ATA_SR_IDX     0x02
#define ATA_SR_ERR     0x01

/* features/errors port */
#define ATA_ER_BBK      0x80
#define ATA_ER_UNC      0x40
#define ATA_ER_MC       0x20
#define ATA_ER_IDNF     0x10
#define ATA_ER_MCR      0x08
#define ATA_ER_ABRT     0x04
#define ATA_ER_TK0NF    0x02
#define ATA_ER_AMNF     0x01

/* command port */
#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC
#define ATAPI_CMD_READ            0xA8
#define ATAPI_CMD_EJECT           0x1B

/* ident space */
#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

/* control port */
#define ATA_CTRL_NIEN  0x02
#define ATA_CTRL_SRST  0x04
#define ATA_CTRL_HOB   0x08

/* interfaces */
#define IDE_ATA        0x00
#define IDE_ATAPI      0x01

/* disks */
#define ATA_MASTER     0xA0
#define ATA_SLAVE      0xB0

/* channels */
#define ATA_PRIMARY    0x00
#define ATA_SECONDARY  0x01

/* direcctions */
#define ATA_READ       0x00
#define ATA_WRITE      0x01


typedef enum {
  IDE_CONTROLLER,
  SATA_AHCI_1,
  UNKNOWN_TYPE=99
} EFI_MASS_STORAGE_TYPE;

typedef struct
{
  UINT32  BasePort;
  UINT32  ContrlPort;
  UINT16  SelDrive; //Master or Slave, value should be A0B0, A000, 00B0
  BOOLEAN Present;
  UINT8   Channel;  //Primary or Secondary
}EFI_IDE_DEVICES;

typedef struct
{
  UINT64  ATA_CONTROLLER_ADDRESS[MAX_SUPPORT_CONTROLLER];    //MAX support 5 IDE/AHCI contoller
  EFI_MASS_STORAGE_TYPE   ATATypte;
  EFI_IDE_DEVICES     IDE_DEVICES[MAX_SUPPORT_DRIVES];
}EFI_ATA_DEVICES;


