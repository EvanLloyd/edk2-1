/** @file HdLcdArmVExpress.c

  Copyright (c) 2012-2017, ARM Ltd. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>

#include <Library/ArmPlatformSysConfigLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/LcdPlatformLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/EdidDiscovered.h>
#include <Protocol/EdidActive.h>

#include <ArmPlatform.h>

typedef struct {
  UINT32                     Mode;
  LCD_BPP                    Bpp;
  UINT32                     OscFreq;

  // These are used by HDLCD
  SCAN_TIMINGS               Horizontal;
  SCAN_TIMINGS               Vertical;
} DISPLAY_MODE;

/** The display modes supported by the platform.
**/
STATIC CONST DISPLAY_MODE mDisplayModes[] = {
  { // Mode 0 : VGA : 640 x 480 x 24 bpp
    VGA, LCD_BITS_PER_PIXEL_24,
    VGA_OSC_FREQUENCY,
    {VGA_H_RES_PIXELS, VGA_H_SYNC, VGA_H_BACK_PORCH, VGA_H_FRONT_PORCH},
    {VGA_V_RES_PIXELS, VGA_V_SYNC, VGA_V_BACK_PORCH, VGA_V_FRONT_PORCH}
  },
  { // Mode 1 : SVGA : 800 x 600 x 24 bpp
    SVGA, LCD_BITS_PER_PIXEL_24,
    SVGA_OSC_FREQUENCY,
    {SVGA_H_RES_PIXELS, SVGA_H_SYNC, SVGA_H_BACK_PORCH, SVGA_H_FRONT_PORCH},
    {SVGA_V_RES_PIXELS, SVGA_V_SYNC, SVGA_V_BACK_PORCH, SVGA_V_FRONT_PORCH}
  },
  { // Mode 2 : XGA : 1024 x 768 x 24 bpp
    XGA, LCD_BITS_PER_PIXEL_24,
    XGA_OSC_FREQUENCY,
    {XGA_H_RES_PIXELS, XGA_H_SYNC, XGA_H_BACK_PORCH, XGA_H_FRONT_PORCH},
    {XGA_V_RES_PIXELS, XGA_V_SYNC, XGA_V_BACK_PORCH, XGA_V_FRONT_PORCH}
  },
  { // Mode 3 : SXGA : 1280 x 1024 x 24 bpp
    SXGA, LCD_BITS_PER_PIXEL_24,
    (SXGA_OSC_FREQUENCY/2),
    {SXGA_H_RES_PIXELS, SXGA_H_SYNC, SXGA_H_BACK_PORCH, SXGA_H_FRONT_PORCH},
    {SXGA_V_RES_PIXELS, SXGA_V_SYNC, SXGA_V_BACK_PORCH, SXGA_V_FRONT_PORCH}
  },
  { // Mode 4 : UXGA : 1600 x 1200 x 24 bpp
    UXGA, LCD_BITS_PER_PIXEL_24,
    (UXGA_OSC_FREQUENCY/2),
    {UXGA_H_RES_PIXELS, UXGA_H_SYNC, UXGA_H_BACK_PORCH, UXGA_H_FRONT_PORCH},
    {UXGA_V_RES_PIXELS, UXGA_V_SYNC, UXGA_V_BACK_PORCH, UXGA_V_FRONT_PORCH}
  },
  { // Mode 5 : HD : 1920 x 1080 x 24 bpp
    HD, LCD_BITS_PER_PIXEL_24,
    (HD_OSC_FREQUENCY/2),
    {HD_H_RES_PIXELS, HD_H_SYNC, HD_H_BACK_PORCH, HD_H_FRONT_PORCH},
    {HD_V_RES_PIXELS, HD_V_SYNC, HD_V_BACK_PORCH, HD_V_FRONT_PORCH}
  }
};

EFI_EDID_DISCOVERED_PROTOCOL  mEdidDiscovered = {
  0,
  NULL
};

EFI_EDID_ACTIVE_PROTOCOL      mEdidActive = {
  0,
  NULL
};

