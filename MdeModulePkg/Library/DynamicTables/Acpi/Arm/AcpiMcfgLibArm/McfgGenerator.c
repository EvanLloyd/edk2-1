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
#include <Library/MemoryAllocationLib.h>
#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/Smbios.h>
#include <IndustryStandard/MemoryMappedConfigurationSpaceAccessTable.h>

#include <DynamicTables/TableGenerator.h>
#include <DynamicTables/AcpiTableGenerator.h>
#include <DynamicTables/ConfigurationManagerObject.h>
#include <DynamicTables/ConfigurationManagerHelper.h>
#include <DynamicTables/StandardNameSpaceObjects.h>
#include <DynamicTables/ArmNameSpaceObjects.h>
#include <Protocol/ConfigurationManagerProtocol.h>
#include <Library/TableHelperLib.h>

/** ARM standard MCFG Generator

    Generates the MCFG Table as specified by the PCI Firmware
    Specification - Revision 3.2, January 26, 2015.

Requirements:
  The following Configuration Manager Object(s) are required by
  this Generator:
  - EArmObjPciConfigSpaceInfo
*/

#pragma pack(1)

/** This typedef is used to shorten the name of the MCFG Table
    header structure.
*/
typedef
  EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_BASE_ADDRESS_TABLE_HEADER
  MCFG_TABLE;

/** This typedef is used to shorten the name of the Enhanced
    Configuration Space address structure.
*/
typedef
  EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE
  MCFG_CFG_SPACE_ADDR;

#pragma pack()

/** Retrieve the PCI Configuration Space Information.
*/
GET_OBJECT_LIST (
  EObjNameSpaceArm,
  EArmObjPciConfigSpaceInfo,
  CM_ARM_PCI_CONFIG_SPACE_INFO
  );

/** Add the PCI Enhanced Configuration Space Information to the MCFG Table.

    @param [in]  Mcfg                Pointer to MCFG Table.
    @param [in]  PciCfgSpaceOffset   Offset for the PCI Configuration Space
                                     Info structure in the MCFG Table.
    @param [in]  PciCfgSpaceInfoList Pointer to the PCI Configuration Space
                                     Info List.
    @param [in]  PciCfgSpaceCount    Count of PCI Configuration Space Info.
*/
STATIC
VOID
AddPciConfigurationSpaceList (
  IN       MCFG_TABLE                   * CONST Mcfg,
  IN CONST UINTN                                PciCfgSpaceOffset,
  IN CONST CM_ARM_PCI_CONFIG_SPACE_INFO * CONST PciCfgSpaceInfoList,
  IN CONST UINTN                                PciCfgSpaceCount
)
{
  UINTN                  idx;
  MCFG_CFG_SPACE_ADDR  * PciCfgSpace;

  ASSERT (Mcfg != NULL);
  ASSERT (PciCfgSpaceInfoList != NULL);

  PciCfgSpace = (MCFG_CFG_SPACE_ADDR *)((UINT8*)Mcfg + PciCfgSpaceOffset);
  for (idx = 0; idx < PciCfgSpaceCount; idx++) {
    // Add PCI Configuration Space entry
    PciCfgSpace[idx].BaseAddress = PciCfgSpaceInfoList[idx].BaseAddress;
    PciCfgSpace[idx].PciSegmentGroupNumber =
      PciCfgSpaceInfoList[idx].PciSegmentGroupNumber;
    PciCfgSpace[idx].StartBusNumber = PciCfgSpaceInfoList[idx].StartBusNumber;
    PciCfgSpace[idx].EndBusNumber = PciCfgSpaceInfoList[idx].EndBusNumber;
    PciCfgSpace[idx].Reserved = EFI_ACPI_RESERVED_DWORD;
  }
}

