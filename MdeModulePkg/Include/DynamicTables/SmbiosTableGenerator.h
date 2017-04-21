/** @file

  Copyright (c) 2017, ARM Limited. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef SMBIOS_TABLE_GENERATOR_H_
#define SMBIOS_TABLE_GENERATOR_H_

#pragma pack(1)

/** The SMBIOS_TABLE_GENERATOR_ID type describes SMBIOS table generator ID.
*/
typedef TABLE_GENERATOR_ID SMBIOS_TABLE_GENERATOR_ID;

/** The ESTD_SMBIOS_TABLE_ID enum describes the SMBIOS table IDs reserved for
  the standard generators.

  NOTE: The SMBIOS Generator IDs do not match the table type numbers!
          This allows 0 to be used to catch invalid parameters.
*/
typedef enum StdSmbiosTableGeneratorId {
  ESTD_SMBIOS_TABLE_ID_RESERVED = 0x0000,
  ESTD_SMBIOS_TABLE_ID_RAW,
  ESTD_SMBIOS_TABLE_ID_TYPE00,
  ESTD_SMBIOS_TABLE_ID_TYPE01,
  ESTD_SMBIOS_TABLE_ID_TYPE02,
  ESTD_SMBIOS_TABLE_ID_TYPE03,
  ESTD_SMBIOS_TABLE_ID_TYPE04,
  ESTD_SMBIOS_TABLE_ID_TYPE05,
  ESTD_SMBIOS_TABLE_ID_TYPE06,
  ESTD_SMBIOS_TABLE_ID_TYPE07,
  ESTD_SMBIOS_TABLE_ID_TYPE08,
  ESTD_SMBIOS_TABLE_ID_TYPE09,
  ESTD_SMBIOS_TABLE_ID_TYPE10,
  ESTD_SMBIOS_TABLE_ID_TYPE11,
  ESTD_SMBIOS_TABLE_ID_TYPE12,
  ESTD_SMBIOS_TABLE_ID_TYPE13,
  ESTD_SMBIOS_TABLE_ID_TYPE14,
  ESTD_SMBIOS_TABLE_ID_TYPE15,
  ESTD_SMBIOS_TABLE_ID_TYPE16,
  ESTD_SMBIOS_TABLE_ID_TYPE17,
  ESTD_SMBIOS_TABLE_ID_TYPE18,
  ESTD_SMBIOS_TABLE_ID_TYPE19,
  ESTD_SMBIOS_TABLE_ID_TYPE20,
  ESTD_SMBIOS_TABLE_ID_TYPE21,
  ESTD_SMBIOS_TABLE_ID_TYPE22,
  ESTD_SMBIOS_TABLE_ID_TYPE23,
  ESTD_SMBIOS_TABLE_ID_TYPE24,
  ESTD_SMBIOS_TABLE_ID_TYPE25,
  ESTD_SMBIOS_TABLE_ID_TYPE26,
  ESTD_SMBIOS_TABLE_ID_TYPE27,
  ESTD_SMBIOS_TABLE_ID_TYPE28,
  ESTD_SMBIOS_TABLE_ID_TYPE29,
  ESTD_SMBIOS_TABLE_ID_TYPE30,
  ESTD_SMBIOS_TABLE_ID_TYPE31,
  ESTD_SMBIOS_TABLE_ID_TYPE32,
  ESTD_SMBIOS_TABLE_ID_TYPE33,
  ESTD_SMBIOS_TABLE_ID_TYPE34,
  ESTD_SMBIOS_TABLE_ID_TYPE35,
  ESTD_SMBIOS_TABLE_ID_TYPE36,
  ESTD_SMBIOS_TABLE_ID_TYPE37,
  ESTD_SMBIOS_TABLE_ID_TYPE38,
  ESTD_SMBIOS_TABLE_ID_TYPE39,
  ESTD_SMBIOS_TABLE_ID_TYPE40,
  ESTD_SMBIOS_TABLE_ID_TYPE41,
  ESTD_SMBIOS_TABLE_ID_TYPE42,

  // IDs 43 - 125 are reserved

  ESTD_SMBIOS_TABLE_ID_TYPE126 = (ESTD_SMBIOS_TABLE_ID_TYPE00 + 126),
  ESTD_SMBIOS_TABLE_ID_TYPE127,
  ESTD_SMBIOS_TABLE_ID_MAX
} ESTD_SMBIOS_TABLE_ID;

/** This macro checks if the Table Generator ID is for an SMBIOS Table
    Generator.
    @param [in] TableGeneratorId  The table generator ID.

    @returns  TRUE if the table generator ID is for an SMBIOS Table
              Generator.
*/
#define IS_GENERATOR_TYPE_SMBIOS(TableGeneratorId) \
          (                                        \
          GET_TABLE_TYPE (TableGeneratorId) ==     \
          ETableGeneratorTypeSmbios                \
          )

/** This macro checks if the Table Generator ID is for a standard SMBIOS
    Table Generator.

    @param [in] TableGeneratorId  The table generator ID.

    @returns  TRUE if the table generator ID is for a standard SMBIOS
              Table Generator.
*/
#define IS_VALID_STD_SMBIOS_GENERATOR_ID(TableGeneratorId)            \
          (                                                           \
          IS_GENERATOR_NAMESPACE_STD(TableGeneratorId) &&             \
          IS_GENERATOR_TYPE_SMBIOS(TableGeneratorId)   &&             \
          ((GET_TABLE_ID(GeneratorId) >= ESTD_SMBIOS_TABLE_ID_RAW) && \
           (GET_TABLE_ID(GeneratorId) < ESTD_SMIOS_TABLE_ID_MAX))     \
          )

