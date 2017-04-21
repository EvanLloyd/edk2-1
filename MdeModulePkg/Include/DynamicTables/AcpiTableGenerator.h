/** @file

  Copyright (c) 2017, ARM Limited. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

  @par Glossary:
    - Cm or CM   - Configuration Manager
    - Obj or OBJ - Object
    - Std or STD - Standard
**/

#ifndef ACPI_TABLE_GENERATOR_H_
#define ACPI_TABLE_GENERATOR_H_

#pragma pack(1)

/**
The Dynamic Tables Framework provisions two classes of ACPI table
generators.
 - Standard generators: The ACPI table generators implemented by the
                        Dynamic Tables Framework.
 - OEM generators:      The ACPI table generatos customized by the OEM.

The Dynamic Tables Framework implements the following ACPI table generators:
  - RAW   : This is the simplest ACPI table generator. It simply installs
            the ACPI table provided in the AcpiTableData member of the
            CM_STD_OBJ_ACPI_TABLE_INFO. The ACPI table data is provided by
            the Configuration Manager and is generated using an implementation
            defined mechanism.
  - DSDT  : The DSDT generator is clone of the RAW generator. The difference
            is in the way the ACPI Table Data is generated from an AML file.
  - SSDT  : The SSDT generator is clone of the RAW generator. The difference
            is in the way the ACPI Table Data is generated from an AML file.
  - FADT  : The FADT generator collates the required platform information for
            building the ACPI FADT table from the Configuration Manager and
            generates the FADT table.
  - MADT  : The MADT generator collates the GIC information for building the
            ACPI MADT table from the Configuration Manager and generates the
            MADT table.
  - GTDT  : The GTDT generator collates the Timer information for building the
            ACPI GTDT table from the Configuration Manager and generates the
            GTDT table.
  - DBG2  : The DBG2 generator collates the debug serial port information for
            building the DBG2 table from the Configuration Manager and
            generates the GTDT table.
  - SPCR  : The SPCR generator collates the serial port information for
            building the SPCR table from the Configuration Manager and
            generates the SPCR table.
  - MCFG  : The MCFG generator collates the PCI configuration space information
            for building the MCFG table from the Configuration Manager and
            generates the MCFG table.
*/

/** The ACPI_TABLE_GENERATOR_ID type describes ACPI table generator ID.
*/
typedef TABLE_GENERATOR_ID ACPI_TABLE_GENERATOR_ID;

/** The ESTD_ACPI_TABLE_ID enum describes the ACPI table IDs reserved for
  the standard generators.
*/
typedef enum StdAcpiTableId {
  ESTD_ACPI_TABLE_ID_RESERVED = 0x0000,             ///< Reserved.
  ESTD_ACPI_TABLE_ID_RAW,                           ///< RAW Generator.
  ESTD_ACPI_TABLE_ID_DSDT = ESTD_ACPI_TABLE_ID_RAW, ///< DSDT Generator.
  ESTD_ACPI_TABLE_ID_SSDT = ESTD_ACPI_TABLE_ID_RAW, ///< SSDT Generator.
  ESTD_ACPI_TABLE_ID_FADT,                          ///< FADT Generator.
  ESTD_ACPI_TABLE_ID_MADT,                          ///< MADT Generator.
  ESTD_ACPI_TABLE_ID_GTDT,                          ///< GTDT Generator.
  ESTD_ACPI_TABLE_ID_DBG2,                          ///< DBG2 Generator.
  ESTD_ACPI_TABLE_ID_SPCR,                          ///< SPCR Generator.
  ESTD_ACPI_TABLE_ID_MCFG,                          ///< MCFG Generator.
  ESTD_ACPI_TABLE_ID_MAX
} ESTD_ACPI_TABLE_ID;

/** This macro checks if the Table Generator ID is for an ACPI Table Generator.
    @param [in] TableGeneratorId  The table generator ID.

    @returns TRUE if the table generator ID is for an ACPI Table
          Generator.
*/
#define IS_GENERATOR_TYPE_ACPI(TableGeneratorId) \
          (GET_TABLE_TYPE (TableGeneratorId) == ETableGeneratorTypeAcpi)

