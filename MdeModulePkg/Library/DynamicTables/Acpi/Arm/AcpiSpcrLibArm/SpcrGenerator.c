/** @file
  SPCR Table Generator

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
#include <IndustryStandard/SerialPortConsoleRedirectionTable.h>

#include <DynamicTables/TableGenerator.h>
#include <DynamicTables/AcpiTableGenerator.h>
#include <DynamicTables/ConfigurationManagerObject.h>
#include <DynamicTables/ConfigurationManagerHelper.h>
#include <DynamicTables/StandardNameSpaceObjects.h>
#include <DynamicTables/ArmNameSpaceObjects.h>
#include <Protocol/ConfigurationManagerProtocol.h>
#include <Library/TableHelperLib.h>

/** ARM standard SPCR Table Generator

    Generates the ACPI SPCR Table for ARM UARTs as specified by
    the Microsoft Serial Port Console Redirection Table
    Specification - Version 1.03 - August 10, 2015.

    Constructs the SPCR table for PL011 or SBSA UART peripherals.

Requirements:
  The following Configuration Manager Object(s) are required by
  this Generator:
  - EArmObjSerialConsolePortInfo

NOTE: This implementation ignores the possibility that the Serial settings may
      be modified from the UEFI Shell.  A more complex handler would be needed
      to (e.g.) recover serial port settings from the UART, or non-volatile
      storage.
*/

#pragma pack(1)

/** This macro defines the no flow control option.
*/
#define SPCR_FLOW_CONTROL_NONE           0

/**A template for generating the SPCR Table.

  Note: fields marked "{Template}" will be updated dynamically.
*/
STATIC
EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE AcpiSpcr = {
  ACPI_HEADER (
    EFI_ACPI_6_1_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_SIGNATURE,
    EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE,
    EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_REVISION
    ),
  EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_INTERFACE_TYPE_ARM_PL011_UART,
  {
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE,
    EFI_ACPI_RESERVED_BYTE
  },
  ARM_GAS32 (0), // {Template}: Serial Port Base Address
  EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_INTERRUPT_TYPE_GIC,
  0, // Not used on ARM
  0, // {Template}: Serial Port Interrupt
  0, // {Template}: Serial Port Baudrate
  EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_PARITY_NO_PARITY,
  EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_STOP_BITS_1,
  SPCR_FLOW_CONTROL_NONE,
  EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_TERMINAL_TYPE_ANSI,
  EFI_ACPI_RESERVED_BYTE,
  0xFFFF,
  0xFFFF,
  0x00,
  0x00,
  0x00,
  0x00000000,
  0x00,
  EFI_ACPI_RESERVED_DWORD
};

#pragma pack()

/** This macro expands to a function that retrieves the Serial
    Port Information from the Configuration Manager.
*/
GET_OBJECT_LIST (
  EObjNameSpaceArm,
  EArmObjSerialConsolePortInfo,
  CM_ARM_SERIAL_PORT_INFO
  )

