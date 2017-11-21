/** @file

  Copyright (c) 2017, ARM Limited. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

  System Control and Management Interface V1.0
    http://infocenter.arm.com/help/topic/com.arm.doc.den0056a/
    DEN0056A_System_Control_and_Management_Interface.pdf
**/

#ifndef ARM_SCMI_PERFORMANCE_PROTOCOL_PRIVATE_H_
#define ARM_SCMI_PERFORMANCE_PROTOCOL_PRIVATE_H_

#include <Drivers/ArmScmiPerformanceProtocol.h>

// Number of performance levels returned by a call to the SCP, Lvls Bits[11:0]
#define NUM_PERF_LEVELS_MASK          0x0FFF
#define NUM_PERF_LEVELS(Lvls) (Lvls & NUM_PERF_LEVELS_MASK)

// Number of performance levels remaining after a call to the SCP, Lvls Bits[31:16]
#define NUM_REMAIN_PERF_LEVELS_SHIFT  16
#define NUM_REMAIN_PERF_LEVELS(Lvls) (Lvls >> NUM_REMAIN_PERF_LEVELS_SHIFT)

/** Return values for SCMI_MESSAGE_ID_PERFORMANCE_DESCRIBE_LEVELS command.
  SCMI Spec § 4.5.2.5
**/
typedef struct {
  UINT32 NumLevels;
  SCMI_PERFORMANCE_LEVEL PerfLevel[]; // Offset to array of performance levels
} PERF_DESCRIBE_LEVELS;

#endif /* ARM_SCMI_PERFORMANCE_PROTOCOL_PRIVATE_H_ */
