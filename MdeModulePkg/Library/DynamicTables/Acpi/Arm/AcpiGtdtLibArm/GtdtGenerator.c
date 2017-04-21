/** @file
  GTDT Table Generator

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

#include <DynamicTables/TableGenerator.h>
#include <DynamicTables/AcpiTableGenerator.h>
#include <DynamicTables/ConfigurationManagerObject.h>
#include <DynamicTables/ConfigurationManagerHelper.h>
#include <DynamicTables/StandardNameSpaceObjects.h>
#include <DynamicTables/ArmNameSpaceObjects.h>
#include <Protocol/ConfigurationManagerProtocol.h>
#include <Library/TableHelperLib.h>

/** ARM standard GTDT Generator

    Generates the ACPI GTDT Table as specified by the
    ACPI 6.1 Specification - January, 2016.

Requirements:
  The following Configuration Manager Object(s) are required by
  this Generator:
  - EArmObjGenericTimerInfo
  - EArmObjPlatformGenericWatchdogInfo (OPTIONAL)
  - EArmObjPlatformGTBlockInfo (OPTIONAL)
*/

/** This macro expands to a function that retrieves the Generic
    Timer Information from the Configuration Manager.
*/
GET_OBJECT_LIST (
  EObjNameSpaceArm,
  EArmObjGenericTimerInfo,
  CM_ARM_GENERIC_TIMER_INFO
  );

/** This macro expands to a function that retrieves the SBSA Generic
    Watchdog Timer Information from the Configuration Manager.
*/
GET_OBJECT_LIST (
  EObjNameSpaceArm,
  EArmObjPlatformGenericWatchdogInfo,
  CM_ARM_GENERIC_WATCHDOG_INFO
  );

/** This macro expands to a function that retrieves the Platform Generic
    Timer Block Information from the Configuration Manager.
*/
GET_OBJECT_LIST (
  EObjNameSpaceArm,
  EArmObjPlatformGTBlockInfo,
  CM_ARM_GTBLOCK_INFO
  );

/** Add the Generic Timer Information to the GTDT table.

    Also update the Platform Timer offset information if the platform
    implements platform timers.

    @param [in]  CfgMgrProtocol     Pointer to the Configuration Manager
                                    Protocol Interface.
    @param [in]  Gtdt               Pointer to the GTDT Table.
    @param [in]  PlatformTimerCount Platform timer count.

    @retval EFI_SUCCESS           Success.
    @retval EFI_INVALID_PARAMETER A parameter is invalid.
    @retval EFI_NOT_FOUND         The required object was not found.
    @retval EFI_BAD_BUFFER_SIZE   The size returned by the Configuration Manager
                                  is less than the Object size for the requested
                                  object.
*/
STATIC
EFI_STATUS
EFIAPI
AddGenericTimerInfo (
  IN  CONST EFI_CONFIGURATION_MANAGER_PROTOCOL * CONST CfgMgrProtocol,
  IN        EFI_ACPI_6_1_GENERIC_TIMER_DESCRIPTION_TABLE * CONST Gtdt,
  IN  CONST UINT32 PlatformTimerCount
)
{
  EFI_STATUS                   Status;
  CM_ARM_GENERIC_TIMER_INFO  * GenericTimerInfo = NULL;

  ASSERT (CfgMgrProtocol != NULL);
  ASSERT (Gtdt != NULL);

  Status = GetEArmObjGenericTimerInfo (
             CfgMgrProtocol,
             &GenericTimerInfo,
             NULL
             );

  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: GTDT: Failed to get GenericTimerInfo. Status = %r\n",
      Status
      ));
    return Status;
  }

  Gtdt->CntControlBasePhysicalAddress =
    GenericTimerInfo->CounterControlBaseAddress;
  Gtdt->Reserved = EFI_ACPI_RESERVED_DWORD;
  Gtdt->CntReadBasePhysicalAddress =
    GenericTimerInfo->CounterReadBaseAddress;
  Gtdt->PlatformTimerCount = PlatformTimerCount;
  Gtdt->PlatformTimerOffset = (PlatformTimerCount == 0) ? 0 :
    sizeof (EFI_ACPI_6_1_GENERIC_TIMER_DESCRIPTION_TABLE);

  Gtdt->SecurePL1TimerGSIV = GenericTimerInfo->SecurePL1TimerGSIV;
  Gtdt->SecurePL1TimerFlags = GenericTimerInfo->SecurePL1TimerFlags;
  Gtdt->NonSecurePL1TimerGSIV = GenericTimerInfo->NonSecurePL1TimerGSIV;
  Gtdt->NonSecurePL1TimerFlags = GenericTimerInfo->NonSecurePL1TimerFlags;
  Gtdt->VirtualTimerGSIV = GenericTimerInfo->VirtualTimerGSIV;
  Gtdt->VirtualTimerFlags = GenericTimerInfo->VirtualTimerFlags;
  Gtdt->NonSecurePL2TimerGSIV = GenericTimerInfo->NonSecurePL2TimerGSIV;
  Gtdt->NonSecurePL2TimerFlags = GenericTimerInfo->NonSecurePL2TimerFlags;

  return EFI_SUCCESS;
}

