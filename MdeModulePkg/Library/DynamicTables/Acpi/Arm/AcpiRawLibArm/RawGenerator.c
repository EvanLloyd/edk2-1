/** @file
  MCFG Table Generator

  Copyright (c) 2017, ARM Limited. All rights reserved.
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include <Protocol/AcpiTable.h>
#include <Library/DebugLib.h>
#include <Library/AcpiLib.h>
#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/Smbios.h>

#include <DynamicTables/TableGenerator.h>
#include <DynamicTables/AcpiTableGenerator.h>
#include <DynamicTables/ConfigurationManagerObject.h>
#include <DynamicTables/StandardNameSpaceObjects.h>
#include <DynamicTables/ArmNameSpaceObjects.h>
#include <Protocol/ConfigurationManagerProtocol.h>
#include <Library/TableHelperLib.h>

/** Construct the ACPI table using the ACPI table data provided.

    This function invokes the Confguration Manager protocol interface
    to get the required hardware information for generating the ACPI
    table.

    If this function allocates any resources then they must be freed
    in the FreeXXXXTableResources function.

    @param [in]  This           Pointer to the table generator.
    @param [in]  AcpiTableInfo  Pointer to the ACPI Table Info.
    @param [in]  CfgMgrProtocol Pointer to the Configuration Manager
                                Protocol Interface.
    @param [out] Table          Pointer to the constructed ACPI Table.

    @retval EFI_SUCCESS           Table generated successfully.
    @retval EFI_INVALID_PARAMETER A parameter is invalid.
*/
STATIC
EFI_STATUS
EFIAPI
BuildRawTable (
  IN  CONST ACPI_TABLE_GENERATOR                * CONST This,
  IN  CONST CM_STD_OBJ_ACPI_TABLE_INFO          * CONST AcpiTableInfo,
  IN  CONST EFI_CONFIGURATION_MANAGER_PROTOCOL  * CONST CfgMgrProtocol,
  OUT       EFI_ACPI_DESCRIPTION_HEADER        ** CONST Table
  )
{
  ASSERT (This != NULL);
  ASSERT (AcpiTableInfo != NULL);
  ASSERT (CfgMgrProtocol != NULL);
  ASSERT (Table != NULL);
  ASSERT (AcpiTableInfo->TableGeneratorId == This->GeneratorID);
  ASSERT (AcpiTableInfo->AcpiTableData != NULL);

  if (AcpiTableInfo->AcpiTableData == NULL) {
    *Table = NULL;
    return EFI_INVALID_PARAMETER;
  }

  *Table = AcpiTableInfo->AcpiTableData;

  return EFI_SUCCESS;
}

/** Free any resources allocated for constructing the ACPI table.

    @param [in]  This           Pointer to the table generator.
    @param [in]  AcpiTableInfo  Pointer to the ACPI Table Info.
    @param [in]  CfgMgrProtocol Pointer to the Configuration Manager
                                Protocol Interface.
    @param [in]  Table          Pointer to the ACPI Table.

    @retval EFI_SUCCESS         The resources were freed successfully.

*/
STATIC
EFI_STATUS
FreeRawTableResources (
  IN  CONST ACPI_TABLE_GENERATOR                * CONST This,
  IN  CONST CM_STD_OBJ_ACPI_TABLE_INFO          * CONST AcpiTableInfo,
  IN  CONST EFI_CONFIGURATION_MANAGER_PROTOCOL  * CONST CfgMgrProtocol,
  IN        EFI_ACPI_DESCRIPTION_HEADER        ** CONST Table
  )
{
  ASSERT (This != NULL);
  ASSERT (AcpiTableInfo != NULL);
  ASSERT (CfgMgrProtocol != NULL);
  ASSERT (AcpiTableInfo->TableGeneratorId == This->GeneratorID);

  return EFI_SUCCESS;
}

/** This macro defines the Raw Generator revision.
*/
#define RAW_GENERATOR_REVISION CREATE_REVISION (1, 0)

/** The interface for the Raw Table Generator.
*/
STATIC
CONST
ACPI_TABLE_GENERATOR RawGenerator = {
  // Generator ID
  CREATE_STD_ACPI_TABLE_GEN_ID (ESTD_ACPI_TABLE_ID_RAW),
  // Generator Description
  L"ACPI.STD.RAW.GENERATOR",
  // ACPI Table Signature - Unused
  0,
  // ACPI Table Revision - Unused
  0,
  // Creator ID
  TABLE_GENERATOR_CREATOR_ID_ARM,
  // Creator Revision
  RAW_GENERATOR_REVISION,
  // Build Table function
  BuildRawTable,
  // Free Resource function
  FreeRawTableResources
};

/** Register the Generator with the ACPI Table Factory.

    @param [in]  ImageHandle  The handle to the image.
    @param [in]  SystemTable  Pointer to the System Table.

    @retval EFI_SUCCESS           The Generator is registered.
    @retval EFI_INVALID_PARAMETER A parameter is invalid.
    @retval EFI_ALREADY_STARTED   The Generator for the Table ID
                                  is already registered.
*/
EFI_STATUS
EFIAPI
AcpiRawLibConstructor (
  IN CONST EFI_HANDLE                ImageHandle,
  IN       EFI_SYSTEM_TABLE  * CONST SystemTable
  )
{
  EFI_STATUS  Status;

  Status = RegisterAcpiTableGenerator (&RawGenerator);
  DEBUG ((DEBUG_INFO, "RAW: Register Generator. Status = %r\n", Status));
  ASSERT_EFI_ERROR (Status);
  return Status;
}

/** Un-register the Generator from the ACPI Table Factory.

    @param [in]  ImageHandle  The handle to the image.
    @param [in]  SystemTable  Pointer to the System Table.


    @retval EFI_SUCCESS           The Generator is un-registered.
    @retval EFI_INVALID_PARAMETER A parameter is invalid.
    @retval EFI_NOT_FOUND         The Generator is not registered.
*/

EFI_STATUS
EFIAPI
AcpiRawLibDestructor (
  IN CONST EFI_HANDLE                ImageHandle,
  IN       EFI_SYSTEM_TABLE  * CONST SystemTable
  )
{
  EFI_STATUS  Status;

  Status = UnRegisterAcpiTableGenerator (&RawGenerator);
  DEBUG ((DEBUG_INFO, "RAW: UnRegister Generator. Status = %r\n", Status));
  ASSERT_EFI_ERROR (Status);
  return Status;
}
