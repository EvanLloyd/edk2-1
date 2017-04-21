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

#ifndef ARM_NAMESPACE_OBJECTS_H_
#define ARM_NAMESPACE_OBJECTS_H_
#pragma pack(1)

/** The EARM_OBJECT_ID enum describes the Object IDs
    in the ARM Namespace
*/
typedef enum ArmObjectID {
  EArmObjReserved,                    ///<  0 - Reserved.
  EArmObjBootArchInfo,                ///<  1 - Boot Architecture Info
  EArmObjCpuInfo,                     ///<  2 - CPU Info
  EArmObjPowerManagementProfileInfo,  ///<  3 - Power Management Profile Info
  EArmObjGicCInfo,                    ///<  4 - GIC CPU Interface Info
  EArmObjGicDInfo,                    ///<  5 - GIC Distributor Info
  EArmObjGicMsiFrameInfo,             ///<  6 - GIC MSI Frame Info
  EArmObjGicRedistributorInfo,        ///<  7 - GIC Redistributor Info
  EArmObjGicItsInfo,                  ///<  8 - GIC ITS Info
  EArmObjSerialConsolePortInfo,       ///<  9 - Serial Console Port Info
  EArmObjSerialDebugPortInfo,         ///< 10 - Serial Debug Port Info
  EArmObjGenericTimerInfo,            ///< 11 - Generic Timer Info
  EArmObjPlatformGTBlockInfo,         ///< 12 - Platform GT Block Info
  EArmObjPlatformGenericWatchdogInfo, ///< 13 - Platform Generic Watchdog
  EArmObjPciConfigSpaceInfo,          ///< 14 - PCI Configuration Space Info
  EArmObjHypervisorVendorIdentity,    ///< 15 - Hypervisor Vendor Id
  EArmObjMax
} EARM_OBJECT_ID;

/** A structure that describes the
    ARM Boot Architecture flags.
*/
typedef struct CmArmBootArchInfo {
  /// This is the ARM_BOOT_ARCH flags field of the FADT Table
  /// described in the ACPI Table Specification.
  UINT32  BootArchFlags;
} CM_ARM_BOOT_ARCH_INFO;

typedef struct CmArmCpuInfo {
  // Reserved for use when SMBIOS tables are implemented.
} CM_ARM_CPU_INFO;

typedef struct CmArmCpuInfoList {
  UINT32             CpuCount;
  CM_ARM_CPU_INFO  * CpuInfo;
} CM_ARM_CPU_INFO_LIST;

/** A structure that describes the
    Power Management Profile Information for the Platform.
*/
typedef struct CmArmPowerManagementProfileInfo {
  /// This is the Preferred_PM_Profile field of the FADT Table
  /// described in the ACPI Specification
  UINT8  PowerManagementProfile;
} CM_ARM_POWER_MANAGEMENT_PROFILE_INFO;

/** A structure that describes the
    GIC CPU Interface for the Platform.
*/
typedef struct CmArmGicCInfo {
  /// The GIC CPU Interface number.
  UINT32  CPUInterfaceNumber;

  /// The ACPI Processor UID. This must match the
  /// _UID of the CPU Device object information described
  /// in the DSDT/SSDT for the CPU.
  UINT32  AcpiProcessorUid;

  /// The flags field as described by the GICC structure
  /// in the ACPI Specification.
  UINT32  Flags;

  /// The parking protocol version field as described by
  /// the GICC structure in the ACPI Specification.
  UINT32  ParkingProtocolVersion;

  /// The Performance Interrupt field as described by
  /// the GICC structure in the ACPI Specification.
  UINT32  PerformanceInterruptGsiv;

  /// The CPU Parked address field as described by
  /// the GICC structure in the ACPI Specification.
  UINT64  ParkedAddress;

  /// The base address for the GIC CPU Interface
  /// as described by the GICC structure in the
  /// ACPI Specification.
  UINT64  PhysicalBaseAddress;

  /// The base address for GICV interface
  /// as described by the GICC structure in the
  /// ACPI Specification.
  UINT64  GICV;

  /// The base address for GICH interface
  /// as described by the GICC structure in the
  /// ACPI Specification.
  UINT64  GICH;

  /// The GICV maintainence interrupt
  /// as described by the GICC structure in the
  /// ACPI Specification.
  UINT32  VGICMaintenanceInterrupt;

  /// The base address for GICR interface
  /// as described by the GICC structure in the
  /// ACPI Specification.
  UINT64  GICRBaseAddress;

  /// The MPIDR for the CPU
  /// as described by the GICC structure in the
  /// ACPI Specification.
  UINT64  MPIDR;

  /// The Processor Power Efficiency class
  /// as described by the GICC structure in the
  /// ACPI Specification.
  UINT8   ProcessorPowerEfficiencyClass;
} CM_ARM_GICC_INFO;