/** Add the SBSA Generic Watchdog Timers to the GTDT table.

    @param [in]  Gtdt             Pointer to the GTDT Table.
    @param [in]  WatchdogOffset   Offset to the watchdog information in the
                                  GTDT Table.
    @param [in]  WatchdogInfoList Pointer to the watchdog information list.
    @param [in]  WatchdogCount    Platform timer count.
*/
STATIC
VOID
AddGenericWatchdogList (
  IN EFI_ACPI_6_1_GENERIC_TIMER_DESCRIPTION_TABLE  * CONST Gtdt,
  IN CONST UINTN                                           WatchdogOffset,
  IN CONST CM_ARM_GENERIC_WATCHDOG_INFO            * CONST WatchdogInfoList,
  IN CONST UINTN                                           WatchdogCount
  )
{
  UINTN                                                idx;
  EFI_ACPI_6_1_GTDT_SBSA_GENERIC_WATCHDOG_STRUCTURE  * Watchdog;

  ASSERT (Gtdt != NULL);
  ASSERT (WatchdogInfoList != NULL);

  Watchdog = (EFI_ACPI_6_1_GTDT_SBSA_GENERIC_WATCHDOG_STRUCTURE *)
             ((UINT8*)Gtdt + WatchdogOffset);

  for (idx = 0; idx < WatchdogCount; idx++) {
    // Add watchdog entry
    DEBUG ((DEBUG_INFO, "GTDT: Watchdog[%d] = 0x%p\n", idx, &Watchdog[idx]));
    Watchdog[idx].Type = EFI_ACPI_6_1_GTDT_SBSA_GENERIC_WATCHDOG;
    Watchdog[idx].Length =
      sizeof (EFI_ACPI_6_1_GTDT_SBSA_GENERIC_WATCHDOG_STRUCTURE);
    Watchdog[idx].Reserved = EFI_ACPI_RESERVED_BYTE;
    Watchdog[idx].RefreshFramePhysicalAddress =
      WatchdogInfoList[idx].RefreshFrameAddress;
    Watchdog[idx].WatchdogControlFramePhysicalAddress =
      WatchdogInfoList[idx].ControlFrameAddress;
    Watchdog[idx].WatchdogTimerGSIV = WatchdogInfoList[idx].TimerGSIV;
    Watchdog[idx].WatchdogTimerFlags = WatchdogInfoList[idx].Flags;

  } // for
}

/** Update the GT Block Timer Frame lists in the GTDT Table.

    @param [in]  GtBlockFrame           Pointer to the GT Block Frames
                                        list to be updated.
    @param [in]  GTBlockTimerFrameList  Pointer to the GT Block Frame
                                        Information List.
    @param [in]  GTBlockFrameCount      Number of GT Block Frames.

    @retval EFI_SUCCESS           Table generated successfully.
    @retval EFI_INVALID_PARAMETER A parameter is invalid.
*/
STATIC
EFI_STATUS
AddGTBlockTimerFrames (
  IN       EFI_ACPI_6_1_GTDT_GT_BLOCK_TIMER_STRUCTURE * CONST GtBlockFrame,
  IN CONST CM_ARM_GTBLOCK_TIMER_FRAME_INFO * CONST GTBlockTimerFrameList,
  IN CONST UINTN GTBlockFrameCount
)
{
  UINTN       idx;

  ASSERT (GtBlockFrame != NULL);
  ASSERT (GTBlockTimerFrameList != NULL);

  if (GTBlockFrameCount > 8) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: GTDT: GT Block Frame Count %d is greater than 8\n",
      GTBlockFrameCount
      ));
    ASSERT (GTBlockFrameCount <= 8);
    return EFI_INVALID_PARAMETER;
  }

  for (idx = 0; idx < GTBlockFrameCount; idx++) {
    DEBUG ((
      DEBUG_INFO,
      "GTDT: GtBlockFrame[%d] = 0x%p\n",
      idx,
      GtBlockFrame[idx]
      ));

    GtBlockFrame[idx].GTFrameNumber = GTBlockTimerFrameList[idx].FrameNumber;
    GtBlockFrame[idx].Reserved[0] = EFI_ACPI_RESERVED_BYTE;
    GtBlockFrame[idx].Reserved[1] = EFI_ACPI_RESERVED_BYTE;
    GtBlockFrame[idx].Reserved[2] = EFI_ACPI_RESERVED_BYTE;

    GtBlockFrame[idx].CntBaseX =
      GTBlockTimerFrameList[idx].PhysicalAddressCntBase;
    GtBlockFrame[idx].CntEL0BaseX =
      GTBlockTimerFrameList[idx].PhysicalAddressCntEL0Base;

    GtBlockFrame[idx].GTxPhysicalTimerGSIV =
      GTBlockTimerFrameList[idx].PhysicalTimerGSIV;
    GtBlockFrame[idx].GTxPhysicalTimerFlags =
      GTBlockTimerFrameList[idx].PhysicalTimerFlags;

    GtBlockFrame[idx].GTxVirtualTimerGSIV =
      GTBlockTimerFrameList[idx].VirtualTimerGSIV;
    GtBlockFrame[idx].GTxVirtualTimerFlags =
      GTBlockTimerFrameList[idx].VirtualTimerFlags;

    GtBlockFrame[idx].GTxCommonFlags = GTBlockTimerFrameList[idx].CommonFlags;

  } // for
  return EFI_SUCCESS;
}