/** This macro creates a standard SMBIOS Table Generator ID.

    @param [in] TableId  The table generator ID.

    @returns a standard SMBIOS table generator ID.
*/
#define CREATE_STD_SMBIOS_TABLE_GEN_ID(TableId) \
          CREATE_TABLE_GEN_ID (                 \
            ETableGeneratorTypeSmbios,          \
            ETableGeneratorNameSpaceStd,        \
            TableId                             \
            )

/** Forward declarations.
*/
typedef struct ConfigurationManagerProtocol EFI_CONFIGURATION_MANAGER_PROTOCOL;
typedef struct CmStdObjSmbiosTableInfo      CM_STD_OBJ_SMBIOS_TABLE_INFO;
typedef struct SmbiosTableGenerator         SMBIOS_TABLE_GENERATOR;

/** This function pointer describes the interface to SMBIOS table build
    functions provided by the SMBIOS table generator and called by the
    Table Manager to build an SMBIOS table.

    @param [in]  Generator       Pointer to the SMBIOS table generator.
    @param [in]  SmbiosTableInfo Pointer to the SMBIOS table information.
    @param [in]  CfgMgrProtocol  Pointer to the Configuration Manager
                                 Protocol interface.
    @param [out] Table           Pointer to the generated SMBIOS table.

    @returns EFI_SUCCESS  If the table is generated successfully or other
                          failure codes as returned by the generator.
*/
typedef EFI_STATUS (*SMBIOS_TABLE_GENERATOR_BUILD_TABLE) (
  IN  CONST SMBIOS_TABLE_GENERATOR              *       Generator,
  IN        CM_STD_OBJ_SMBIOS_TABLE_INFO        * CONST SmbiosTableInfo,
  IN  CONST EFI_CONFIGURATION_MANAGER_PROTOCOL  * CONST CfgMgrProtocol,
  OUT       SMBIOS_STRUCTURE                   **       Table
  );

/** This function pointer describes the interface to used by the
    Table Manager to give the generator an opportunity to free
    any resources allocated for building the SMBIOS table.

    @param [in]  Generator       Pointer to the SMBIOS table generator.
    @param [in]  SmbiosTableInfo Pointer to the SMBIOS table information.
    @param [in]  CfgMgrProtocol  Pointer to the Configuration Manager
                                 Protocol interface.
    @param [in]  Table           Pointer to the generated SMBIOS table.

    @returns  EFI_SUCCESS If freed successfully or other failure codes
                          as returned by the generator.
*/
typedef EFI_STATUS (*SMBIOS_TABLE_GENERATOR_FREE_TABLE) (
  IN  CONST SMBIOS_TABLE_GENERATOR              *       Generator,
  IN  CONST CM_STD_OBJ_SMBIOS_TABLE_INFO        * CONST SmbiosTableInfo,
  IN  CONST EFI_CONFIGURATION_MANAGER_PROTOCOL  * CONST CfgMgrProtocol,
  IN        SMBIOS_STRUCTURE                   **       Table
  );

/** The SMBIOS_TABLE_GENERATOR structure provides an interface that the
    Table Manager can use to invoke the functions to build SMBIOS tables.

*/
typedef struct SmbiosTableGenerator {
  /// The SMBIOS table generator ID.
  SMBIOS_TABLE_GENERATOR_ID                GeneratorID;

  /// String describing the DT table
  /// generator.
  CONST CHAR16*                            Description;

  /// The SMBIOS table type.
  SMBIOS_TYPE                              Type;

  /// SMBIOS table build function pointer.
  SMBIOS_TABLE_GENERATOR_BUILD_TABLE       BuildSmbiosTable;

  /// The function to free any resources
  /// allocated for building the SMBIOS table.
  SMBIOS_TABLE_GENERATOR_FREE_TABLE        FreeTableResources;
} SMBIOS_TABLE_GENERATOR;

/** Register SMBIOS table factory generator.

    The SMBIOS table factory maintains a list of the Standard and OEM SMBIOS
    table generators.

    @param [in]  Generator       Pointer to the SMBIOS table generator.

    @retval EFI_SUCCESS           The Generator was registered
                                  successfully.
    @retval EFI_INVALID_PARAMETER The Generator ID is invalid or
                                  the Generator pointer is NULL.
    @retval EFI_ALREADY_STARTED   The Generator for the Table ID is
                                  already registered.
*/
EFI_STATUS
EFIAPI
RegisterSmbiosTableGenerator (
  IN CONST SMBIOS_TABLE_GENERATOR                 * CONST Generator
  );

/** Unregister SMBIOS generator.

    This function is called by the SMBIOS table generator to unregister itself
    from the SMBIOS table factory.

    @param [in]  Generator       Pointer to the SMBIOS table generator.

    @retval EFI_SUCCESS           Success.
    @retval EFI_INVALID_PARAMETER The generator is invalid.
    @retval EFI_NOT_FOUND         The requested generator is not found
                                  in the list of registered generators.
*/
EFI_STATUS
EFIAPI
UnRegisterSmbiosTableGenerator (
  IN CONST SMBIOS_TABLE_GENERATOR                 * CONST Generator
  );
#pragma pack()

#endif // SMBIOS_TABLE_GENERATOR_H_

