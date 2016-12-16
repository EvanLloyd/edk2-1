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

#include <IndustryStandard/Acpi.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include "AcpiParser.h"
#include "AcpiView.h"

// Local Variables
STATIC CONST UINT8* SratRAType;
STATIC CONST UINT8* SratRALength;

/** This function validates the Reserved field in the SRAT table header.

  @params [in] Ptr     Pointer to the start of the field data.
  @params [in] Context Pointer to context specific information e.g. this
                       could be a pointer to the ACPI table header.
**/
STATIC
VOID
ValidateSratReserved (
  IN UINT8* Ptr,
  IN VOID*  Context
  );

/** This function traces the APIC Proximity Domain field.

  @params [in] Format  Format string for tracing the data.
  @params [in] Ptr     Pointer to the start of the buffer.

**/
STATIC
VOID
DumpSratApicProximity (
  IN  CONST CHAR16*  Format,
  IN  UINT8*         Ptr
  );

/** An ACPI_PARSER array describing the SRAT Table.

**/
STATIC CONST ACPI_PARSER SratParser[] = {
  PARSE_ACPI_HEADER (NULL, NULL, NULL),
  {L"Reserved", 4, 36, L"0x%x", NULL, NULL, ValidateSratReserved, NULL},
  {L"Reserved", 8, 40, L"0x%lx", NULL, NULL, NULL, NULL}
};

/** An ACPI_PARSER array describing the Resource Allocation
    structure header.

**/
STATIC CONST ACPI_PARSER SratResourceAllocationParser[] = {
  {L"Type", 1, 0, NULL, NULL, (VOID**)&SratRAType, NULL, NULL},
  {L"Length", 1, 1, NULL, NULL, (VOID**)&SratRALength, NULL, NULL}
};