/** This macro checks if the Table Generator ID is for a standard ACPI
    Table Generator.

    @param [in] TableGeneratorId  The table generator ID.

    @returns TRUE if the table generator ID is for a standard ACPI
            Table Generator.
*/
#define IS_VALID_STD_ACPI_GENERATOR_ID(TableGeneratorId)               \
          (                                                            \
          IS_GENERATOR_NAMESPACE_STD (TableGeneratorId) &&             \
          IS_GENERATOR_TYPE_ACPI (TableGeneratorId)     &&             \
          ((GET_TABLE_ID (GeneratorId) >= ESTD_ACPI_TABLE_ID_RAW) &&   \
           (GET_TABLE_ID (GeneratorId) < ESTD_ACPI_TABLE_ID_MAX))      \
          )

/** This macro creates a standard ACPI Table Generator ID.

    @param [in] TableId  The table generator ID.

    @returns a standard ACPI table generator ID.
*/
#define CREATE_STD_ACPI_TABLE_GEN_ID(TableId) \
          CREATE_TABLE_GEN_ID (               \
            ETableGeneratorTypeAcpi,          \
            ETableGeneratorNameSpaceStd,      \
            TableId                           \
            )

/** The Creator ID for the ACPI tables generated using
  the standard ACPI table generators.
*/
#define TABLE_GENERATOR_CREATOR_ID_ARM  SIGNATURE_32('A', 'R', 'M', 'H')

/** A macro to initialise the common header part of EFI ACPI tables as
    defined by the EFI_ACPI_DESCRIPTION_HEADER structure.

    @param [in] Signature The ACPI table signature.
    @param [in] Type      The ACPI table structure.
    @param [in] Revision  The ACPI table revision.
*/
#define ACPI_HEADER(Signature, Type, Revision) {              \
          Signature,             /* UINT32  Signature */      \
          sizeof (Type),         /* UINT32  Length */         \
          Revision,              /* UINT8   Revision */       \
          0,                     /* UINT8   Checksum */       \
          { 0, 0, 0, 0, 0, 0 },  /* UINT8   OemId[6] */       \
          0,                     /* UINT64  OemTableId */     \
          0,                     /* UINT32  OemRevision */    \
          0,                     /* UINT32  CreatorId */      \
          0                      /* UINT32  CreatorRevision */\
          }

/** A macro to dump the common header part of EFI ACPI tables as
    defined by the EFI_ACPI_DESCRIPTION_HEADER structure.

    @param [in] AcpiHeader The pointer to the ACPI table header.
*/
#define DUMP_ACPI_TABLE_HEADER(AcpiHeader)                        \
          DEBUG ((                                                \
            DEBUG_INFO,                                           \
            "ACPI TABLE %c%c%c%c : Rev 0x%x : Length : 0x%x\n",   \
            (AcpiHeader->Signature & 0xFF),                       \
            ((AcpiHeader->Signature >> 8) & 0xFF),                \
            ((AcpiHeader->Signature >> 16) & 0xFF),               \
            ((AcpiHeader->Signature >> 24) & 0xFF),               \
            AcpiHeader->Revision,                                 \
            AcpiHeader->Length                                    \
            ));

/** Forward declarations.
*/
typedef struct ConfigurationManagerProtocol EFI_CONFIGURATION_MANAGER_PROTOCOL;
typedef struct CmAStdObjAcpiTableInfo       CM_STD_OBJ_ACPI_TABLE_INFO;
typedef struct AcpiTableGenerator           ACPI_TABLE_GENERATOR;

