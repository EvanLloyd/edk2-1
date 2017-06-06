/** @file  HdLcd.c

  Copyright (c) 2011-2017, ARM Ltd. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/LcdPlatformLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>

#include <Drivers/HdLcd.h>

#include "LcdGraphicsOutputDxe.h"

/**********************************************************************
 *
 *  This file contains all the bits of the Lcd that are
 *  platform independent.
 *
 **********************************************************************/

/** Initialize display.
  *
  * @param  VramBaseAddress        Address of the frame buffer.
  *
  * @retval EFI_SUCCESS            Display initialization success.
**/
EFI_STATUS
LcdInitialize (
  IN CONST EFI_PHYSICAL_ADDRESS   VramBaseAddress
  )
{
  // Disable the controller
  MmioWrite32 (HDLCD_REG_COMMAND, HDLCD_DISABLE);

  // Disable all interrupts
  MmioWrite32 (HDLCD_REG_INT_MASK, 0);

  // Define start of the VRAM. This never changes for any graphics mode
  MmioWrite32 (HDLCD_REG_FB_BASE, (UINT32)VramBaseAddress);

  // Setup various registers that never change
  MmioWrite32 (HDLCD_REG_BUS_OPTIONS,  (4 << 8) | HDLCD_BURST_8);

  MmioWrite32 (
    HDLCD_REG_POLARITIES,
    HDLCD_PXCLK_LOW
    | HDLCD_DATA_HIGH
    | HDLCD_DATEN_HIGH
    | HDLCD_HSYNC_LOW
    | HDLCD_VSYNC_HIGH
    );

  MmioWrite32 (
    HDLCD_REG_PIXEL_FORMAT,
    HDLCD_LITTLE_ENDIAN | HDLCD_4BYTES_PER_PIXEL
    );

  MmioWrite32 (HDLCD_REG_RED_SELECT,   (0 << 16 | 8 << 8 | 0));
  MmioWrite32 (HDLCD_REG_GREEN_SELECT, (0 << 16 | 8 << 8 | 8));
  MmioWrite32 (HDLCD_REG_BLUE_SELECT,  (0 << 16 | 8 << 8 | 16));

  return EFI_SUCCESS;
}

/** Set requested mode of the display.
  *
  * @param  ModeNumber             Display mode number.
  * @retval EFI_SUCCESS            Display set mode success.
  * @retval EFI_DEVICE_ERROR       If mode not found/supported.
**/
EFI_STATUS
LcdSetMode (
  IN CONST UINT32  ModeNumber
  )
{
  EFI_STATUS          Status;
  CONST SCAN_TIMINGS *Horizontal;
  CONST SCAN_TIMINGS *Vertical;
  UINT32              BytesPerPixel;
  LCD_BPP             LcdBpp;


  // Set the video mode timings and other relevant information
  Status = LcdPlatformGetTimings (
             ModeNumber,
             &Horizontal,
             &Vertical
             );
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
  ASSERT (Horizontal != NULL);
  ASSERT (Vertical != NULL);

  Status = LcdPlatformGetBpp (ModeNumber, &LcdBpp);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  BytesPerPixel = GetBytesPerPixel (LcdBpp);

  // Disable the controller
  MmioWrite32 (HDLCD_REG_COMMAND, HDLCD_DISABLE);

  // Update the frame buffer information with the new settings
  MmioWrite32 (
    HDLCD_REG_FB_LINE_LENGTH,
    Horizontal->Resolution * BytesPerPixel
    );

  MmioWrite32 (HDLCD_REG_FB_LINE_PITCH, Horizontal->Resolution * BytesPerPixel);

  MmioWrite32 (HDLCD_REG_FB_LINE_COUNT, Vertical->Resolution - 1);

  // Set the vertical timing information
  MmioWrite32 (HDLCD_REG_V_SYNC,        Vertical->Sync);
  MmioWrite32 (HDLCD_REG_V_BACK_PORCH,  Vertical->BackPorch);
  MmioWrite32 (HDLCD_REG_V_DATA,        Vertical->Resolution - 1);
  MmioWrite32 (HDLCD_REG_V_FRONT_PORCH, Vertical->FrontPorch);

  // Set the horizontal timing information
  MmioWrite32 (HDLCD_REG_H_SYNC,        Horizontal->Sync);
  MmioWrite32 (HDLCD_REG_H_BACK_PORCH,  Horizontal->BackPorch);
  MmioWrite32 (HDLCD_REG_H_DATA,        Horizontal->Resolution - 1);
  MmioWrite32 (HDLCD_REG_H_FRONT_PORCH, Horizontal->FrontPorch);

  // Enable the controller
  MmioWrite32 (HDLCD_REG_COMMAND, HDLCD_ENABLE);

  return EFI_SUCCESS;
}

/** De-initializes the display.
**/
VOID
LcdShutdown (
  VOID
  )
{
  // Disable the controller
  MmioWrite32 (HDLCD_REG_COMMAND, HDLCD_DISABLE);
}

/** Get the HDLCD Product Id (from the version register).
**/
STATIC
UINT32 GetHdlcdProductId(VOID)
{
  return ((MmioRead32 (HDLCD_REG_VERSION)) >> 16) ;
}

/** Check if an HDLCD is present.
**/
STATIC
BOOLEAN HdlcdPresent(VOID)
{
  return (GetHdlcdProductId() == HDLCD_PRODUCT_ID);
}

/** Check for presence of HDLCD.
  *
  * @retval EFI_SUCCESS            Platform implements HDLCD.
  * @retval EFI_NOT_FOUND          HDLCD display controller not
  *                                found.
**/
EFI_STATUS
LcdIdentify (
  VOID
  )
{
  if (HdlcdPresent()) {
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}