/** A structure that describes the
    GIC Distributor information for the Platform.
*/
typedef struct CmArmGicDInfo {
  /// The GIC Distributor ID.
  UINT32  GicId;

  /// The Physical Base address
  /// for the GIC Distributor.
  UINT64  PhysicalBaseAddress;

  /// The global system interrupt
  /// number where this GIC Distributor's
  /// interrupt inputs start.
  UINT32  SystemVectorBase;

  /// The GIC version as described
  /// by the GICD structure in the
  /// ACPI Specification.
  UINT8   GicVersion;
} CM_ARM_GICD_INFO;

/** A structure that describes the
    GIC MSI Frame information for the Platform.
*/
typedef struct CmArmGicMsiFrameInfo {
  /// The GIC MSI Frame ID
  UINT32  GicMsiFrameId;

  /// The Physical base address for the
  /// MSI Frame.
  UINT64  PhysicalBaseAddress;

  /// The GIC MSI Frame flags
  /// as described by the GIC MSI frame
  /// structure in the ACPI Specification.
  UINT32  Flags;

  /// SPI Count used by this frame.
  UINT16  SPICount;

  /// SPI Base used by this frame.
  UINT16  SPIBase;
} CM_ARM_GIC_MSI_FRAME_INFO;

/** A structure that describes the
    GIC Redistributor information for the Platform.
*/
typedef struct CmArmGicRedistInfo {
  /// The physical address of a page range
  /// containing all GIC Redistributors.
  UINT64  DiscoveryRangeBaseAddress;

  /// Length of the GIC Redistributor
  /// Discovery page range
  UINT32  DiscoveryRangeLength;
} CM_ARM_GIC_REDIST_INFO;

/** A structure that describes the
    GIC Interrupt Translation Service information for the Platform.
*/
typedef struct CmArmGicItsInfo {
  /// The GIC ITS ID.
  UINT32  GicItsId;

  /// The physical address for the
  /// Interrupt Translation Service
  UINT64  PhysicalBaseAddress;
} CM_ARM_GIC_ITS_INFO;

/** A structure that describes the
    Serial Port information for the Platform.
*/
typedef struct CmArmSerialPortInfo {
  /// The physical base address for the
  /// serial port.
  UINT64  BaseAddress;

  /// The serial port interrupt.
  UINT32  Interrupt;

  /// The serial port baud rate.
  UINT64  BaudRate;

  /// The serial port clock.
  UINT32  Clock;
} CM_ARM_SERIAL_PORT_INFO;

