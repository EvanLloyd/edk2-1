## @file
#  Dsc include file for Dynamic Tables Framework.
#
#  Copyright (c) 2017, ARM Limited. All rights reserved.<BR>
#
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
##

[Defines]

[BuildOptions]
  *_*_*_ASL_FLAGS          = -th -li

[LibraryClasses.common]

[Components.common]
  #
  # Dynamic Table Factory Dxe
  #
  MdeModulePkg/Universal/DynamicTables/DynamicTableFactoryDxe/DynamicTableFactoryDxe.inf {
    <LibraryClasses>
      TableHelperLib|MdeModulePkg/Library/DynamicTables/Common/TableHelperLib/TableHelperLib.inf
      NULL|MdeModulePkg/Library/DynamicTables/Acpi/Arm/AcpiRawLibArm/AcpiRawLibArm.inf
      NULL|MdeModulePkg/Library/DynamicTables/Acpi/Arm/AcpiFadtLibArm/AcpiFadtLibArm.inf
      NULL|MdeModulePkg/Library/DynamicTables/Acpi/Arm/AcpiMadtLibArm/AcpiMadtLibArm.inf
      NULL|MdeModulePkg/Library/DynamicTables/Acpi/Arm/AcpiGtdtLibArm/AcpiGtdtLibArm.inf
      NULL|MdeModulePkg/Library/DynamicTables/Acpi/Arm/AcpiSpcrLibArm/AcpiSpcrLibArm.inf
      NULL|MdeModulePkg/Library/DynamicTables/Acpi/Arm/AcpiDbg2LibArm/AcpiDbg2LibArm.inf
      NULL|MdeModulePkg/Library/DynamicTables/Acpi/Arm/AcpiMcfgLibArm/AcpiMcfgLibArm.inf
  }

  #
  # Dynamic Tables Manager Dxe
  #
  MdeModulePkg/Universal/DynamicTables/DynamicTableManagerDxe/DynamicTableManagerDxe.inf {
    <LibraryClasses>
      TableHelperLib|MdeModulePkg/Library/DynamicTables/Common/TableHelperLib/TableHelperLib.inf
  }
