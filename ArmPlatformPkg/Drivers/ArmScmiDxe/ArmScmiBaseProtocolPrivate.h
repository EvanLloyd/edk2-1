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

#ifndef ARM_SCMI_BASE_PROTOCOL_PRIVATE_H_
#define ARM_SCMI_BASE_PROTOCOL_PRIVATE_H_

// Return values of BASE_DISCOVER_LIST_PROTOCOLS command.
typedef struct {
  UINT32 NumProtocols;
  // Array of four protocols in each element
  // Total elements = 1 + (NumProtocols-1)/4
  UINT8 Protocols[];
} BASE_DISCOVER_LIST;

#endif /* ARM_SCMI_BASE_PROTOCOL_PRIVATE_H_ */
