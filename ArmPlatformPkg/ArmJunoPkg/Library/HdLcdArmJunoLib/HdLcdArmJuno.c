/** @file
*
*  Copyright (c) 2013 - 2016, ARM Ltd. All rights reserved.
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

#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/LcdPlatformLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/Cpu.h>
#include <ArmPlatform.h>

typedef struct {
  UINT32                     Mode;
  UINT32                     HorizontalResolution;
  UINT32                     VerticalResolution;
  LCD_BPP                    Bpp;
  UINT32                     OscFreq;

  // These are used by HDLCD
  UINT32                     HSync;
  UINT32                     HBackPorch;
  UINT32                     HFrontPorch;
  UINT32                     VSync;
  UINT32                     VBackPorch;
  UINT32                     VFrontPorch;
} LCD_RESOLUTION;


STATIC CONST LCD_RESOLUTION mResolutions[] = {
  { // Mode 0 : VGA : 640 x 480 x 24 bpp
    VGA, VGA_H_RES_PIXELS, VGA_V_RES_PIXELS, LCD_BITS_PER_PIXEL_24, VGA_OSC_FREQUENCY,
    VGA_H_SYNC, VGA_H_BACK_PORCH, VGA_H_FRONT_PORCH,
    VGA_V_SYNC, VGA_V_BACK_PORCH, VGA_V_FRONT_PORCH
  },
  { // Mode 1 : SVGA : 800 x 600 x 24 bpp
    SVGA, SVGA_H_RES_PIXELS, SVGA_V_RES_PIXELS, LCD_BITS_PER_PIXEL_24, SVGA_OSC_FREQUENCY,
    SVGA_H_SYNC, SVGA_H_BACK_PORCH, SVGA_H_FRONT_PORCH,
    SVGA_V_SYNC, SVGA_V_BACK_PORCH, SVGA_V_FRONT_PORCH
  },
  { // Mode 2 : XGA : 1024 x 768 x 24 bpp
    XGA, XGA_H_RES_PIXELS, XGA_V_RES_PIXELS, LCD_BITS_PER_PIXEL_24, XGA_OSC_FREQUENCY,
    XGA_H_SYNC, XGA_H_BACK_PORCH, XGA_H_FRONT_PORCH,
    XGA_V_SYNC, XGA_V_BACK_PORCH, XGA_V_FRONT_PORCH
  },
  { // Mode 3 : SXGA : 1280 x 1024 x 24 bpp
    SXGA, SXGA_H_RES_PIXELS, SXGA_V_RES_PIXELS, LCD_BITS_PER_PIXEL_24, SXGA_OSC_FREQUENCY,
    SXGA_H_SYNC, SXGA_H_BACK_PORCH, SXGA_H_FRONT_PORCH,
    SXGA_V_SYNC, SXGA_V_BACK_PORCH, SXGA_V_FRONT_PORCH
  },
  { // Mode 4 : WSXGA+ : 1680 x 1050 x 24 bpp
    WSXGA, WSXGA_H_RES_PIXELS, WSXGA_V_RES_PIXELS, LCD_BITS_PER_PIXEL_24, WSXGA_OSC_FREQUENCY,
    WSXGA_H_SYNC, WSXGA_H_BACK_PORCH, WSXGA_H_FRONT_PORCH,
    WSXGA_V_SYNC, WSXGA_V_BACK_PORCH, WSXGA_V_FRONT_PORCH
  },
  { // Mode 5 : UXGA : 1600 x 1200 x 24 bpp
    UXGA, UXGA_H_RES_PIXELS, UXGA_V_RES_PIXELS, LCD_BITS_PER_PIXEL_24, (UXGA_OSC_FREQUENCY ),
    UXGA_H_SYNC, UXGA_H_BACK_PORCH, UXGA_H_FRONT_PORCH,
    UXGA_V_SYNC, UXGA_V_BACK_PORCH, UXGA_V_FRONT_PORCH
  },
  { // Mode 6 : HD : 1920 x 1080 x 24 bpp
    HD, HD_H_RES_PIXELS, HD_V_RES_PIXELS, LCD_BITS_PER_PIXEL_24, (HD_OSC_FREQUENCY),
    HD_H_SYNC, HD_H_BACK_PORCH, HD_H_FRONT_PORCH,
    HD_V_SYNC, HD_V_BACK_PORCH, HD_V_FRONT_PORCH
  }
};


EFI_STATUS
LcdPlatformInitializeDisplay (
  IN EFI_HANDLE   Handle
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
LcdPlatformGetVram (
  OUT EFI_PHYSICAL_ADDRESS*  VramBaseAddress,
  OUT UINTN*                 VramSize
  )
{
  EFI_STATUS              Status;
  EFI_CPU_ARCH_PROTOCOL  *Cpu;
  EFI_ALLOCATE_TYPE       AllocationType;

  // Set the VRAM size
  *VramSize = (UINTN)FixedPcdGet64(PcdLcdFrameBufferSize);

  *VramBaseAddress = (EFI_PHYSICAL_ADDRESS)FixedPcdGet64(PcdLcdFrameBufferBase);

  // Allocate the VRAM from the DRAM so that nobody else uses it.
  if (*VramBaseAddress == 0) {
    AllocationType = AllocateAnyPages;
  } else {
    AllocationType = AllocateAddress;
  }

  Status = gBS->AllocatePages (
                  AllocationType,
                  EfiRuntimeServicesData,
                  EFI_SIZE_TO_PAGES(((UINTN)FixedPcdGet64(PcdLcdFrameBufferSize))),
                  VramBaseAddress
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  // Ensure the Cpu architectural protocol is already installed
  Status = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **)&Cpu);
  ASSERT_EFI_ERROR(Status);
  if (EFI_ERROR(Status)) {
    gBS->FreePool (VramBaseAddress);
    return Status;
  }

  // Mark the VRAM as un-cached. The VRAM is inside the DRAM, which is cached.
  Status = Cpu->SetMemoryAttributes (Cpu, *VramBaseAddress, *VramSize, EFI_MEMORY_UC);
  ASSERT_EFI_ERROR(Status);
  if (EFI_ERROR(Status)) {
    gBS->FreePool (VramBaseAddress);
    return Status;
  }

  return EFI_SUCCESS;
}

UINT32
LcdPlatformGetMaxMode (
  VOID
  )
{
  //
  // The following line will report correctly the total number of graphics modes
  // that could be supported by the graphics driver:
  //
  return (sizeof(mResolutions) / sizeof(LCD_RESOLUTION));
}

EFI_STATUS
LcdPlatformSetMode (
  IN UINT32                         ModeNumber
  )
{
  if (ModeNumber >= LcdPlatformGetMaxMode ()) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Currently the HDLCD clock is configured by SCP for supporting 1920 x 1080
  // If any other resolution is selected then the HDLCD clock needs to be set
  // appropriately in this function.
  //

  return EFI_SUCCESS;
}

EFI_STATUS
LcdPlatformQueryMode (
  IN  UINT32                                ModeNumber,
  OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  *Info
  )
{
  if ((Info == NULL) || (ModeNumber >= LcdPlatformGetMaxMode ())) {
    return EFI_INVALID_PARAMETER;
  }

  Info->Version = 0;
  Info->HorizontalResolution = mResolutions[ModeNumber].HorizontalResolution;
  Info->VerticalResolution = mResolutions[ModeNumber].VerticalResolution;
  Info->PixelsPerScanLine = mResolutions[ModeNumber].HorizontalResolution;

  if (mResolutions[ModeNumber].Bpp == LCD_BITS_PER_PIXEL_24) {
    Info->PixelFormat = PixelBlueGreenRedReserved8BitPerColor;
  } else {
    // All other formats are not supported
    ASSERT (FALSE);
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
LcdPlatformGetTimings (
  IN  UINT32                              ModeNumber,
  OUT UINT32*                             HRes,
  OUT UINT32*                             HSync,
  OUT UINT32*                             HBackPorch,
  OUT UINT32*                             HFrontPorch,
  OUT UINT32*                             VRes,
  OUT UINT32*                             VSync,
  OUT UINT32*                             VBackPorch,
  OUT UINT32*                             VFrontPorch
  )
{
  if ((HRes == NULL) || (HSync == NULL) || (HBackPorch == NULL) ||
        (HFrontPorch == NULL) || (VRes == NULL) || (VSync == NULL) ||
        (VBackPorch == NULL) || (VFrontPorch == NULL) ||
        (ModeNumber >= LcdPlatformGetMaxMode ())) {
    return EFI_INVALID_PARAMETER;
  }

  *HRes           = mResolutions[ModeNumber].HorizontalResolution;
  *HSync          = mResolutions[ModeNumber].HSync;
  *HBackPorch     = mResolutions[ModeNumber].HBackPorch;
  *HFrontPorch    = mResolutions[ModeNumber].HFrontPorch;
  *VRes           = mResolutions[ModeNumber].VerticalResolution;
  *VSync          = mResolutions[ModeNumber].VSync;
  *VBackPorch     = mResolutions[ModeNumber].VBackPorch;
  *VFrontPorch    = mResolutions[ModeNumber].VFrontPorch;

  return EFI_SUCCESS;
}

EFI_STATUS
LcdPlatformGetBpp (
  IN  UINT32                              ModeNumber,
  OUT LCD_BPP  *                          Bpp
  )
{
  if ((Bpp == NULL) || (ModeNumber >= LcdPlatformGetMaxMode ())) {
    return EFI_INVALID_PARAMETER;
  }

  *Bpp = mResolutions[ModeNumber].Bpp;

  return EFI_SUCCESS;
}
