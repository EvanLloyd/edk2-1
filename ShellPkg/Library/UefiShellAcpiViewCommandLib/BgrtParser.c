/**
  BGRT table parser

  Copyright (c) 2017, ARM Limited. All rights reserved.
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

  @par Reference(s):
    - ACPI 6.2 Specification - Errata A, September 2017
**/

#include <IndustryStandard/Acpi.h>
#include <Library/UefiLib.h>
#include "AcpiParser.h"
#include "AcpiView.h"
#include "AcpiTableParser.h"

// Local variables
STATIC ACPI_DESCRIPTION_HEADER_INFO AcpiHdrInfo;

/** An ACPI_PARSER array describing the ACPI BDRT Table.
*/
STATIC CONST ACPI_PARSER BgrtParser[] = {
  PARSE_ACPI_HEADER (&AcpiHdrInfo),
  {L"Version", 2, 36, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Status", 1, 38, L"0x%x", NULL,  NULL, NULL, NULL},
  {L"Image Type", 1, 39, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Image Address", 8, 40, L"0x%lx", NULL, NULL, NULL, NULL},
  {L"Image Offset X", 4, 48, L"%d", NULL, NULL, NULL, NULL},
  {L"Image Offset Y", 4, 52, L"%d", NULL, NULL, NULL, NULL}
};

/** This function parses the ACPI BGRT table.
  When trace is enabled this function parses the BGRT table and
  traces the ACPI table fields.

  This function also parses the ACPI header for the DSDT table and
  invokes the parser for the ACPI DSDT table.

  This function also performs validation of the ACPI table fields.

  @param [in] Trace              If TRUE, trace the ACPI fields.
  @param [in] Ptr                Pointer to the start of the buffer.
  @param [in] AcpiTableLength    Length of the ACPI table.
  @param [in] AcpiTableRevision  Revision of the ACPI table.
*/
VOID
EFIAPI
ParseAcpiBgrt (
  IN BOOLEAN Trace,
  IN UINT8*  Ptr,
  IN UINT32  AcpiTableLength,
  IN UINT8   AcpiTableRevision
  )
{
  if (!Trace) {
    return;
  }

  ParseAcpi (
    Trace,
    0,
    "BGRT",
    Ptr,
    AcpiTableLength,
    PARSER_PARAMS (BgrtParser)
    );
}

/** Register the parser.

  @param [in]  ImageHandle  The handle to the image.
  @param [in]  SystemTable  Pointer to the System Table.

  @retval EFI_SUCCESS           The parser is registered.
  @retval EFI_ALREADY_STARTED   The parser for the ACPI Table
                                is already registered.
*/
EFI_STATUS
EFIAPI
BgrtParserLibConstructor (
  IN CONST EFI_HANDLE                ImageHandle,
  IN       EFI_SYSTEM_TABLE  * CONST SystemTable
  )
{
  return RegisterParser (
           EFI_ACPI_6_2_BOOT_GRAPHICS_RESOURCE_TABLE_SIGNATURE,
           ParseAcpiBgrt
           );
}

/** Deregister the parser.

  @param [in]  ImageHandle  The handle to the image.
  @param [in]  SystemTable  Pointer to the System Table.

  @retval EFI_SUCCESS           The parser is deregistered.
  @retval EFI_NOT_FOUND         The parser was not registered.
*/
EFI_STATUS
EFIAPI
BgrtParserLibDestructor (
  IN CONST EFI_HANDLE                ImageHandle,
  IN       EFI_SYSTEM_TABLE  * CONST SystemTable
  )
{
  return DeregisterParser (
           EFI_ACPI_6_2_BOOT_GRAPHICS_RESOURCE_TABLE_SIGNATURE
           );
}