/** Construct the SPCR ACPI table.

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
    @retval EFI_UNSUPPORTED       An unsupported baudrate was specified by the
                                  Confguration Manager.
    @retval EFI_BAD_BUFFER_SIZE   The size returned by the Configuration Manager
                                  is less than the Object size for the requested
                                  object.
*/
STATIC
EFI_STATUS
EFIAPI
BuildSpcrTable (
  IN  CONST ACPI_TABLE_GENERATOR                * CONST This,
  IN  CONST CM_STD_OBJ_ACPI_TABLE_INFO          * CONST AcpiTableInfo,
  IN  CONST EFI_CONFIGURATION_MANAGER_PROTOCOL  * CONST CfgMgrProtocol,
  OUT       EFI_ACPI_DESCRIPTION_HEADER        ** CONST Table
  )
{
  EFI_STATUS                 Status;
  CM_ARM_SERIAL_PORT_INFO  * SerialPortInfo = NULL;

  ASSERT (This != NULL);
  ASSERT (AcpiTableInfo != NULL);
  ASSERT (CfgMgrProtocol != NULL);
  ASSERT (Table != NULL);
  ASSERT (AcpiTableInfo->TableGeneratorId == This->GeneratorID);
  ASSERT (AcpiTableInfo->AcpiTableSignature == This->AcpiTableSignature);

  *Table = NULL;

  Status = GetEArmObjSerialConsolePortInfo (
             CfgMgrProtocol,
             &SerialPortInfo,
             NULL
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: SPCR: Failed to get serial port information. Status = %r\n",
      Status
      ));
    goto error_handler;
  }

  DEBUG ((DEBUG_INFO, "SPCR UART Configuration:\n"));
  DEBUG ((DEBUG_INFO, "  UART Base  = 0x%lx\n", SerialPortInfo->BaseAddress));
  DEBUG ((DEBUG_INFO, "  Clock      = %d\n", SerialPortInfo->Clock));
  DEBUG ((DEBUG_INFO, "  Baudrate   = %ld\n", SerialPortInfo->BaudRate));
  DEBUG ((DEBUG_INFO, "  Interrupt  = %d\n", SerialPortInfo->Interrupt));

  Status = AddAcpiHeader (
             CfgMgrProtocol,
             This,
             (EFI_ACPI_DESCRIPTION_HEADER*)&AcpiSpcr,
             sizeof (EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE)
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "ERROR: SPCR: Failed to add ACPI header. Status = %r\n",
      Status
      ));
    goto error_handler;
  }

  AcpiSpcr.BaseAddress.Address = SerialPortInfo->BaseAddress;
  AcpiSpcr.GlobalSystemInterrupt = SerialPortInfo->Interrupt;

  switch (SerialPortInfo->BaudRate) {
    case 9600:
      AcpiSpcr.BaudRate =
        EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_BAUD_RATE_9600;
      break;
    case 19200:
      AcpiSpcr.BaudRate =
        EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_BAUD_RATE_19200;
      break;
    case 57600:
      AcpiSpcr.BaudRate =
        EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_BAUD_RATE_57600;
      break;
    case 115200:
      AcpiSpcr.BaudRate =
        EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_BAUD_RATE_115200;
      break;
    default:
      Status = EFI_UNSUPPORTED;
      DEBUG ((
        DEBUG_ERROR,
        "ERROR: SPCR: Invalid Baud Rate %ld, Status = %r\n",
        SerialPortInfo->BaudRate,
        Status
        ));
      goto error_handler;
  } // switch

  *Table = (EFI_ACPI_DESCRIPTION_HEADER*)&AcpiSpcr;

error_handler:
  return Status;
}

/** Free any resources allocated for constructing the SPCR.

    @param [in]  This           Pointer to the table generator.
    @param [in]  AcpiTableInfo  Pointer to the ACPI Table Info.
    @param [in]  CfgMgrProtocol Pointer to the Configuration Manager
                                Protocol Interface.
    @param [in]  Table          Pointer to the ACPI Table.

    @retval EFI_SUCCESS         The resources were freed successfully.
*/
STATIC
EFI_STATUS
FreeSpcrTableResources (
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

  return EFI_SUCCESS;
}

/** This macro defines the SPCR Table Generator revision.
*/
#define SPCR_GENERATOR_REVISION CREATE_REVISION (1, 0)

/** The interface for the SPCR Table Generator.
*/
STATIC
CONST
ACPI_TABLE_GENERATOR SpcrGenerator = {
  // Generator ID
  CREATE_STD_ACPI_TABLE_GEN_ID (ESTD_ACPI_TABLE_ID_SPCR),
  // Generator Description
  L"ACPI.STD.SPCR.GENERATOR",
  // ACPI Table Signature
  EFI_ACPI_6_1_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_SIGNATURE,
  // ACPI Table Revision
  EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_REVISION,
  // Creator ID
  TABLE_GENERATOR_CREATOR_ID_ARM,
  // Creator Revision
  SPCR_GENERATOR_REVISION,
  // Build Table function
  BuildSpcrTable,
  // Free Resource function
  FreeSpcrTableResources
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
AcpiSpcrLibConstructor (
  IN CONST EFI_HANDLE                ImageHandle,
  IN       EFI_SYSTEM_TABLE  * CONST SystemTable
  )
{
  EFI_STATUS  Status;

  Status = RegisterAcpiTableGenerator (&SpcrGenerator);
  DEBUG ((DEBUG_INFO, "SPCR: Register Generator. Status = %r\n", Status));
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
AcpiSpcrLibDestructor (
  IN CONST EFI_HANDLE                ImageHandle,
  IN       EFI_SYSTEM_TABLE  * CONST SystemTable
  )
{
  EFI_STATUS  Status;

  Status = UnRegisterAcpiTableGenerator (&SpcrGenerator);
  DEBUG ((DEBUG_INFO, "SPCR: Unregister Generator. Status = %r\n", Status));
  ASSERT_EFI_ERROR (Status);
  return Status;
}
