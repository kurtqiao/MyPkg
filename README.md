MyPkg
============
A sample UEFI Application package

License:
------
 Free as in Freedom(GPLv3).

Build(windows):
------
 1. open VS cmd prompt
 2. edksetup.bat
 3. Build -a IA32/X64 -p MyPkg\MyPkg.dsc  
 4. output in Build\MyPkg  

> * check uefi.blogspot.com for how to setting VS UEFI build env  
> * refer to UEFI readme check how to build in linux

Application:
------
  1. AHCI
     sample how to manipulate AHCI mmio issue HDD identify cmd and ODD eject cmd.
  2. ctest
     a uefi sample import c language.
  3. HddId:
     sample how to identify HDD data,support both AHCI and IDE mode.
  4. HiiMenu
     sample how to use uefi HII.
  5. bootmgr
     sample to manipulate UEFI variables, show/set BootOrder, show Boot####, BootCurrent.