/** Construct the MCFG ACPI table.

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
    @retval EFI_NOT_FOUND         The required object was not found.
    @retval EFI_BAD_BUFFER_SIZE   The size returned by the Configuration Manager
                                  is less than the Object size for the requested
                                  object.
*/
STATIC
EFI_STATUS
EFIAPI
BuildMcfgTable (
  IN  CONST ACPI_TABLE_GENERATOR                * CONST This,
  IN  CONST CM_STD_OBJ_ACPI_TABLE_INFO          * CONST AcpiTableInfo,
  IN  CONST EFI_CONFIGURATION_MANAGER_PROTOCOL  * CONST CfgMgrProtocol,
  OUT       EFI_ACPI_DESCRIPTION_HEADER        ** CONST Table
  )
{
  EFI_STATUS                      Status;
  UINTN                           TableSize;
  UINTN                           ConfigurationSpaceCount;
  CM_ARM_PCI_CONFIG_SPACE_INFO  * PciConfigSpaceInfoList = NULL;
  MCFG_TABLE                    * Mcfg;

  ASSERT (This != NULL);
  ASSERT (AcpiTableInfo != NULL);
  ASSERT (CfgMgrProtocol != NULL);
  ASSERT (Table != NULL);
  ASSERT (AcpiTableInfo->TableGeneratorId == This->GeneratorID);
  ASSERT (AcpiTableInfo->AcpiTableSignature == This->AcpiTableSignature);

  *Table = NULL;
  Status = GetEArmObjPciConfigSpaceInfo (
              CfgMgrProtocol,
              &PciConfigSpaceInfoList,
              &ConfigurationSpaceCount
              );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR,
      "ERROR: MCFG: Failed to get PCI Configuration Space Information." \
      " Status = %r\n",
      Status
      ));
    goto error_handler;
  }
  ASSERT (ConfigurationSpaceCount != 0);

  DEBUG ((
    DEBUG_INFO,
    "MCFG: Configuration Space Count = %d\n",
    ConfigurationSpaceCount
    ));

  // Calculate the MCFG Table Size
  TableSize = sizeof (MCFG_TABLE) +
    ((sizeof (MCFG_CFG_SPACE_ADDR) * ConfigurationSpaceCount));

  *Table = (EFI_ACPI_DESCRIPTION_HEADER*)AllocateZeroPool (TableSize);
  if (*Table == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: MCFG: Failed to allocate memory for MCFG Table, Size = %d," \
      " Status = %r\n",
      TableSize,
      Status
      ));
    goto error_handler;
  }

  Mcfg = (MCFG_TABLE*)*Table;
  DEBUG ((
    DEBUG_INFO,
    "MCFG: Mcfg = 0x%p TableSize = 0x%x\n",
    Mcfg,
    TableSize
    ));

  Status = AddAcpiHeader (CfgMgrProtocol, This, &Mcfg->Header, TableSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: MCFG: Failed to add ACPI header. Status = %r\n",
      Status
      ));
    goto error_handler;
  }

  Mcfg->Reserved = EFI_ACPI_RESERVED_QWORD;

  AddPciConfigurationSpaceList (
    Mcfg,
    sizeof (MCFG_TABLE),
    PciConfigSpaceInfoList,
    ConfigurationSpaceCount
    );

  return EFI_SUCCESS;

error_handler:
  if (*Table != NULL) {
    FreePool (*Table);
    *Table = NULL;
  }
  return Status;
}

/** Free any resources allocated for constructing the MCFG

    @param [in]  This           Pointer to the table generator.
    @param [in]  AcpiTableInfo  Pointer to the ACPI Table Info.
    @param [in]  CfgMgrProtocol Pointer to the Configuration Manager
                                Protocol Interface.
    @param [in]  Table          Pointer to the ACPI Table.

    @retval EFI_SUCCESS           The resources were freed successfully.
    @retval EFI_INVALID_PARAMETER The table pointer is NULL or invalid.
*/
STATIC
EFI_STATUS
FreeMcfgTableResources (
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
  ASSERT (AcpiTableInfo->AcpiTableSignature == This->AcpiTableSignature);

  if ((Table == NULL) || (*Table == NULL)) {
    DEBUG ((DEBUG_ERROR, "ERROR: MCFG: Invalid Table Pointer\n"));
    ASSERT ((Table != NULL) && (*Table != NULL));
    return EFI_INVALID_PARAMETER;
  }

  FreePool (*Table);
  *Table = NULL;
  return EFI_SUCCESS;
}

/** This macro defines the MCFG Table Generator revision.
*/
#define MCFG_GENERATOR_REVISION CREATE_REVISION (1, 0)

/** The interface for the MCFG Table Generator.
*/
STATIC
CONST
ACPI_TABLE_GENERATOR McfgGenerator = {
  // Generator ID
  CREATE_STD_ACPI_TABLE_GEN_ID (ESTD_ACPI_TABLE_ID_MCFG),
  // Generator Description
  L"ACPI.STD.MCFG.GENERATOR",
  // ACPI Table Signature
  EFI_ACPI_6_1_PCI_EXPRESS_MEMORY_MAPPED_CONFIGURATION_SPACE_BASE_ADDRESS_DESCRIPTION_TABLE_SIGNATURE,
  // ACPI Table Revision
  EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_SPACE_ACCESS_TABLE_REVISION,
  // Creator ID
  TABLE_GENERATOR_CREATOR_ID_ARM,
  // Creator Revision
  MCFG_GENERATOR_REVISION,
  // Build Table function
  BuildMcfgTable,
  // Free Resource function
  FreeMcfgTableResources
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
AcpiMcfgLibConstructor (
  IN CONST EFI_HANDLE                ImageHandle,
  IN       EFI_SYSTEM_TABLE  * CONST SystemTable
  )
{
  EFI_STATUS  Status;

  Status = RegisterAcpiTableGenerator (&McfgGenerator);
  DEBUG ((DEBUG_INFO, "MCFG: Register Generator. Status = %r\n", Status));
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
AcpiMcfgLibDestructor (
  IN CONST EFI_HANDLE                ImageHandle,
  IN       EFI_SYSTEM_TABLE  * CONST SystemTable
  )
{
  EFI_STATUS  Status;

  Status = UnRegisterAcpiTableGenerator (&McfgGenerator);
  DEBUG ((DEBUG_INFO, "MCFG: Unregister Generator. Status = %r\n", Status));
  ASSERT_EFI_ERROR (Status);
  return Status;
}
