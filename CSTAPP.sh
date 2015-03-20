#!/bin/sh
#
#  Construct UEFI Application Script
#

#check input
if [ -z "$1" ]; then
  echo please give a file name!
  return
fi
#init
mCurrentYear=$(date +%Y)
mGUID=$(uuidgen)
mDIR="./Application/$1/"
mFILE_INF=$1".inf"
mFILE_SRC=$1".c"

#create files
if [ ! -d $mDIR ]; then
mkdir $mDIR
fi
touch $mDIR$mFILE_INF $mDIR$mFILE_SRC

#write inf file
cat <<EOF >  $mDIR$mFILE_INF
## 
#  UEFI Application
#
#  A shell application
#
#  Free As Freedom @ $mCurrentYear
#
#
##

[Defines]
  INF_VERSION                    = 0x00010006
  BASE_NAME                      = ${1^^}
  FILE_GUID                      = ${mGUID^^} 
  MODULE_TYPE                    = UEFI_APPLICATION
  VERSION_STRING                 = 1.0
  ENTRY_POINT                    = ShellCEntryLib

[Sources]
  $mFILE_SRC

[Packages]
  MdePkg/MdePkg.dec
  MdeModulePkg/MdeModulePkg.dec
  ShellPkg/ShellPkg.dec
  MyPkg/MyPkg.dec

[LibraryClasses]
  UefiApplicationEntryPoint
  UefiLib
  ShellCEntryLib

[Protocols]

EOF

#write source c
cat <<EOF >   $mDIR$mFILE_SRC

/*++

FreeAsFreedom @ $mCurrentYear Kurt Qiao

Module Name:

    $mFILE_SRC 
        
    Abstract:

    Revision History

--*/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include "Library/UefiBootServicesTableLib.h"

EFI_STATUS
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
)
{
	      	
  EFI_STATUS  Status;
  Status = EFI_SUCCESS;

  return Status;
}
EOF