/** HDLCD Platform specific initialization function.
  *
  * @retval EFI_SUCCESS            Plaform library initialization success.
  * @retval !(EFI_SUCCESS)         Other errors.
**/
EFI_STATUS
LcdPlatformInitializeDisplay (
  IN EFI_HANDLE   Handle
  )
{
  EFI_STATUS  Status;

  /* Set the FPGA multiplexer to select the video output from the
   * motherboard or the daughterboard */
  Status = ArmPlatformSysConfigSet (
             SYS_CFG_MUXFPGA,
             ARM_VE_DAUGHTERBOARD_1_SITE
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Install the EDID Protocols
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEfiEdidDiscoveredProtocolGuid,
                  &mEdidDiscovered,
                  &gEfiEdidActiveProtocolGuid,
                  &mEdidActive,
                  NULL
                  );

  return Status;
}

/** Reserve VRAM memory in DRAM for the frame buffer
  * (unless it is reserved already).
  *
  * The allocated address can be used to set the frame buffer.
  *
  * @param OUT VramBaseAddress      A pointer to the frame buffer address.
  * @param OUT VramSize             A pointer to the size of the frame
  *                                 buffer in bytes
  *
  * @retval EFI_SUCCESS             Frame buffer memory allocation success.
  * @retval EFI_INVALID_PARAMETER   VramBaseAddress or VramSize are NULL.
  * @retval !(EFI_SUCCESS)          Other errors.
**/
EFI_STATUS
LcdPlatformGetVram (
  OUT EFI_PHYSICAL_ADDRESS * CONST  VramBaseAddress,
  OUT UINTN * CONST                 VramSize
  )
{
  EFI_STATUS              Status;
  EFI_ALLOCATE_TYPE       AllocationType;

  // Check VramBaseAddress and VramSize are not NULL.
  if (VramBaseAddress == NULL || VramSize == NULL) {
    ASSERT (VramBaseAddress != NULL);
    ASSERT (VramSize != NULL);
    return EFI_INVALID_PARAMETER;
  }

  // Set the vram size
  *VramSize = LCD_VRAM_SIZE;

  *VramBaseAddress = (EFI_PHYSICAL_ADDRESS)LCD_VRAM_CORE_TILE_BASE;

  // Allocate the VRAM from the DRAM so that nobody else uses it.
  if (*VramBaseAddress == 0) {
    AllocationType = AllocateAnyPages;
  } else {
    AllocationType = AllocateAddress;
  }
  Status = gBS->AllocatePages (
                  AllocationType,
                  EfiBootServicesData,
                  EFI_SIZE_TO_PAGES (((UINTN)LCD_VRAM_SIZE)),
                  VramBaseAddress
                  );
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  /* Mark the VRAM as write-combining.
   * The VRAM is inside the DRAM, which is cacheable. */
  Status = gDS->SetMemorySpaceAttributes (
                  *VramBaseAddress,
                  *VramSize,
                  EFI_MEMORY_WC
                  );
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    gBS->FreePages (*VramBaseAddress, EFI_SIZE_TO_PAGES (*VramSize));
    return Status;
  }

  return EFI_SUCCESS;
}

/** Return total number of modes supported.
  *
  * Note: Valid mode numbers are 0 to MaxMode - 1
  * See Section 11.9 of the UEFI Specification 2.6 Errata A (Jan 2017)
  *
  * @retval UINT32             Mode Number.
**/
UINT32
LcdPlatformGetMaxMode(VOID)
{
  /* The following line will report correctly the total number of graphics modes
   * that could be supported by the graphics driver: */
  return (sizeof (mDisplayModes) / sizeof (DISPLAY_MODE));
}

/** Set the requested display mode.
  *
  * @param IN ModeNumber             Mode Number.
**/
EFI_STATUS
LcdPlatformSetMode (
  IN CONST UINT32                         ModeNumber
  )
{
  EFI_STATUS            Status;

  if (ModeNumber >= LcdPlatformGetMaxMode ()) {
    ASSERT (ModeNumber < LcdPlatformGetMaxMode ());
    return EFI_INVALID_PARAMETER;
  }

  // Set the video mode oscillator
  Status = ArmPlatformSysConfigSetDevice (
              SYS_CFG_OSC_SITE1,
              FixedPcdGet32 (PcdHdLcdVideoModeOscId),
              mDisplayModes[ModeNumber].OscFreq
              );
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  // Set the DVI into the new mode
  Status = ArmPlatformSysConfigSet (
              SYS_CFG_DVIMODE,
              mDisplayModes[ModeNumber].Mode
              );
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  // Set the multiplexer
  Status = ArmPlatformSysConfigSet (
             SYS_CFG_MUXFPGA,
             ARM_VE_DAUGHTERBOARD_1_SITE
             );
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  return Status;
}

