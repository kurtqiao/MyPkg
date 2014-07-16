

[Defines]
  PLATFORM_NAME                  = MyPkg
  PLATFORM_GUID                  = 549867eb-059f-44d1-b1e1-13ec2f96c7a9
  PLATFORM_VERSION               = 0.01
  DSC_SPECIFICATION              = 0x00010006
  OUTPUT_DIRECTORY               = Build/MyPkg
  SUPPORTED_ARCHITECTURES        = IA32|IPF|X64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT

[PcdsFeatureFlag]

[PcdsFixedAtBuild]

[PcdsFixedAtBuild.IPF]

[LibraryClasses]
  #
  # Entry Point Libraries
  #
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  ShellCEntryLib|ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
  #kurt2013.03.05 add for UEFI driver, err:"Instance of library class [UefiDriverEntryPoint] is not found"
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  #
  # Common Libraries
  #
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  PerformanceLib|MdeModulePkg/Library/DxePerformanceLib/DxePerformanceLib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf
  FileHandleLib|ShellPkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
  SortLib|ShellPkg/Library/UefiSortLib/UefiSortLib.inf
#kurt add+++
	PciLib|MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
	PciCf8Lib|MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
#
  # MyPkgLib
  #
  AppFrameWorkLib|MyPkg/Library/AppFrameWorkLib/AppFrameWorkLib.inf
  AppCommonLib|MyPkg/Library/AppCommonLib/AppCommonLib.inf
#kurt add+++
[LibraryClasses.IA32]
  TimerLib|PerformancePkg/Library/TscTimerLib/DxeTscTimerLib.inf
  # To run in an emulation environment, such as Nt32Pkg, comment out the TimerLib
  # description above and un-comment the line below.
#  TimerLib|MdePkg/Library/BaseTimerLibNullTemplate/BaseTimerLibNullTemplate.inf

[LibraryClasses.X64]
  TimerLib|PerformancePkg/Library/TscTimerLib/DxeTscTimerLib.inf

[LibraryClasses.IPF]
  PalLib|MdePkg/Library/UefiPalLib/UefiPalLib.inf
  TimerLib|MdePkg/Library/SecPeiDxeTimerLibCpu/SecPeiDxeTimerLibCpu.inf

###################################################################################################
#
# Components Section - list of the modules and components that will be processed by compilation
#                      tools and the EDK II tools to generate PE32/PE32+/Coff image files.
#
# Note: The EDK II DSC file is not used to specify how compiled binary images get placed
#       into firmware volume images. This section is just a list of modules to compile from
#       source into UEFI-compliant binaries.
#       It is the FDF file that contains information on combining binary files into firmware
#       volume images, whose concept is beyond UEFI and is described in PI specification.
#       Binary modules do not need to be listed in this section, as they should be
#       specified in the FDF file. For example: Shell binary (Shell_Full.efi), FAT binary (Fat.efi),
#       Logo (Logo.bmp), and etc.
#       There may also be modules listed in this section that are not required in the FDF file,
#       When a module listed here is excluded from FDF file, then UEFI-compliant binary will be
#       generated for it, but the binary will not be put into any firmware volume.
#
###################################################################################################

[Components]
#### Sample Applications.
  MyPkg/Application/HddId/HddId.inf
  MyPkg/Application/AHCI/AHCI.inf
################################################################
#
# See the additional comments below if you plan to run applications under the
# Nt32 emulation environment.
#
[BuildOptions]
  INTEL:*_*_*_CC_FLAGS      = /Qfreestanding
   MSFT:*_*_*_CC_FLAGS      = /X /Zc:wchar_t
    GCC:*_*_*_CC_FLAGS      = -ffreestanding -nostdinc -nostdlib

##############################################################################
#
#  Include Boilerplate text required for building with the Standard Libraries.
#
##############################################################################
!include StdLib/StdLib.inc