/** @file  ArmMaliDp.c

  ARM Mali DP 500/550/650 display controller driver

  Copyright (c) 2017, ARM Ltd. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/LcdPlatformLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Drivers/ArmMaliDp.h>
#include <Drivers/LcdHw.h>

// CORE_ID of the MALI DP
STATIC UINT32 gDpDeviceId;

/** Disable the graphics layer
  *
  * This is done by clearing the EN bit
  * of the LG_CONTROL register.
**/
STATIC
VOID
LayerGraphicsDisable (VOID)
{
  MmioAnd32 (DP_BASE + DP_DE_LG_CONTROL, ~DP_DE_LG_ENABLE);
}

/** Enable the graphics layer
  *
  * This is done by setting the EN bit
  * of the LG_CONTROL register.
**/
STATIC
VOID
LayerGraphicsEnable (VOID)
{
  MmioOr32 (DP_BASE + DP_DE_LG_CONTROL, DP_DE_LG_ENABLE);
}

/** Set the frame address of the graphics layer.
  *
  * @param  FrameBaseAddress        Address of the data buffer
  *                                 to be used as a frame buffer.
**/
STATIC
VOID
LayerGraphicsSetFrame (
  IN CONST EFI_PHYSICAL_ADDRESS FrameBaseAddress
  )
{
  // Disable the graphics layer.
  LayerGraphicsDisable ();

  /* Set up memory address of the data buffer for graphics layer.
   * write lower bits of the address.
   */
  MmioWrite32 (
    DP_BASE + DP_DE_LG_PTR_LOW,
    DP_DE_LG_PTR_LOW_MASK & FrameBaseAddress
    );

  // Write higher bits of the address.
  MmioWrite32 (
    DP_BASE + DP_DE_LG_PTR_HIGH,
    (UINT32)(FrameBaseAddress >> DP_DE_LG_PTR_HIGH_SHIFT)
    );

  // Enable the graphics layer.
  LayerGraphicsEnable ();
}

/** Configures various graphics layer characteristics.
  *
  * @param  HRes                 Horizontal resolution of the graphics layer.
  * @param  VRes                 Vertical resolution of the graphics layer.
**/
STATIC
VOID
LayerGraphicsConfig (
  IN CONST EFI_GRAPHICS_PIXEL_FORMAT UefiGfxPixelFormat,
  IN CONST UINT32 HRes,
  IN CONST UINT32 VRes
  )
{
  UINT32 PixelFormat;

  // Disable the graphics layer before configuring any settings.
  LayerGraphicsDisable ();

  // Setup graphics layer size.
  MmioWrite32 (DP_BASE + DP_DE_LG_IN_SIZE, FRAME_IN_SIZE (HRes, VRes));

  // Setup graphics layer composition size.
  MmioWrite32 (DP_BASE + DP_DE_LG_CMP_SIZE, FRAME_CMP_SIZE (HRes, VRes));

  // Setup memory stride (total visible pixels on a line * 4 ).
  MmioWrite32 (DP_BASE + DP_DE_LG_H_STRIDE, (HRes * sizeof (UINT32)));

  /* Set the format.
   *
   * In PixelBlueGreenRedReserved8BitPerColor format, byte 0 represents blue,
   * byte 1 represents green, and byte 2 represents red, and byte 3 is reserved
   * which is equivalent to XRGB format of the DP500/DP550/DP650. Whereas
   * PixelRedGreenBlueReserved8BitPerColor is equivalent to XBGR of the
   * DP500/DP550/DP650.
   */
  if (UefiGfxPixelFormat == PixelBlueGreenRedReserved8BitPerColor) {
    PixelFormat = (gDpDeviceId == MALIDP_500) ? DP_PIXEL_FORMAT_DP500_XRGB_8888
                     : DP_PIXEL_FORMAT_XRGB_8888;
  } else {
    PixelFormat = (gDpDeviceId == MALIDP_500) ? DP_PIXEL_FORMAT_DP500_XBGR_8888
                     : DP_PIXEL_FORMAT_XBGR_8888;
  }

  MmioWrite32 (DP_BASE + DP_DE_LG_FORMAT, PixelFormat);

  // Enable graphics layer.
  LayerGraphicsEnable ();
}