/** Return information for the requested mode number.
  *
  * @param IN ModeNumber            Mode Number.
  * @param OUT Info                 Pointer for returned mode information
  *                                 (on success).
  *
  * @retval EFI_SUCCESS             Success if the requested mode is found.
  * @retval EFI_INVALID_PARAMETER   Requested mode not found.
  * @retval EFI_INVALID_PARAMETER   Info is NULL.
**/
EFI_STATUS
LcdPlatformQueryMode (
  IN CONST UINT32                                   ModeNumber,
  OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION * CONST  Info
  )
{
  if (ModeNumber >= LcdPlatformGetMaxMode () || Info == NULL) {
    ASSERT (ModeNumber < LcdPlatformGetMaxMode ());
    ASSERT (Info != NULL);
    return EFI_INVALID_PARAMETER;
  }

  Info->Version = 0;
  Info->HorizontalResolution = mDisplayModes[ModeNumber].Horizontal.Resolution;
  Info->VerticalResolution = mDisplayModes[ModeNumber].Vertical.Resolution;
  Info->PixelsPerScanLine = mDisplayModes[ModeNumber].Horizontal.Resolution;

  switch (mDisplayModes[ModeNumber].Bpp) {
    case LCD_BITS_PER_PIXEL_24:
      Info->PixelFormat                   = PixelRedGreenBlueReserved8BitPerColor;
      Info->PixelInformation.RedMask      = LCD_24BPP_RED_MASK;
      Info->PixelInformation.GreenMask    = LCD_24BPP_GREEN_MASK;
      Info->PixelInformation.BlueMask     = LCD_24BPP_BLUE_MASK;
      Info->PixelInformation.ReservedMask = LCD_24BPP_RESERVED_MASK;
      break;

    case LCD_BITS_PER_PIXEL_16_555:
    case LCD_BITS_PER_PIXEL_16_565:
    case LCD_BITS_PER_PIXEL_12_444:
    case LCD_BITS_PER_PIXEL_8:
    case LCD_BITS_PER_PIXEL_4:
    case LCD_BITS_PER_PIXEL_2:
    case LCD_BITS_PER_PIXEL_1:
    default:
      // These are not supported
      ASSERT (FALSE);
      break;
  }

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
  * @retval EFI_SUCCESS             Success if the requested mode is found.
  * @retval EFI_INVALID_PARAMETER   Requested mode not found.
**/
EFI_STATUS
LcdPlatformGetTimings (
  IN  CONST UINT32                  ModeNumber,
  OUT CONST SCAN_TIMINGS         ** Horizontal,
  OUT CONST SCAN_TIMINGS         ** Vertical
  )
{
  if (ModeNumber >= LcdPlatformGetMaxMode ()) {
    ASSERT (ModeNumber < LcdPlatformGetMaxMode ());
    return EFI_INVALID_PARAMETER;
  }

  if (Horizontal == NULL || Vertical == NULL) {
    ASSERT (Horizontal != NULL);
    ASSERT (Vertical != NULL);
    return EFI_INVALID_PARAMETER;
  }

  *Horizontal = &mDisplayModes[ModeNumber].Horizontal;
  *Vertical   = &mDisplayModes[ModeNumber].Vertical;

  return EFI_SUCCESS;
}

/** Return bits per pixel for a mode number.
  *
  * @param IN  ModeNumber           Mode Number.
  * @param OUT Bpp                  Pointer to value Bits Per Pixel.
  *
  * @retval EFI_SUCCESS             The requested mode is found.
  * @retval EFI_INVALID_PARAMETER   Requested mode not found.
  * @retval EFI_INVALID_PARAMETER   Bpp is NULL.
**/
EFI_STATUS
LcdPlatformGetBpp (
  IN CONST UINT32                        ModeNumber,
  OUT LCD_BPP * CONST                    Bpp
  )
{
  if (ModeNumber >= LcdPlatformGetMaxMode () || Bpp == NULL) {
    ASSERT (ModeNumber < LcdPlatformGetMaxMode ());
    ASSERT (Bpp != NULL);
    return EFI_INVALID_PARAMETER;
  }

  *Bpp = mDisplayModes[ModeNumber].Bpp;

  return EFI_SUCCESS;
}