/** Add the GT Block Timers in the GTDT Table.

    @param [in]  Gtdt             Pointer to the GTDT Table.
    @param [in]  GTBlockOffset    Offset of the GT Block
                                  information in the GTDT Table.
    @param [in]  GTBlockInfo      Pointer to the GT Block
                                  Information List.
    @param [in]  BlockTimerCount  Number of GT Block Timers.

    @retval EFI_SUCCESS           Table generated successfully.
    @retval EFI_INVALID_PARAMETER A parameter is invalid.

*/
STATIC
EFI_STATUS
AddGTBlockList (
  IN EFI_ACPI_6_1_GENERIC_TIMER_DESCRIPTION_TABLE  * CONST Gtdt,
  IN CONST UINTN                                           GTBlockOffset,
  IN CONST CM_ARM_GTBLOCK_INFO                     * CONST GTBlockInfo,
  IN CONST UINTN                                           BlockTimerCount
)
{
  EFI_STATUS                                    Status;
  UINTN                                         idx;
  EFI_ACPI_6_1_GTDT_GT_BLOCK_STRUCTURE        * GTBlock;
  EFI_ACPI_6_1_GTDT_GT_BLOCK_TIMER_STRUCTURE  * GtBlockFrame;

  ASSERT (Gtdt != NULL);
  ASSERT (GTBlockInfo != NULL);

  GTBlock = (EFI_ACPI_6_1_GTDT_GT_BLOCK_STRUCTURE *)((UINT8*)Gtdt +
              GTBlockOffset);

  for (idx = 0; idx < BlockTimerCount; idx++) {
    DEBUG ((DEBUG_INFO, "GTDT: GTBlock[%d] = 0x%p\n",
      idx,
      GTBlock
      ));

    GTBlock->Type = EFI_ACPI_6_1_GTDT_GT_BLOCK;
    GTBlock->Length = sizeof (EFI_ACPI_6_1_GTDT_GT_BLOCK_STRUCTURE) +
                        (sizeof (EFI_ACPI_6_1_GTDT_GT_BLOCK_TIMER_STRUCTURE) *
                          GTBlockInfo[idx].GTBlockTimerFrameCount);

    GTBlock->Reserved = EFI_ACPI_RESERVED_BYTE;
    GTBlock->CntCtlBase = GTBlockInfo[idx].GTBlockPhysicalAddress;
    GTBlock->GTBlockTimerCount = GTBlockInfo[idx].GTBlockTimerFrameCount;
    GTBlock->GTBlockTimerOffset =
      sizeof (EFI_ACPI_6_1_GTDT_GT_BLOCK_STRUCTURE);

    GtBlockFrame = (EFI_ACPI_6_1_GTDT_GT_BLOCK_TIMER_STRUCTURE*)
      ((UINT8*)GTBlock + GTBlock->GTBlockTimerOffset);

    // Add GT Block Timer frames
    Status = AddGTBlockTimerFrames (
               GtBlockFrame,
               GTBlockInfo[idx].GTBlockTimerFrameList,
               GTBlockInfo[idx].GTBlockTimerFrameCount
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: GTDT: Failed to add Generic Timer Frames. Status = %r\n",
        Status
        ));
      return Status;
    }

    // Next GTBlock
    GTBlock = (EFI_ACPI_6_1_GTDT_GT_BLOCK_STRUCTURE *)((UINT8*)GTBlock +
                GTBlock->Length);
  }// for
  return EFI_SUCCESS;
}