/** Configure timing information of the display.
  * @param IN Horizontal           Pointer to horizontal timing parameters.
  *                                (Resolution, Sync, Back porch, Front porch)
  * @param IN Vertical             Pointer to vertical timing parameters.
  *                                (Resolution, Sync, Back porch, Front porch)
**/
STATIC
VOID
SetDisplayEngineTiming (
  IN CONST SCAN_TIMINGS * CONST Horizontal,
  IN CONST SCAN_TIMINGS * CONST Vertical
  )
{
  UINTN RegHIntervals;
  UINTN RegVIntervals;
  UINTN RegSyncControl;
  UINTN RegHVActiveSize;

  if (gDpDeviceId == MALIDP_500) {
    // MALI DP500 timing registers.
    RegHIntervals = DP_BASE + DP_DE_DP500_H_INTERVALS;
    RegVIntervals = DP_BASE + DP_DE_DP500_V_INTERVALS;
    RegSyncControl = DP_BASE + DP_DE_DP500_SYNC_CONTROL;
    RegHVActiveSize = DP_BASE + DP_DE_DP500_HV_ACTIVESIZE;
  } else {
    // MALI DP550/DP650 timing registers.
    RegHIntervals = DP_BASE + DP_DE_H_INTERVALS;
    RegVIntervals = DP_BASE + DP_DE_V_INTERVALS;
    RegSyncControl = DP_BASE + DP_DE_SYNC_CONTROL;
    RegHVActiveSize = DP_BASE + DP_DE_HV_ACTIVESIZE;
  }

  // Horizontal back porch and front porch.
  MmioWrite32 (
    RegHIntervals,
    H_INTERVALS (Horizontal->FrontPorch, Horizontal->BackPorch)
    );

  // Vertical back porch and front porch.
  MmioWrite32 (
    RegVIntervals,
    V_INTERVALS (Vertical->FrontPorch, Vertical->BackPorch)
    );

  // Sync control, Horizontal and Vertical sync.
  MmioWrite32 (
    RegSyncControl,
    SYNC_WIDTH (Horizontal->Sync, Vertical->Sync)
    );

  // Set up Horizontal and Vertical area size.
  MmioWrite32 (
    RegHVActiveSize,
    HV_ACTIVE (Horizontal->Resolution, Vertical->Resolution)
    );
}

/** Return CORE_ID of the ARM Mali DP.
  *
  * @retval 0                      No Mali DP found.
  * @retval 0x500                  Mali DP core id for DP500.
  * @retval 0x550                  Mali DP core id for DP550.
  * @retval 0x650                  Mali DP core id for DP650.
**/
STATIC
UINT32
ArmMaliDpGetCoreId (
  )
{
  UINT32 DpCoreId;

  /* First check for DP500 as register offset for DP550/DP650 CORE_ID
   * is beyond 3K/4K register space of the DP500.
   */
  DpCoreId = MmioRead32 (DP_BASE + DP_DE_DP500_CORE_ID);
  DpCoreId >>= DP_DE_DP500_CORE_ID_SHIFT;

  if (DpCoreId == MALIDP_500) {
    return DpCoreId;
  }

  // Check for DP550 or DP650.
  DpCoreId = MmioRead32 (DP_BASE + DP_DC_CORE_ID);
  DpCoreId >>= DP_DC_CORE_ID_SHIFT;

  if (DpCoreId == MALIDP_550
    || DpCoreId == MALIDP_650) {
    return DpCoreId;
  }

  return 0;
}

/** Check for presence of MALI.
  *
  * This function returns success if the platform implements
  * DP500/DP550/DP650 ARM Mali display processor.
  *
  * @retval EFI_SUCCESS            Platform implements DP500/DP550/DP650.
  * @retval EFI_NOT_FOUND          DP500/DP550/DP650 display processor not
  *                                found on the platform.
**/
EFI_STATUS
LcdIdentify (VOID)
{
  UINT32 DpCoreId;

  DEBUG ((DEBUG_WARN,
    "Probing ARM Mali DP500/DP550/DP650 at base address 0x%p\n",
    DP_BASE
    ));

#if (DP_BASE == 0)
#error "ARM Mali DP peripheral base address is invalid\n"
#endif

  DpCoreId = ArmMaliDpGetCoreId ();

  if (DpCoreId != 0) {
     DEBUG ((DEBUG_WARN, "Found ARM Mali DP %x\n", DpCoreId));
     return EFI_SUCCESS;
  }

  DEBUG ((DEBUG_WARN, "ARM Mali DP not found...\n"));

  return EFI_NOT_FOUND;
}

