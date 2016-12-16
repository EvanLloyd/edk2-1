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
#include "AcpiTableParser.h"

// Local variables
STATIC CONST UINT32* TableSignature;
STATIC CONST UINT32* XsdtLength;


/** An ACPI_PARSER array describing the ACPI XSDT table.

**/
STATIC CONST ACPI_PARSER XsdtParser[] = {
  PARSE_ACPI_HEADER (&TableSignature, &XsdtLength, NULL)
};


/** This function parses the ACPI XSDT table.
  This function parses the XSDT table and optionally traces the ACPI
  table fields.

  This function invokes the parsers for the following ACPI tables:
    - FADT
    - GTDT
    - SPCR
    - MADT
    - DBG2
    - MCFG
    - SSDT
    - SRAT
    - SLIT
    - IORT

  This function also performs validation of the XSDT table.

  @params [in] Trace              If TRUE, trace the ACPI fields.
  @params [in] Ptr                Pointer to the start of the buffer.
  @params [in] AcpiTableLength    Length of the ACPI table.
  @params [in] AcpiTableRevision  Revision of the ACPI table.
**/
VOID
ParseAcpiXsdt (
  IN BOOLEAN Trace,
  IN UINT8*  Ptr,
  IN UINT32  AcpiTableLength,
  IN UINT8   AcpiTableRevision
  )
{
  UINT32        Offset;
  UINT64*       TablePointer;

  // Parse the ACPI header to get the length
  ParseAcpi (
    FALSE,
    "XSDT",
    Ptr,
    ACPI_DESCRIPTION_HEADER_LENGTH,
    PARSER_PARAMS (XsdtParser)
    );

  Offset = ParseAcpi (
             Trace,
             "XSDT",
             Ptr,
             *XsdtLength,
             PARSER_PARAMS (XsdtParser)
             );

  Ptr += Offset;

  TablePointer = (UINT64*)Ptr;

  while (Offset < (*XsdtLength)) {
    Ptr = (UINT8*)(UINTN)(*TablePointer);

    ProcessAcpiTable (Ptr);


    Offset += sizeof (UINT64);
    TablePointer++;
  } // while
}
