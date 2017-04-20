/**
  Header file for ACPI table parser

  Copyright (c) 2016 - 2017, ARM Limited. All rights reserved.
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef ACPITABLEPARSER_H_
#define ACPITABLEPARSER_H_

/** The maximum number of ACPI table parsers.
*/
#define MAX_ACPI_TABLE_PARSERS          16

/** An invalid/NULL signature value.
*/
#define ACPI_PARSER_SIGNATURE_NULL      0

/** A function that parses the ACPI table.

  @param [in] Trace              If TRUE, trace the ACPI fields.
  @param [in] Ptr                Pointer to the start of the buffer.
  @param [in] AcpiTableLength    Length of the ACPI table.
  @param [in] AcpiTableRevision  Revision of the ACPI table.
*/
typedef
VOID
EFIAPI
(EFIAPI * PARSE_ACPI_TABLE_PROC) (
  IN BOOLEAN Trace,
  IN UINT8*  Ptr,
  IN UINT32  AcpiTableLength,
  IN UINT8   AcpiTableRevision
  );

/** The ACPI table parser information
*/
typedef struct AcpiTableParser {
  /// ACPI table signature
  UINT32                  Signature;

  /// The ACPI table parser function.
  PARSE_ACPI_TABLE_PROC   Parser;
} ACPI_TABLE_PARSER;

/** Register the ACPI table Parser

  This function registers the ACPI table parser..

  @param [in] Signature   The ACPI table Signature.
  @param [in] ParserProc  The ACPI table parser.

  @retval EFI_SUCCESS           The parser is registered.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_ALREADY_STARTED   The parser for the Table
                                is already registered.
  @retval EFI_OUT_OF_RESOURCES  No space to register the
                                parser.
*/
EFI_STATUS
EFIAPI
RegisterParser (
  IN  UINT32                  Signature,
  IN  PARSE_ACPI_TABLE_PROC   ParserProc
);

/** Deregister the ACPI table Parser

  This function deregisters the ACPI table parser..

  @param [in] Signature   The ACPI table Signature.

  @retval EFI_SUCCESS           The parser was deregistered.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         A registered parser was not found.
*/
EFI_STATUS
EFIAPI
DeregisterParser (
  IN  UINT32                  Signature
);

/** This function processes the ACPI tables.
  This function calls ProcessTableReportOptions() to list the ACPI
  tables, perform binary dump of the tables and determine if the
  ACPI fields should be traced.

  This function also invokes the parser for the ACPI tables.

  This function also performs a RAW dump of the ACPI table including
  the unknown/unparsed ACPI tables and validates the checksum.

  @param [in] Ptr                Pointer to the start of the ACPI
                                 table data buffer.
*/
VOID
ProcessAcpiTable (
  IN UINT8* Ptr
  );

/** This function parses the ACPI RSDP table.
  This function parses the RSDP table and optionally traces the ACPI
  table fields. ProcessTableReportOptions() is called to determine if
  the ACPI fields should be traced.

  This function invokes the parser for the XSDT table.
  * Note - This function does not support parsing of RSDT table.

  This function also performs a RAW dump of the ACPI table and
  validates the checksum.

  @param [in] Ptr                Pointer to the start of the buffer.

  @retval EFI_SUCCESS            Success.
  @retval EFI_NOT_FOUND          Valid XSDT pointer not found.
*/
EFI_STATUS
ParseRsdp (
  IN UINT8* Ptr
  );

#endif // ACPITABLEPARSER_H_