/** Initialize platform display.
  *
  * @param  FrameBaseAddress       Address of the frame buffer.
  *
  * @retval EFI_SUCCESS            Display initialization success.
  * @retval !(EFI_SUCCESS)         Display initialization failure.
**/
EFI_STATUS
LcdInitialize (
  IN CONST EFI_PHYSICAL_ADDRESS FrameBaseAddress
  )
{
  DEBUG ((DEBUG_WARN, "Frame buffer base address = %p\n", FrameBaseAddress));

  gDpDeviceId = ArmMaliDpGetCoreId ();
  if (gDpDeviceId == 0) {
    DEBUG ((DEBUG_ERROR, "ARM Mali DP initialization fail,"
       "no ARM Mali DP present\n"));
    return EFI_NOT_FOUND;
  }

  // We are using graphics layer of the Mali DP as a main frame buffer.
  LayerGraphicsSetFrame (FrameBaseAddress);

  return EFI_SUCCESS;
}

/** Set ARM Mali DP in cofiguration mode.
  *
  * The ARM Mali DP must be in the configuration mode for
  * configuration of the H_INTERVALS, V_INTERVALS, SYNC_CONTROL
  * and HV_ACTIVESIZE.
**/
STATIC
VOID
SetConfigurationMode (VOID)
{
 // Request configuration Mode.
 if (gDpDeviceId == MALIDP_500) {
   MmioOr32 (DP_BASE + DP_DE_DP500_CONTROL, DP_DE_DP500_CONTROL_CONFIG_REQ);
 } else {
   MmioOr32 (DP_BASE + DP_DC_CONTROL, DP_DC_CONTROL_CM_ACTIVE);
 }
}

/** Set ARM Mali DP in normal mode.
  *
  * Normal mode is the main operating mode of the display processor
  * in which display layer data is fetched from frame buffer and
  * displayed.
**/
STATIC
VOID
SetNormalMode (VOID)
{
 // Disable configuration Mode.
 if (gDpDeviceId == MALIDP_500) {
   MmioAnd32 (DP_BASE + DP_DE_DP500_CONTROL, ~DP_DE_DP500_CONTROL_CONFIG_REQ);
 } else {
   MmioAnd32 (DP_BASE + DP_DC_CONTROL, ~DP_DC_CONTROL_CM_ACTIVE );
 }
}

/** Set the global configuration valid flag.
  *
  * Any new configuration parameters written to the display engine are not
  * activated until the global configuration valid flag is set in the
  * CONFIG_VALID register.
**/
STATIC
VOID
SetConfigValid (VOID)
{
  if (gDpDeviceId == MALIDP_500) {
    MmioOr32 (DP_BASE + DP_DP500_CONFIG_VALID, DP_DC_CONFIG_VALID);
  } else {
    MmioOr32 (DP_BASE + DP_DC_CONFIG_VALID, DP_DC_CONFIG_VALID );
  }
}

/** Set requested mode of the display.
  *
  * @param  ModeNumber             Display mode number.
  * @retval EFI_SUCCESS            Display set mode success.
  * @retval EFI_DEVICE_ERROR       If mode not found/supported.
  * @retval EFI_DEVICE_ERROR       If mode does not support more
  *                                than 24 bytes per pixel format.
**/
EFI_STATUS
LcdSetMode (
  IN CONST UINT32  ModeNumber
  )
{
  EFI_STATUS          Status;
  CONST SCAN_TIMINGS  *Horizontal;
  CONST SCAN_TIMINGS  *Vertical;

  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  ModeInfo;

  // Get the display mode timings and other relevant information.
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

  // Get the pixel format information.
  Status = LcdPlatformQueryMode (ModeNumber, &ModeInfo);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  // Request configuration mode.
  SetConfigurationMode ();

  // Configure the graphics layer.
  LayerGraphicsConfig (
    ModeInfo.PixelFormat,
    Horizontal->Resolution,
    Vertical->Resolution
    );

  // Set the display engine timings.
  SetDisplayEngineTiming (Horizontal, Vertical);

  // After configuration, set Mali DP in normal mode.
  SetNormalMode ();

  /* Any parameters written to the display engine are not activated until
   * CONFIG_VALID is set.
   */
  SetConfigValid ();

  return EFI_SUCCESS;
}

/**
  * This function de-initializes the display.
**/
VOID
LcdShutdown (VOID)
{
  // Disable graphics layer.
  LayerGraphicsDisable ();
}