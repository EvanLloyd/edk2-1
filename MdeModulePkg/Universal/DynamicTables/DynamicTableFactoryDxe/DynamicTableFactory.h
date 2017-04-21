/** @file

  Copyright (c) 2017, ARM Limited. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

  @par Glossary:
    - Std - Standard
    - ACPI   - Advanced Configuration and Power Interface
    - SMBIOS - System Management BIOS
**/

#ifndef DYNAMIC_TABLE_FACTORY_H_
#define DYNAMIC_TABLE_FACTORY_H_

#pragma pack(1)

/** A structure that holds the list of registered ACPI and
    SMBIOS table generators.
*/
typedef struct DynamicTableFactoryInfo {
  /// An array for holding the list of Standard ACPI Table  Generators.
  CONST ACPI_TABLE_GENERATOR *
          StdAcpiTableGeneratorList[ESTD_ACPI_TABLE_ID_MAX];

  /// An array for holding the list of Custom ACPI Table Generators.
  CONST ACPI_TABLE_GENERATOR *
          CustomAcpiTableGeneratorList[FixedPcdGet16 (
            PcdMaxCustomACPIGenerators)];

  /// An array for holding the list of Standard SMBIOS Table Generators.
  CONST SMBIOS_TABLE_GENERATOR *
          StdSmbiosTableGeneratorList[ESTD_SMBIOS_TABLE_ID_MAX];

  /// An array for holding the list of Custom SMBIOS Table Generators.
  CONST SMBIOS_TABLE_GENERATOR *
          CustomSmbiosTableGeneratorList[FixedPcdGet16 (
            PcdMaxCustomSMBIOSGenerators)];
} EFI_DYNAMIC_TABLE_FACTORY_INFO;

/** Return a pointer to the ACPI table generator.

    @param [in]  This         Poiner to the Dynamic Table Factory Protocol.
    @param [in]  GeneratorId  The ACPI table generator ID for the
                              requested generator.
    @param [out] Generator    Pointer to the requested ACPI table
                              generator.

    @retval EFI_SUCCESS           Success.
    @retval EFI_INVALID_PARAMETER A parameter is invalid.
    @retval EFI_NOT_FOUND         The requested generator is not found
                                  in the list of registered generators.
*/
EFI_STATUS
EFIAPI
GetAcpiTableGenerator (
  IN  CONST EFI_DYNAMIC_TABLE_FACTORY_PROTOCOL  * CONST This,
  IN  CONST ACPI_TABLE_GENERATOR_ID                     GeneratorId,
  OUT CONST ACPI_TABLE_GENERATOR               ** CONST Generator
  );

/** Return a pointer to the SMBIOS table generator.

    @param [in]  This         Poiner to the Dynamic Table Factory Protocol.
    @param [in]  GeneratorId  The SMBIOS table generator ID for the
                              requested generator.
    @param [out] Generator    Pointer to the requested SMBIOS table
                              generator.

    @retval EFI_SUCCESS           Success.
    @retval EFI_INVALID_PARAMETER A parameter is invalid.
    @retval EFI_NOT_FOUND         The requested generator is not found
                                  in the list of registered generators.
*/
EFI_STATUS
EFIAPI
GetSmbiosTableGenerator (
  IN  CONST EFI_DYNAMIC_TABLE_FACTORY_PROTOCOL  * CONST This,
  IN  CONST SMBIOS_TABLE_GENERATOR_ID                   GeneratorId,
  OUT CONST SMBIOS_TABLE_GENERATOR             ** CONST Generator
  );

#pragma pack()

#endif // DYNAMIC_TABLE_FACTORY_H_
