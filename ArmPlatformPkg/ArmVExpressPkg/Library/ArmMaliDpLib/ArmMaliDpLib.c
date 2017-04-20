/** @file ArmMaliDpLib.c
*
*  The file contains ARM Mali DP platform specific implementation.
*
*  Copyright (c) 2017, ARM Ltd. All rights reserved.
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
#include <PiDxe.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/LcdPlatformLib.h>
#include <ArmPlatform.h>

/** Check an address is within 40 bits.
  *
  * The ARM Mali DP frame buffer address size can not be wider than 40 bits
**/
#define  DP_VALID_BASE_ADDR(Address)  ((Address >> 40) == 0)

typedef struct {
  UINT32                      Mode;
  UINT32                      OscFreq;
  SCAN_TIMINGS                Horizontal;
  SCAN_TIMINGS                Vertical;
} DISPLAY_MODE;

/** The display modes implemented by this driver.
  *
  * On Models, the OSC frequencies (listed for each mode below) are not used.
  * However these frequencies are useful on hardware plaforms where related
  * clock (or PLL) settings are based on these pixel clocks.
  *
  * Since the clock settings are defined externally, the driver must
  * communicate pixel clock frequencies to relevant modules
  * responsible for setting clocks. e.g. SCP.
**/
STATIC CONST DISPLAY_MODE mDisplayModes[] = {
  {
    // Mode 0 : VGA : 640 x 480 x 24 bpp.
    VGA,
    VGA_OSC_FREQUENCY,
    {VGA_H_RES_PIXELS, VGA_H_SYNC, VGA_H_BACK_PORCH, VGA_H_FRONT_PORCH},
    {VGA_V_RES_PIXELS, VGA_V_SYNC, VGA_V_BACK_PORCH, VGA_V_FRONT_PORCH}
  },
  {
    // Mode 1 : WVGA : 800 x 480 x 24 bpp.
    WVGA,
    WVGA_OSC_FREQUENCY,
    {WVGA_H_RES_PIXELS, WVGA_H_SYNC, WVGA_H_BACK_PORCH, WVGA_H_FRONT_PORCH},
    {WVGA_V_RES_PIXELS, WVGA_V_SYNC, WVGA_V_BACK_PORCH, WVGA_V_FRONT_PORCH}
  },
  {
    // Mode 2 : SVGA : 800 x 600 x 24 bpp.
    SVGA,
    SVGA_OSC_FREQUENCY,
    {SVGA_H_RES_PIXELS, SVGA_H_SYNC, SVGA_H_BACK_PORCH, SVGA_H_FRONT_PORCH},
    {SVGA_V_RES_PIXELS, SVGA_V_SYNC, SVGA_V_BACK_PORCH, SVGA_V_FRONT_PORCH}
  },
  {
    // Mode 3 : QHD : 960 x 540 x 24 bpp.
    QHD,
    QHD_OSC_FREQUENCY,
    {QHD_H_RES_PIXELS, QHD_H_SYNC, QHD_H_BACK_PORCH, QHD_H_FRONT_PORCH},
    {QHD_V_RES_PIXELS, QHD_V_SYNC, QHD_V_BACK_PORCH, QHD_V_FRONT_PORCH}
  },
  {
    // Mode 4 : WSVGA : 1024 x 600 x 24 bpp.
    WSVGA,
    WSVGA_OSC_FREQUENCY,
    {WSVGA_H_RES_PIXELS, WSVGA_H_SYNC, WSVGA_H_BACK_PORCH, WSVGA_H_FRONT_PORCH},
    {WSVGA_V_RES_PIXELS, WSVGA_V_SYNC, WSVGA_V_BACK_PORCH, WSVGA_V_FRONT_PORCH}
  },
  {
    // Mode 5 : XGA : 1024 x 768 x 24 bpp.
    XGA,
    XGA_OSC_FREQUENCY,
    {XGA_H_RES_PIXELS, XGA_H_SYNC, XGA_H_BACK_PORCH, XGA_H_FRONT_PORCH},
    {XGA_V_RES_PIXELS, XGA_V_SYNC, XGA_V_BACK_PORCH, XGA_V_FRONT_PORCH}
  },
  {
    // Mode 6 : HD : 1280 x 720 x 24 bpp.
    HD720,
    HD720_OSC_FREQUENCY,
    {HD720_H_RES_PIXELS, HD720_H_SYNC, HD720_H_BACK_PORCH, HD720_H_FRONT_PORCH},
    {HD720_V_RES_PIXELS, HD720_V_SYNC, HD720_V_BACK_PORCH, HD720_V_FRONT_PORCH}
  },
  {
    // Mode 7 : WXGA : 1280 x 800 x 24 bpp.
    WXGA,
    WXGA_OSC_FREQUENCY,
    {WXGA_H_RES_PIXELS, WXGA_H_SYNC, WXGA_H_BACK_PORCH, WXGA_H_FRONT_PORCH},
    {WXGA_V_RES_PIXELS, WXGA_V_SYNC, WXGA_V_BACK_PORCH, WXGA_V_FRONT_PORCH}
  },
  { // Mode 8 : SXGA : 1280 x 1024 x 24 bpp.
    SXGA,
    SXGA_OSC_FREQUENCY,
    {SXGA_H_RES_PIXELS, SXGA_H_SYNC, SXGA_H_BACK_PORCH, SXGA_H_FRONT_PORCH},
    {SXGA_V_RES_PIXELS, SXGA_V_SYNC, SXGA_V_BACK_PORCH, SXGA_V_FRONT_PORCH}
  },
  { // Mode 9 : WSXGA+ : 1680 x 1050 x 24 bpp.
    WSXGA,
    WSXGA_OSC_FREQUENCY,
    {WSXGA_H_RES_PIXELS, WSXGA_H_SYNC, WSXGA_H_BACK_PORCH, WSXGA_H_FRONT_PORCH},
    {WSXGA_V_RES_PIXELS,WSXGA_V_SYNC, WSXGA_V_BACK_PORCH, WSXGA_V_FRONT_PORCH}
  },
  {
    // Mode 10 : HD : 1920 x 1080 x 24 bpp.
    HD,
    HD_OSC_FREQUENCY,
    {HD_H_RES_PIXELS, HD_H_SYNC, HD_H_BACK_PORCH, HD_H_FRONT_PORCH},
    {HD_V_RES_PIXELS, HD_V_SYNC, HD_V_BACK_PORCH, HD_V_FRONT_PORCH}
  }
};