/** A structure that describes the
    Generic Timer information for the Platform.
*/
typedef struct CmArmGenericTimerInfo {
  /// The physical base address for the
  /// counter control frame.
  UINT64  CounterControlBaseAddress;

  /// The physical base address for the
  /// counter read frame.
  UINT64  CounterReadBaseAddress;

  /// The secure PL1 timer interrupt.
  UINT32  SecurePL1TimerGSIV;

  /// The secure PL1 timer flags.
  UINT32  SecurePL1TimerFlags;

  /// The non-secure PL1 timer interrupt.
  UINT32  NonSecurePL1TimerGSIV;

  /// The non-secure PL1 timer flags.
  UINT32  NonSecurePL1TimerFlags;

  /// The virtual timer interrupt.
  UINT32  VirtualTimerGSIV;

  /// The virtual timer flags.
  UINT32  VirtualTimerFlags;

  /// The non-secure PL2 timer interrupt.
  UINT32  NonSecurePL2TimerGSIV;

  /// The non-secure PL2 timer flags.
  UINT32  NonSecurePL2TimerFlags;
} CM_ARM_GENERIC_TIMER_INFO;

/** A structure that describes the
    Platform Generic Block Timer Frame information for the Platform.
*/
typedef struct CmArmGTBlockTimerFrameInfo {
  /// The Generic Timer frame number.
  UINT8   FrameNumber;

  /// The physical base address
  /// for the CntBase block.
  UINT64  PhysicalAddressCntBase;

  /// The physical base address
  /// for the CntEL0Base block.
  UINT64  PhysicalAddressCntEL0Base;

  /// The physical timer interrupt.
  UINT32  PhysicalTimerGSIV;

  /// The physical timer flags
  /// as described by the GT Block
  /// Timer frame Structure in the
  /// ACPI Specification.
  UINT32  PhysicalTimerFlags;

  /// The virtual timer interrupt.
  UINT32  VirtualTimerGSIV;

  /// The virtual timer flags
  /// as described by the GT Block
  /// Timer frame Structure in the
  /// ACPI Specification.
  UINT32  VirtualTimerFlags;

  /// The common timer flags
  /// as described by the GT Block
  /// Timer frame Structure in the
  /// ACPI Specification.
  UINT32  CommonFlags;
} CM_ARM_GTBLOCK_TIMER_FRAME_INFO;

/** A structure that describes the
    Platform Generic Block Timer information for the Platform.
*/
typedef struct CmArmGTBlockInfo {
  /// The physical base address for
  /// the GT Block Timer structure.
  UINT64                             GTBlockPhysicalAddress;

  /// The number of timer frames
  /// implemented in the GT Block.
  UINT32                             GTBlockTimerFrameCount;

  /// Pointer to the GT Block timer
  /// frame list.
  CM_ARM_GTBLOCK_TIMER_FRAME_INFO  * GTBlockTimerFrameList;
} CM_ARM_GTBLOCK_INFO;

/** A structure that describes the
    SBSA Generic Watchdog information for the Platform.
*/
typedef struct CmArmGenericWatchdogInfo {
  /// The physical base address of the
  /// SBSA Watchdog control frame.
  UINT64  ControlFrameAddress;

  /// The physical base address of the
  /// SBSA Watchdog refresh frame.
  UINT64  RefreshFrameAddress;

  /// The watchdog interrupt.
  UINT32  TimerGSIV;

  /// The flags for the watchdog
  /// as described by the SBSA watchdog
  /// structure in the ACPI specification.
  UINT32  Flags;
} CM_ARM_GENERIC_WATCHDOG_INFO;

/** A structure that describes the
    PCI Configuration Space information for the Platform.
*/
typedef struct CmArmPciConfigSpaceInfo {
  /// The physical base address for the PCI segment.
  UINT64  BaseAddress;

  /// The PCI segment group number.
  UINT16  PciSegmentGroupNumber;

  /// The start bus number.
  UINT8   StartBusNumber;

  /// The end bus number.
  UINT8   EndBusNumber;
} CM_ARM_PCI_CONFIG_SPACE_INFO;

/** A structure that describes the
    Hypervisor Vendor ID information for the Platform.
*/
typedef struct CmArmHypervisorVendorId {
  /// The hypervisor Vendor ID.
  UINT64  HypervisorVendorId;
} CM_ARM_HYPERVISOR_VENDOR_ID;

#pragma pack()

#endif // ARM_NAMESPACE_OBJECTS_H_