/** This function pointer describes the interface to ACPI table build
    functions provided by the ACPI table generator and called by the
    Table Manager to build an ACPI table.

    @param [in]  Generator       Pointer to the ACPI table generator.
    @param [in]  AcpiTableInfo   Pointer to the ACPI table information.
    @param [in]  CfgMgrProtocol  Pointer to the Configuration Manager
                                 Protocol interface.
    @param [out] Table           Pointer to the generated ACPI table.

    @returns  EFI_SUCCESS If the table is generated successfully or other
                          failure codes as returned by the generator.
*/
typedef EFI_STATUS (*ACPI_TABLE_GENERATOR_BUILD_TABLE) (
  IN  CONST ACPI_TABLE_GENERATOR                *       Generator,
  IN  CONST CM_STD_OBJ_ACPI_TABLE_INFO          * CONST AcpiTableInfo,
  IN  CONST EFI_CONFIGURATION_MANAGER_PROTOCOL  * CONST CfgMgrProtocol,
  OUT       EFI_ACPI_DESCRIPTION_HEADER        **       Table
  );

/** This function pointer describes the interface to used by the
    Table Manager to give the generator an opportunity to free
    any resources allocated for building the ACPI table.

    @param [in]  Generator       Pointer to the ACPI table generator.
    @param [in]  AcpiTableInfo   Pointer to the ACPI table information.
    @param [in]  CfgMgrProtocol  Pointer to the Configuration Manager
                                 Protocol interface.
    @param [in]  Table           Pointer to the generated ACPI table.

    @returns EFI_SUCCESS  If freed successfully or other failure codes
                          as returned by the generator.
*/
typedef EFI_STATUS (*ACPI_TABLE_GENERATOR_FREE_TABLE) (
  IN  CONST ACPI_TABLE_GENERATOR                *       Generator,
  IN  CONST CM_STD_OBJ_ACPI_TABLE_INFO          * CONST AcpiTableInfo,
  IN  CONST EFI_CONFIGURATION_MANAGER_PROTOCOL  * CONST CfgMgrProtocol,
  IN        EFI_ACPI_DESCRIPTION_HEADER        **       Table
  );

/** The ACPI_TABLE_GENERATOR structure provides an interface that the
    Table Manager can use to invoke the functions to build ACPI tables.

*/
typedef struct AcpiTableGenerator {
  /// The ACPI table generator ID.
  ACPI_TABLE_GENERATOR_ID                GeneratorID;

  /// String describing the ACPI table
  /// generator.
  CONST CHAR16                         * Description;

  /// The ACPI table signature.
  UINT32                                 AcpiTableSignature;

  /// The ACPI table revision.
  UINT32                                 AcpiTableRevision;

  /// The ACPI table creator ID.
  UINT32                                 CreatorId;

  /// The ACPI table creator revision.
  UINT32                                 CreatorRevision;

  /// ACPI table build function pointer.
  ACPI_TABLE_GENERATOR_BUILD_TABLE       BuildAcpiTable;

  /// The function to free any resources
  /// allocated for building the ACPI table.
  ACPI_TABLE_GENERATOR_FREE_TABLE        FreeTableResources;
} ACPI_TABLE_GENERATOR;

/** Register ACPI table factory generator.

    The ACPI table factory maintains a list of the Standard and OEM ACPI
    table generators.

    @param [in]  Generator       Pointer to the ACPI table generator.

    @retval EFI_SUCCESS           The Generator was registered
                                  successfully.
    @retval EFI_INVALID_PARAMETER The Generator ID is invalid or
                                  the Generator pointer is NULL.
    @retval EFI_ALREADY_STARTED   The Generator for the Table ID is
                                  already registered.
*/
EFI_STATUS
EFIAPI
RegisterAcpiTableGenerator (
  IN CONST ACPI_TABLE_GENERATOR                 * CONST Generator
  );

/** Unregister ACPI generator.

    This function is called by the ACPI table generator to unregister itself
    from the ACPI table factory.

    @param [in]  Generator       Pointer to the ACPI table generator.

    @retval EFI_SUCCESS           Success.
    @retval EFI_INVALID_PARAMETER The generator is invalid.
    @retval EFI_NOT_FOUND         The requested generator is not found
                                  in the list of registered generators.
*/
EFI_STATUS
EFIAPI
UnRegisterAcpiTableGenerator (
  IN CONST ACPI_TABLE_GENERATOR                 * CONST Generator
  );

#pragma pack()

#endif // ACPI_TABLE_GENERATOR_H_