/** Construct the GTDT ACPI table.

    Called by the Dynamic Table Manager, this function invokes the
    Confguration Manager protocol interface to get the required hardware
    information for generating the ACPI table.

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
    @retval EFI_OUT_OF_RESOURCES  Memory allocation failed.
*/
STATIC
EFI_STATUS
EFIAPI
BuildGtdtTable (
  IN  CONST ACPI_TABLE_GENERATOR                * CONST This,
  IN  CONST CM_STD_OBJ_ACPI_TABLE_INFO          * CONST AcpiTableInfo,
  IN  CONST EFI_CONFIGURATION_MANAGER_PROTOCOL  * CONST CfgMgrProtocol,
  OUT       EFI_ACPI_DESCRIPTION_HEADER        ** CONST Table
  )
{
  EFI_STATUS                                      Status;
  UINTN                                           TableSize;
  UINTN                                           PlatformTimerCount = 0;
  UINTN                                           WatchdogCount;
  UINTN                                           BlockTimerCount;
  CM_ARM_GENERIC_WATCHDOG_INFO                  * WatchdogInfoList = NULL;
  CM_ARM_GTBLOCK_INFO                           * GTBlockInfo = NULL;
  EFI_ACPI_6_1_GENERIC_TIMER_DESCRIPTION_TABLE  * Gtdt;
  UINTN                                           idx;
  UINTN                                           GTBlockOffset;
  UINTN                                           WatchdogOffset = 0;

  ASSERT (This != NULL);
  ASSERT (AcpiTableInfo != NULL);
  ASSERT (CfgMgrProtocol != NULL);
  ASSERT (Table != NULL);
  ASSERT (AcpiTableInfo->TableGeneratorId == This->GeneratorID);
  ASSERT (AcpiTableInfo->AcpiTableSignature == This->AcpiTableSignature);

  *Table = NULL;
  Status = GetEArmObjPlatformGTBlockInfo (
             CfgMgrProtocol,
             &GTBlockInfo,
             &BlockTimerCount
             );
  if (EFI_ERROR (Status) && (Status != EFI_NOT_FOUND)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: GTDT: Failed to Get Platform GT Block Information." \
      " Status = %r\n",
      Status
      ));
    goto error_handler;
  }

  Status = GetEArmObjPlatformGenericWatchdogInfo (
             CfgMgrProtocol,
             &WatchdogInfoList,
             &WatchdogCount
             );
  if (EFI_ERROR (Status) && (Status != EFI_NOT_FOUND)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: GTDT: Failed to Get Platform Generic Watchdog Information." \
      " Status = %r\n",
      Status
      ));
    goto error_handler;
  }

  DEBUG ((
    DEBUG_INFO,
    "GTDT: BlockTimerCount = %d, WatchdogCount = %d\n",
    BlockTimerCount,
    WatchdogCount
    ));

  // Calculate the GTDT Table Size
  TableSize = sizeof (EFI_ACPI_6_1_GENERIC_TIMER_DESCRIPTION_TABLE);
  if (BlockTimerCount != 0) {
    GTBlockOffset = TableSize;
    PlatformTimerCount += BlockTimerCount;
    TableSize += (sizeof (EFI_ACPI_6_1_GTDT_GT_BLOCK_STRUCTURE) *
                  BlockTimerCount);

    for (idx = 0; idx < BlockTimerCount; idx++) {
      if (GTBlockInfo[idx].GTBlockTimerFrameCount > 8) {
        Status = EFI_INVALID_PARAMETER;
        DEBUG ((
          DEBUG_ERROR,
          "GTDT: GTBockFrameCount cannot be more than 8." \
          " GTBockFrameCount = %d, Status = %r\n",
          GTBlockInfo[idx].GTBlockTimerFrameCount,
          Status
          ));
        goto error_handler;
      }
      TableSize += (sizeof (EFI_ACPI_6_1_GTDT_GT_BLOCK_TIMER_STRUCTURE) *
        GTBlockInfo[idx].GTBlockTimerFrameCount);
    }

    DEBUG ((
      DEBUG_INFO,
      "GTDT: GTBockOffset = 0x%x, PLATFORM_TIMER_COUNT = %d\n",
      GTBlockOffset,
      PlatformTimerCount
      ));
  }

  if (WatchdogCount != 0) {
    WatchdogOffset = TableSize;
    PlatformTimerCount += WatchdogCount;
    TableSize += (sizeof (EFI_ACPI_6_1_GTDT_SBSA_GENERIC_WATCHDOG_STRUCTURE) *
                  WatchdogCount);
    DEBUG ((
      DEBUG_INFO,
      "GTDT: WatchdogOffset = 0x%x, PLATFORM_TIMER_COUNT = %d\n",
      WatchdogOffset,
      PlatformTimerCount
      ));
  }

  *Table = (EFI_ACPI_DESCRIPTION_HEADER*)AllocateZeroPool (TableSize);
  if (*Table == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: GTDT: Failed to allocate memory for GTDT Table, Size = %d," \
      " Status = %r\n",
      TableSize,
      Status
      ));
    goto error_handler;
  }

  Gtdt = (EFI_ACPI_6_1_GENERIC_TIMER_DESCRIPTION_TABLE*)*Table;
  DEBUG ((
    DEBUG_INFO,
    "GTDT: Gtdt = 0x%p TableSize = 0x%x\n",
    Gtdt,
    TableSize
    ));

  Status = AddAcpiHeader (CfgMgrProtocol, This, &Gtdt->Header, TableSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: GTDT: Failed to add ACPI header. Status = %r\n",
      Status
      ));
    goto error_handler;
  }

  Status = AddGenericTimerInfo (
             CfgMgrProtocol,
             Gtdt,
             PlatformTimerCount
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: GTDT: Failed to add Generic Timer Info. Status = %r\n",
      Status
      ));
    goto error_handler;
  }

  if (BlockTimerCount != 0) {
    Status = AddGTBlockList (
               Gtdt,
               GTBlockOffset,
               GTBlockInfo,
               BlockTimerCount
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: GTDT: Failed to add GT Block timers. Status = %r\n",
        Status
        ));
      goto error_handler;
    }
  }

  if (WatchdogCount != 0) {
    AddGenericWatchdogList (
      Gtdt,
      WatchdogOffset,
      WatchdogInfoList,
      WatchdogCount
      );
  }

  return Status;

