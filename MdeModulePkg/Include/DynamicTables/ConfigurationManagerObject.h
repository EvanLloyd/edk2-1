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
**/

#ifndef CONFIGURATION_MANAGER_OBJECT_H_
#define CONFIGURATION_MANAGER_OBJECT_H_

#pragma pack(1)

/** The CM_OBJECT_ID type is used to identify the Configration Manager objects.

 Description of Configuration Manager Object ID
________________________________________________________________________________
|31 |30 |29 |28 || 27 | 26 | 25 | 24 || 23 | 22 | 21 | 20 || 19 | 18 | 17 | 16 |
--------------------------------------------------------------------------------
| Name Space ID ||  0 |  0 |  0 |  0 ||  0 |  0 |  0 |  0 ||  0 |  0 |  0 |  0 |
________________________________________________________________________________

Bits: [31:28] - Name Space ID
                0000 - Standard
                0001 - ARM
                1000 - Custom/OEM
                All other values are reserved.

Bits: [27:16] - Reserved.
________________________________________________________________________________
|15 |14 |13 |12 || 11 | 10 |  9 |  8 ||  7 |  6 |  5 |  4 ||  3 |  2 |  1 |  0 |
--------------------------------------------------------------------------------
| 0 | 0 | 0 | 0 ||  0 |  0 |  0 |  0 ||                 Object ID              |
________________________________________________________________________________

Bits: [15:8] - Are reserved and must be zero.

Bits: [7:0] - Object ID

Object ID's in the Standard Namespace:
  0 - Configuration Manager Revision
  1 - ACPI Table List
  2 - SMBIOS Table List

Object ID's in the ARM Namespace:
   0 - Reserved
   1 - Boot Architecture Info
   2 - CPU Info
   3 - Power Management Profile Info
   4 - GICC Info
   5 - GICD Info
   6 - GIC MSI Frame Info
   7 - GIC Redistributor Info
   8 - GIC ITS Info
   9 - Serial Console Port Info
  10 - Serial Debug Port Info
  12 - Generic Timer Info
  13 - Platform GT Block Info
  14 - Platform Generic Watchdog
  15 - PCI Configuration Space Info
  16 - Hypervisor Vendor Id
*/
typedef UINT32  CM_OBJECT_ID;

/** The EOBJECT_NAMESPACE_ID enum describes the defined namespaces
    for the Confguration Manager Objects.
*/
typedef enum ObjectNameSpaceID {
  EObjNameSpaceStandard,      ///< Standard Objects Namespace
  EObjNameSpaceArm,           ///< ARM Objects Namespace
  EObjNameSpaceOem = 0x8,     ///< OEM Objects Namespace
  EObjNameSpaceMax
} EOBJECT_NAMESPACE_ID;

/** The CM_OBJ_DESCRIPTOR structure describes the Configuration
    Manager Object descriptor. The Configuration Manager Protocol
    interface uses this descriptor to return the Configuration
    Manager Objects.
*/
typedef struct CmObjDescriptor {
  /// Size of the Object or Object List
  /// described by this descriptor.
  UINT32   Size;

  /// Pointer to the Object or Object List
  /// described by this descriptor.
  UINTN  * Data;
} CM_OBJ_DESCRIPTOR;

#pragma pack()

/** This macro returns the namespace ID from the CmObjectID.

  @param [in] CmObjectId  The Configuration Manager Object ID.

  @retval Returns the Namespace ID corresponding to the CmObjectID.
*/
#define GET_CM_NAMESPACE_ID(CmObjectId) (((CmObjectId) >> 28) & 0xF)

/** This macro returns the Object ID from the CmObjectID.

  @param [in] CmObjectId  The Configuration Manager Object ID.

  @retval Returns the Object ID corresponding to the CmObjectID.
*/
#define GET_CM_OBJECT_ID(CmObjectId)    ((CmObjectId) & 0xFF)

/** This macro returns a Configuration Manager Object ID
    from the NameSpace ID and the ObjectID.

  @param [in] NameSpaceId The namespace ID for the Object.
  @param [in] ObjectId    The Object ID.

  @retval Returns the Configuration Manager Object ID.
*/
#define CREATE_CM_OBJECT_ID(NameSpaceId, ObjectId) \
          ((((NameSpaceId) & 0xF) << 28) | ((ObjectId) & 0xF))

/** This macro returns a Configuration Manager Object ID
    in the Standard Object Namespace.

  @param [in] ObjectId    The Object ID.

  @retval Returns a Standard Configuration Manager Object ID.
*/
#define CREATE_CM_STD_OBJECT_ID(ObjectId) \
          (CREATE_CM_OBJECT_ID (EObjNameSpaceStandard, ObjectId))

/** This macro returns a Configuration Manager Object ID
    in the ARM Object Namespace.

  @param [in] ObjectId    The Object ID.

  @retval Returns an ARM Configuration Manager Object ID.
*/
#define CREATE_CM_ARM_OBJECT_ID(ObjectId) \
          (CREATE_CM_OBJECT_ID (EObjNameSpaceArm, ObjectId))

/** This macro returns a Configuration Manager Object ID
    in the OEM Object Namespace.

  @param [in] ObjectId    The Object ID.

  @retval Returns an OEM Configuration Manager Object ID.
*/
#define CREATE_CM_OEM_OBJECT_ID(ObjectId) \
          (CREATE_CM_OBJECT_ID (EObjNameSpaceOem, ObjectId))

#endif // CONFIGURATION_MANAGER_OBJECT_H_