/** An ACPI_PARSER array describing the GICC Affinity structure.

**/
STATIC CONST ACPI_PARSER SratGicCAffinityParser[] = {
  {L"Type", 1, 0, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Length", 1, 1, L"0x%x", NULL, NULL, NULL, NULL},

  {L"Proximity Domain", 4, 2, L"0x%x", NULL, NULL, NULL, NULL},
  {L"ACPI Processor UID", 4, 6, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Flags", 4, 10, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Clock Domain", 4, 14, L"0x%x", NULL, NULL, NULL, NULL}
};

/** An ACPI_PARSER array describing the Memory Affinity structure.

**/
STATIC CONST ACPI_PARSER SratMemAffinityParser[] = {
  {L"Type", 1, 0, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Length", 1, 1, L"0x%x", NULL, NULL, NULL, NULL},

  {L"Proximity Domain", 4, 2, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Reserved", 2, 6, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Base Address Low", 4, 8, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Base Address High", 4, 12, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Length Low", 4, 16, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Length High", 4, 20, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Reserved", 4, 24, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Flags", 4, 28, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Reserved", 8, 32, L"0x%lx", NULL, NULL, NULL, NULL}
};

/** An ACPI_PARSER array describing the APIC/SAPIC Affinity structure.

**/
STATIC CONST ACPI_PARSER SratApciSapicAffinityParser[] = {
  {L"Type", 1, 0, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Length", 1, 1, L"0x%x", NULL, NULL, NULL, NULL},

  {L"Proximity Domain [7:0]", 1, 2, L"0x%x", NULL, NULL, NULL, NULL},
  {L"APIC ID", 1, 3, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Flags", 4, 4, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Local SAPIC EID", 1, 8, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Proximity Domain [31:8]", 3, 9, L"0x%x", DumpSratApicProximity,
   NULL, NULL, NULL},
  {L"Clock Domain", 4, 12, L"0x%x", NULL, NULL, NULL, NULL}
};

/** An ACPI_PARSER array describing the Processor Local x2APIC
    Affinity structure.

**/
STATIC CONST ACPI_PARSER SratX2ApciAffinityParser[] = {
  {L"Type", 1, 0, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Length", 1, 1, L"0x%x", NULL, NULL, NULL, NULL},

  {L"Reserved", 2, 2, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Proximity Domain", 4, 4, L"0x%x", NULL, NULL, NULL, NULL},
  {L"X2APIC ID", 4, 8, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Flags", 4, 12, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Clock Domain", 4, 16, L"0x%x", NULL, NULL, NULL, NULL},
  {L"Reserved", 4, 20, L"0x%x", NULL, NULL, NULL, NULL}
};


/** This function validates the Reserved field in the SRAT table header.

  @params [in] Ptr     Pointer to the start of the field data.
  @params [in] Context Pointer to context specific information e.g. this
                       could be a pointer to the ACPI table header.
**/
STATIC
VOID
ValidateSratReserved (
  IN UINT8* Ptr,
  IN VOID*  Context
  )
{
  if (1 != *(UINT32*)Ptr) {
    IncrementErrorCount ();
    Print (L"\nERROR: Reserved should be 1 for backward compatibility.");
  }
}

/** This function traces the APIC Proximity Domain field.

  @params [in] Format  Format string for tracing the data.
  @params [in] Ptr     Pointer to the start of the buffer.

**/
STATIC
VOID
DumpSratApicProximity (
 IN CONST CHAR16* Format,
 IN UINT8*        Ptr
 )
{
  UINT32 ProximityDomain = Ptr[0] | (Ptr[1] << 8) | (Ptr[2] << 16);
  Print (Format, ProximityDomain);
}



/** This function parses the ACPI SRAT table.
  This function parses the SRAT table and optionally traces the ACPI
  table fields.

  This function parses the following Resource Allocation Structures:
    - Processor Local APIC/SAPIC Affinity Structure
    - Memory Affinity Structure
    - Processor Local x2APIC Affinity Structure
    - GICC Affinity Structure

  This function also performs validation of the ACPI table fields.

  @params [in] Trace              If TRUE, trace the ACPI fields.
  @params [in] Ptr                Pointer to the start of the buffer.
  @params [in] AcpiTableLength    Length of the ACPI table.
  @params [in] AcpiTableRevision  Revision of the ACPI table.
**/
VOID
ParseAcpiSrat (
  IN BOOLEAN Trace,
  IN UINT8*  Ptr,
  IN UINT32  AcpiTableLength,
  IN UINT8   AcpiTableRevision
  )
{
  UINT32 Offset;
  UINT8* ResourcePtr;
  UINT32 GicCAffinityIndex = 0;
  UINT32 MemoryAffinityIndex = 0;
  UINT32 ApicSapicAffinityIndex = 0;
  UINT32 X2ApicAffinityIndex = 0;
  CHAR8  Buffer[80];  // Used for AsciiName param of ParseAcpi

  if (!Trace) {
    return;
  }

  Offset = ParseAcpi (
             TRUE,
             "SRAT",
             Ptr,
             AcpiTableLength,
             PARSER_PARAMS (SratParser)
             );
  ResourcePtr = Ptr + Offset;

  while (Offset < AcpiTableLength) {
    ParseAcpi (
      FALSE,
      NULL,
      ResourcePtr,
      2,  // The length is 1 byte at offset 1
      PARSER_PARAMS (SratResourceAllocationParser)
      );

    switch (*SratRAType) {
      case EFI_ACPI_6_1_GICC_AFFINITY:
        AsciiSPrint (
          Buffer,
          sizeof (Buffer),
          "GICC Affinity Structure [%d]",
          GicCAffinityIndex++
          );
        ParseAcpi (
          TRUE,
          Buffer,
          ResourcePtr,
          *SratRALength,
          PARSER_PARAMS (SratGicCAffinityParser)
          );
        break;

      case EFI_ACPI_6_1_MEMORY_AFFINITY:
        AsciiSPrint (
          Buffer,
          sizeof (Buffer),
          "Memory Affinity Structure [%d]",
          MemoryAffinityIndex++
          );
        ParseAcpi (
          TRUE,
          Buffer,
          ResourcePtr,
          *SratRALength,
          PARSER_PARAMS (SratMemAffinityParser)
          );
        break;

      case EFI_ACPI_6_1_PROCESSOR_LOCAL_APIC_SAPIC_AFFINITY:
        AsciiSPrint (
          Buffer,
          sizeof (Buffer),
          "APIC/SAPIC Affinity Structure [%d]",
          ApicSapicAffinityIndex++
          );
        ParseAcpi (
          TRUE,
          Buffer,
          ResourcePtr,
          *SratRALength,
          PARSER_PARAMS (SratApciSapicAffinityParser)
          );
        break;

      case EFI_ACPI_6_1_PROCESSOR_LOCAL_X2APIC_AFFINITY:
        AsciiSPrint (
          Buffer,
          sizeof (Buffer),
          "X2APIC Affinity Structure [%d]",
          X2ApicAffinityIndex++
          );
        ParseAcpi (
          TRUE,
          Buffer,
          ResourcePtr,
          *SratRALength,
          PARSER_PARAMS (SratX2ApciAffinityParser)
          );
        break;

      default:
        IncrementErrorCount ();
        Print (L"ERROR: Unknown SRAT Affinity type\n");
        break;
    }

    ResourcePtr += (*SratRALength);
    Offset += (*SratRALength);
  }

}