error_handler:
  if (*Table != NULL) {
    FreePool (*Table);
    *Table = NULL;
  }
  return Status;
}

/** Free any resources allocated for constructing the GTDT.

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
FreeGtdtTableResources (
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
    DEBUG ((DEBUG_ERROR, "ERROR: GTDT: Invalid Table Pointer\n"));
    ASSERT ((Table != NULL) && (*Table != NULL));
    return EFI_INVALID_PARAMETER;
  }

  FreePool (*Table);
  *Table = NULL;
  return EFI_SUCCESS;
}

/** This macro defines the GTDT Table Generator revision.
*/
#define GTDT_GENERATOR_REVISION CREATE_REVISION (1, 0)

/** The interface for the GTDT Table Generator.
*/
STATIC
CONST
ACPI_TABLE_GENERATOR GtdtGenerator = {
  // Generator ID
  CREATE_STD_ACPI_TABLE_GEN_ID (ESTD_ACPI_TABLE_ID_GTDT),
  // Generator Description
  L"ACPI.STD.GTDT.GENERATOR",
  // ACPI Table Signature
  EFI_ACPI_6_1_GENERIC_TIMER_DESCRIPTION_TABLE_SIGNATURE,
  // ACPI Table Revision
  EFI_ACPI_6_1_GENERIC_TIMER_DESCRIPTION_TABLE_REVISION,
  // Creator ID
  TABLE_GENERATOR_CREATOR_ID_ARM,
  // Creator Revision
  GTDT_GENERATOR_REVISION,
  // Build Table function
  BuildGtdtTable,
  // Free Resource function
  FreeGtdtTableResources
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
AcpiGtdtLibConstructor (
  IN CONST EFI_HANDLE                ImageHandle,
  IN       EFI_SYSTEM_TABLE  * CONST SystemTable
  )
{
  EFI_STATUS  Status;

  Status = RegisterAcpiTableGenerator (&GtdtGenerator);
  DEBUG ((DEBUG_INFO, "GTDT: Register Generator. Status = %r\n", Status));
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
AcpiGtdtLibDestructor (
  IN CONST EFI_HANDLE                ImageHandle,
  IN       EFI_SYSTEM_TABLE  * CONST SystemTable
  )
{
  EFI_STATUS  Status;

  Status = UnRegisterAcpiTableGenerator (&GtdtGenerator);
  DEBUG ((DEBUG_INFO, "GTDT: Unregister Generator. Status = %r\n", Status));
  ASSERT_EFI_ERROR (Status);
  return Status;
}