/** If PcdArmMaliDpMaxMode is 0, platform supports full range of modes
  * else platform supports modes from 0 to PcdArmMaliDpMaxMode - 1
**/
STATIC CONST UINT32 mMaxMode = ((FixedPcdGet32 (PcdArmMaliDpMaxMode) != 0)
                                   ? FixedPcdGet32 (PcdArmMaliDpMaxMode)
                                   : sizeof (mDisplayModes) / sizeof (DISPLAY_MODE));


/** Platform related initialization function.
  *
  * @retval EFI_SUCCESS            Platform initialization success.
  * @retval EFI_UNSUPPORTED        PcdGopPixelFormat must be
  *                                PixelRedGreenBlueReserved8BitPerColor OR
  *                                PixelBlueGreenRedReserved8BitPerColor
  *                                any other format is not supported.
**/
EFI_STATUS
LcdPlatformInitializeDisplay (
  IN CONST EFI_HANDLE   Handle
  )
{
  EFI_GRAPHICS_PIXEL_FORMAT PixelFormat;

  (VOID)Handle;

  // PixelBitMask and PixelBltOnly pixel formats are not supported
  PixelFormat = FixedPcdGet32 (PcdGopPixelFormat);
  if (PixelFormat != PixelRedGreenBlueReserved8BitPerColor
    && PixelFormat != PixelBlueGreenRedReserved8BitPerColor) {

    ASSERT (PixelFormat == PixelRedGreenBlueReserved8BitPerColor
      ||  PixelFormat == PixelBlueGreenRedReserved8BitPerColor);
   return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

/** Reserve VRAM memory in DRAM for the frame buffer
  *
  * (unless it is reserved already).
  *
  * The allocated address can be used to set the frame buffer as a base buffer
  * address for any layer of the ARM Mali DP.
  *
  * @param OUT VramBaseAddress      A pointer to the frame buffer address.
  * @param OUT VramSize             A pointer to the size of the frame
  *                                 buffer in bytes
  *
  * @retval EFI_SUCCESS             Frame buffer memory allocation success.
  * @retval EFI_INVALID_PARAMETER   VramBaseAddress or VramSize are NULL.
  * @retval EFI_UNSUPPORTED         Allocated address wider than 40 bits
  * @retval !EFI_SUCCESS            Other errors.
**/
EFI_STATUS
LcdPlatformGetVram (
  OUT EFI_PHYSICAL_ADDRESS * CONST VramBaseAddress,
  OUT UINTN                * CONST VramSize
  )
{
  EFI_STATUS      Status;

  // Check VramBaseAddress and VramSize are not NULL.
  if (VramBaseAddress == NULL || VramSize == NULL) {
    ASSERT (VramBaseAddress != NULL);
    ASSERT (VramSize != NULL);
    return EFI_INVALID_PARAMETER;
  }

  // Set the VRAM size.
  *VramSize = (UINTN)FixedPcdGet32 (PcdArmLcdDdrFrameBufferSize);

  // Check if memory is already reserved for the frame buffer.
#if (FixedPcdGet64 (PcdArmLcdDdrFrameBufferBase) != 0)

#if (!DP_VALID_BASE_ADDR (FixedPcdGet64 (PcdArmLcdDdrFrameBufferBase)))
#error ARM Mali DP frame buffer base address cannot be wider than 40 bits.
#else

  *VramBaseAddress =
    (EFI_PHYSICAL_ADDRESS)FixedPcdGet64 (PcdArmLcdDdrFrameBufferBase);

  Status = EFI_SUCCESS;
#endif

#else
  // If not already reserved, attempt to allocate the VRAM from the DRAM.
  Status = gBS->AllocatePages (
                  AllocateAnyPages,
                  EfiBootServicesData,
                  EFI_SIZE_TO_PAGES (*VramSize),
                  VramBaseAddress
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ArmMaliDpLib: Failed to allocate frame buffer.\n"));
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  // ARM Mali DP frame buffer base address can not be wider than 40 bits.
  if (!DP_VALID_BASE_ADDR (*VramBaseAddress)) {
    gBS->FreePages (*VramBaseAddress, EFI_SIZE_TO_PAGES (*VramSize));
    ASSERT (DP_VALID_BASE_ADDR (*VramBaseAddress));
    return EFI_UNSUPPORTED;
  }

  /* Mark the VRAM as write-combining. The VRAM is inside the DRAM, which is cacheable.
   * For ARM/AArch64 EFI_MEMORY_WC memory is actually uncached.
   */
  Status = gDS->SetMemorySpaceAttributes (
                  *VramBaseAddress,
                  *VramSize,
                  EFI_MEMORY_WC
                  );

  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    gBS->FreePages (*VramBaseAddress, EFI_SIZE_TO_PAGES (*VramSize));
  }

#endif
  return Status;
}

/** Return total number of modes supported.
  *
  * Note: Valid mode numbers are 0 to MaxMode - 1
  * See Section 12.9 of the UEFI Specification 2.7
  *
  * @retval UINT32             Mode Number.
**/
UINT32
LcdPlatformGetMaxMode (VOID)
{
  return  mMaxMode;
}

/** Set the requested display mode.
  *
  * @param IN ModeNumber             Mode Number.
  *
  * @retval EFI_SUCCESS              Set mode success.
  * @retval EFI_INVALID_PARAMETER    Requested mode not found.
**/
EFI_STATUS
LcdPlatformSetMode (
  IN CONST UINT32 ModeNumber
  )
{

  if (ModeNumber >= mMaxMode) {
    ASSERT (ModeNumber < mMaxMode);
    return EFI_INVALID_PARAMETER;
  }
  /* On models, platform specific clock/mux settings are not required
   * Display controller specific settings for Mali DP are done in LcdSetMode.
   */
  return EFI_SUCCESS;
}

/** Return information for the requested mode number.
  *
  * @param IN ModeNumber            Mode Number.
  * @param OUT Info                 Pointer for returned mode information
  *                                 (on success).
  *
  * @retval EFI_SUCCESS             Requested mode found.
  * @retval EFI_INVALID_PARAMETER   Info is NULL.
  * @retval EFI_INVALID_PARAMETER   Requested mode not found.
**/
EFI_STATUS
LcdPlatformQueryMode (
  IN  CONST UINT32                                 ModeNumber,
  OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION * CONST Info
  )
{
  if (ModeNumber >= mMaxMode || Info == NULL) {
    ASSERT (ModeNumber < mMaxMode);
    ASSERT (Info != NULL);
    return EFI_INVALID_PARAMETER;
  }

  Info->Version = 0;
  Info->HorizontalResolution = mDisplayModes[ModeNumber].Horizontal.Resolution;
  Info->VerticalResolution = mDisplayModes[ModeNumber].Vertical.Resolution;
  Info->PixelsPerScanLine = mDisplayModes[ModeNumber].Horizontal.Resolution;

  Info->PixelFormat = FixedPcdGet32 (PcdGopPixelFormat);

  return EFI_SUCCESS;
}

/** Returns the display timing information for the requested mode number.
  *
  * @param IN  ModeNumber           Mode Number.
  * @param OUT Horizontal           Pointer to horizontal timing parameters.
  *                                 (Resolution, Sync, Back porch, Front porch)
  * @param OUT Vertical             Pointer to vertical timing parameters.
  *                                 (Resolution, Sync, Back porch, Front porch)
  *
  * @retval EFI_SUCCESS             Requested mode found.
  * @retval EFI_INVALID_PARAMETER   Requested mode not found.
  * @retval EFI_INVALID_PARAMETER   One of the OUT parameters is NULL.
**/
EFI_STATUS
LcdPlatformGetTimings (
  IN  UINT32         ModeNumber,
  OUT CONST SCAN_TIMINGS ** Horizontal,
  OUT CONST SCAN_TIMINGS ** Vertical
  )
{
  if (ModeNumber >= mMaxMode || Horizontal == NULL || Vertical == NULL) {
    ASSERT (ModeNumber < mMaxMode);
    // One of the pointers is NULL
    ASSERT (Horizontal != NULL);
    ASSERT (Vertical != NULL);
    return EFI_INVALID_PARAMETER;
  }

  *Horizontal = &mDisplayModes[ModeNumber].Horizontal;
  *Vertical = &mDisplayModes[ModeNumber].Vertical;

  return EFI_SUCCESS;
}

/** Return bytes per pixel information for a mode number.
  *
  * @param IN  ModeNumber           Mode Number.
  * @param OUT Bpp                  Pointer to value Bytes Per Pixel.
  *
  * @retval EFI_SUCCESS             The requested mode is found.
  * @retval EFI_INVALID_PARAMETER   Requested mode not found.
  * @retval EFI_INVALID_PARAMETER   Bpp is NULL.
**/
EFI_STATUS
LcdPlatformGetBpp (
  IN  CONST UINT32    ModeNumber,
  OUT LCD_BPP * CONST Bpp
  )
{
  if (ModeNumber >= mMaxMode || Bpp == NULL) {
    // Check valid ModeNumber and Bpp.
    ASSERT (ModeNumber < mMaxMode);
    ASSERT (Bpp != NULL);
    return EFI_INVALID_PARAMETER;
  }

  *Bpp = LCD_BITS_PER_PIXEL_24;

  return EFI_SUCCESS;
}
