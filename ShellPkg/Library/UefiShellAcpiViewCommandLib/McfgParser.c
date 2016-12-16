/** @file
*
*  Copyright (c) 2016, ARM Limited. All rights reserved.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/

#include <Library/UefiLib.h>
#include "AcpiParser.h"
#include "AcpiView.h"

/** An ACPI_PARSER array describing the ACPI MCFG Table.
**/
STATIC CONST ACPI_PARSER McfgParser[] = {
  PARSE_ACPI_HEADER (NULL, NULL, NULL),
  {L"Reserved", 8, 36, L"0x%lx", NULL, NULL, NULL, NULL},
};

/** An ACPI_PARSER array describing the PCI configuration Space
    Base Address structure.
**/
STATIC CONST ACPI_PARSER PciCfgSpaceBaseAddrParser[] = {
  {L"Base Address", 8, 0, L"0x%lx", NULL, NULL, NULL, NULL},
  {L"PCI Segment Group No.", 2, 8, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Start Bus No.", 1, 10, L"0x%x", NULL, NULL, NULL, NULL},
  {L"End Bus No.", 1, 11, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Reserved", 4, 12, L"0x%x", NULL, NULL, NULL, NULL}
};


/** This function parses the ACPI MCFG table.
  This function parses the MCFG table and optionally traces the ACPI
  table fields.

  This function also performs validation of the ACPI table fields.

  @params [in] Trace              If TRUE, trace the ACPI fields.
  @params [in] Ptr                Pointer to the start of the buffer.
  @params [in] AcpiTableLength    Length of the ACPI table.
  @params [in] AcpiTableRevision  Revision of the ACPI table.
**/
VOID
ParseAcpiMcfg (
  IN BOOLEAN Trace,
  IN UINT8*  Ptr,
  IN UINT32  AcpiTableLength,
  IN UINT8   AcpiTableRevision
  )
{
  UINT32 Offset;
  UINT32 PciCfgOffset;
  UINT8* PciCfgSpacePtr;

  if (!Trace) {
    return;
  }

  Offset = ParseAcpi (
             TRUE,
             "MCFG",
             Ptr,
             AcpiTableLength,
             PARSER_PARAMS (McfgParser)
             );

  PciCfgSpacePtr = Ptr + Offset;

  while (Offset < AcpiTableLength) {
    PciCfgOffset = ParseAcpi (
                     TRUE,
                     "PCI Configuration Space",
                     PciCfgSpacePtr,
                     (AcpiTableLength - Offset),
                     PARSER_PARAMS (PciCfgSpaceBaseAddrParser)
                     );
    PciCfgSpacePtr += PciCfgOffset;
    Offset += PciCfgOffset;
  }
}
