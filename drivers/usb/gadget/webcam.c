/*
 *    webcam.c -- USB webcam gadget driver
 *
 *    Copyright (C) 2009-2010
 *        Laurent Pinchart (laurent.pinchart@ideasonboard.com)
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/usb/video.h>

#include "f_uvc.h"

/*
 * Kbuild is not very cooperative with respect to linking separately
 * compiled library objects into one module.  So for now we won't use
 * separate compilation ... ensuring init/exit sections work to shrink
 * the runtime footprint, and giving us at least some parts of what
 * a "gcc --combine ... part1.c part2.c part3.c ... " build would.
 */
#include "composite.c"
#include "usbstring.c"
#include "config.c"
#include "epautoconf.c"

#include "uvc_queue.c"
#include "uvc_video.c"
#include "uvc_v4l2.c"
#include "f_uvc.c"

/* --------------------------------------------------------------------------
 * Device descriptor
 */

#define WEBCAM_VENDOR_ID            0x1d6b    /* Linux Foundation */
#define WEBCAM_PRODUCT_ID           0x0102    /* Webcam A/V gadget */
#define WEBCAM_DEVICE_BCD           0x0010    /* 0.10 */

static char webcam_vendor_label[]   = "Linux Foundation";
static char webcam_product_label[]  = "Webcam gadget";
static char webcam_config_label[]   = "Video";

/* string IDs are assigned dynamically */

#define STRING_MANUFACTURER_IDX     0
#define STRING_PRODUCT_IDX          1
#define STRING_DESCRIPTION_IDX      2

static struct usb_string webcam_strings[] = {
    [STRING_MANUFACTURER_IDX].s = webcam_vendor_label,
    [STRING_PRODUCT_IDX].s = webcam_product_label,
    [STRING_DESCRIPTION_IDX].s = webcam_config_label,
    {  }
};

static struct usb_gadget_strings webcam_stringtab = {
    .language = 0x0409,    /* en-us */
    .strings = webcam_strings,
};

static struct usb_gadget_strings *webcam_device_strings[] = {
    &webcam_stringtab,
    NULL,
};

static struct usb_device_descriptor webcam_device_descriptor = {
    .bLength            = USB_DT_DEVICE_SIZE,
    .bDescriptorType    = USB_DT_DEVICE,
    .bcdUSB             = cpu_to_le16(0x0200),
    .bDeviceClass       = USB_CLASS_MISC,
    .bDeviceSubClass    = 0x02,
    .bDeviceProtocol    = 0x01,
    .bMaxPacketSize0    = 0, /* dynamic */
    .idVendor           = cpu_to_le16(WEBCAM_VENDOR_ID),
    .idProduct          = cpu_to_le16(WEBCAM_PRODUCT_ID),
    .bcdDevice          = cpu_to_le16(WEBCAM_DEVICE_BCD),
    .iManufacturer      = 0, /* dynamic */
    .iProduct           = 0, /* dynamic */
    .iSerialNumber      = 0, /* dynamic */
    .bNumConfigurations = 0, /* dynamic */
};

DECLARE_UVC_HEADER_DESCRIPTOR(1);

static const struct UVC_HEADER_DESCRIPTOR(1) uvc_control_header = {
    .bLength            = UVC_DT_HEADER_SIZE(1),
    .bDescriptorType    = USB_DT_CS_INTERFACE,
    .bDescriptorSubType = UVC_VC_HEADER,
    .bcdUVC             = cpu_to_le16(0x0100),
    .wTotalLength       = 0, /* dynamic */
    .dwClockFrequency   = cpu_to_le32(48000000),
    .bInCollection      = 0, /* dynamic */
    .baInterfaceNr[0]   = 0, /* dynamic */
};

static const struct uvc_camera_terminal_descriptor uvc_camera_terminal = {
    .bLength            = UVC_DT_CAMERA_TERMINAL_SIZE(3),
    .bDescriptorType    = USB_DT_CS_INTERFACE,
    .bDescriptorSubType = UVC_VC_INPUT_TERMINAL,
    .bTerminalID        = 1,
    .wTerminalType      = cpu_to_le16(0x0201),
    .bAssocTerminal     = 0,
    .iTerminal          = 0,
    .wObjectiveFocalLengthMin   = cpu_to_le16(0),
    .wObjectiveFocalLengthMax   = cpu_to_le16(0),
    .wOcularFocalLength         = cpu_to_le16(0),
    .bControlSize       = 3,
    .bmControls[0]      = 2,
    .bmControls[1]      = 0,
    .bmControls[2]      = 0,
};

static const struct uvc_processing_unit_descriptor uvc_processing = {
    .bLength            = UVC_DT_PROCESSING_UNIT_SIZE(2),
    .bDescriptorType    = USB_DT_CS_INTERFACE,
    .bDescriptorSubType = UVC_VC_PROCESSING_UNIT,
    .bUnitID            = 2,
    .bSourceID          = 1,
    .wMaxMultiplier     = cpu_to_le16(16*1024),
    .bControlSize       = 2,
    .bmControls[0]      = 1,
    .bmControls[1]      = 0,
    .iProcessing        = 0,
};

#define UVC_GUID_UVC_PROCESSING \
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01}
static const struct uvc_extension_unit_descriptor uvc_xud = {
    .bLength            = UVC_DT_EXTENSION_UNIT_SIZE(1,1),
    .bDescriptorType    = USB_DT_CS_INTERFACE,
    .bDescriptorSubType = UVC_VC_EXTENSION_UNIT,
    .bUnitID=  4,
    .guidExtensionCode = {0x70, 0x33, 0xf0, \
        0x28, 0x11, 0x63,\
        0x2e, 0x4a, 0xba, \
        0x2c, 0x68, 0x90, \
        0xeb, 0x33, 0x40,\
        0x16},
    .bNumControls =  8,
    .bNrInPins = 1,
    .baSourceID = 2,
    .bControlSize =1,
    .bmControls= 0x1f,
    .iExtension        = 0,
};
static const struct uvc_output_terminal_descriptor uvc_output_terminal = {
    .bLength            = UVC_DT_OUTPUT_TERMINAL_SIZE,
    .bDescriptorType    = USB_DT_CS_INTERFACE,
    .bDescriptorSubType = UVC_VC_OUTPUT_TERMINAL,
    .bTerminalID        = 3,
    .wTerminalType      = cpu_to_le16(0x0101),
    .bAssocTerminal     = 0,
    .bSourceID          = 1,
    .iTerminal          = 0,
};

#if defined(CONFIG_USB_G_WEBCAM_ALL_MODE)

DECLARE_UVC_INPUT_HEADER_DESCRIPTOR(1, 3);

static const struct UVC_INPUT_HEADER_DESCRIPTOR(1, 3) uvc_input_header = {
    .bLength            = UVC_DT_INPUT_HEADER_SIZE(1, 3),
    .bDescriptorType    = USB_DT_CS_INTERFACE,
    .bDescriptorSubType = UVC_VS_INPUT_HEADER,
    .bNumFormats        = 3,
    .wTotalLength       = 0, /* dynamic */
    .bEndpointAddress   = 0, /* dynamic */
    .bmInfo             = 0,
    .bTerminalLink      = 3,
    .bStillCaptureMethod= 0,
    .bTriggerSupport    = 0,
    .bTriggerUsage      = 0,
    .bControlSize       = 1,
    .bmaControls[0][0]  = 0,
    .bmaControls[1][0]  = 0,
    .bmaControls[2][0]  = 4,
};

static const struct uvc_format_uncompressed uvc_format_yuv = {
    .bLength            = UVC_DT_FORMAT_UNCOMPRESSED_SIZE,
    .bDescriptorType    = USB_DT_CS_INTERFACE,
    .bDescriptorSubType = UVC_VS_FORMAT_UNCOMPRESSED,
    .bFormatIndex       = 1,
    .bNumFrameDescriptors = 2,
    .guidFormat         =
        { 'Y',  'U',  'Y',  '2', 0x00, 0x00, 0x10, 0x00,
         0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71},
    .bBitsPerPixel      = 16,
    .bDefaultFrameIndex = 1,
    .bAspectRatioX      = 0,
    .bAspectRatioY      = 0,
    .bmInterfaceFlags   = 0,
    .bCopyProtect       = 0,
};

DECLARE_UVC_FRAME_UNCOMPRESSED(1);
DECLARE_UVC_FRAME_UNCOMPRESSED(3);

static const struct UVC_FRAME_UNCOMPRESSED(3) uvc_frame_yuv_360p = {
    .bLength                = UVC_DT_FRAME_UNCOMPRESSED_SIZE(3),
    .bDescriptorType        = USB_DT_CS_INTERFACE,
    .bDescriptorSubType     = UVC_VS_FRAME_UNCOMPRESSED,
    .bFrameIndex            = 1,
    .bmCapabilities         = 0,
    .wWidth                 = cpu_to_le16(640),
    .wHeight                = cpu_to_le16(360),
    .dwMinBitRate           = cpu_to_le32(18432000),
    .dwMaxBitRate           = cpu_to_le32(55296000),
    .dwMaxVideoFrameBufferSize = cpu_to_le32(460800),
    .dwDefaultFrameInterval = cpu_to_le32(666666),
    .bFrameIntervalType     = 3,
    .dwFrameInterval[0]     = cpu_to_le32(666666),
    .dwFrameInterval[1]     = cpu_to_le32(1000000),
    .dwFrameInterval[2]     = cpu_to_le32(5000000),
};

static const struct UVC_FRAME_UNCOMPRESSED(1) uvc_frame_yuv_720p = {
    .bLength                = UVC_DT_FRAME_UNCOMPRESSED_SIZE(1),
    .bDescriptorType        = USB_DT_CS_INTERFACE,
    .bDescriptorSubType     = UVC_VS_FRAME_UNCOMPRESSED,
    .bFrameIndex            = 2,
    .bmCapabilities         = 0,
    .wWidth                 = cpu_to_le16(1280),
    .wHeight                = cpu_to_le16(720),
    .dwMinBitRate           = cpu_to_le32(29491200),
    .dwMaxBitRate           = cpu_to_le32(29491200),
    .dwMaxVideoFrameBufferSize = cpu_to_le32(1843200),
    .dwDefaultFrameInterval = cpu_to_le32(5000000),
    .bFrameIntervalType     = 1,
    .dwFrameInterval[0]     = cpu_to_le32(5000000),
};

static const struct uvc_format_mjpeg uvc_format_mjpg = {
    .bLength                = UVC_DT_FORMAT_MJPEG_SIZE,
    .bDescriptorType        = USB_DT_CS_INTERFACE,
    .bDescriptorSubType     = UVC_VS_FORMAT_MJPEG,
    .bFormatIndex           = 1,
    .bNumFrameDescriptors   = 2,
    .bmFlags                = 0,
    .bDefaultFrameIndex     = 1,
    .bAspectRatioX          = 0,
    .bAspectRatioY          = 0,
    .bmInterfaceFlags       = 0,
    .bCopyProtect           = 0,
};

DECLARE_UVC_FRAME_MJPEG(1);
DECLARE_UVC_FRAME_MJPEG(2);
DECLARE_UVC_FRAME_MJPEG(3);

static const struct UVC_FRAME_MJPEG(3) uvc_frame_mjpg_360p = {
    .bLength                = UVC_DT_FRAME_MJPEG_SIZE(3),
    .bDescriptorType        = USB_DT_CS_INTERFACE,
    .bDescriptorSubType     = UVC_VS_FRAME_MJPEG,
    .bFrameIndex            = 1,
    .bmCapabilities         = 0,
    .wWidth                 = cpu_to_le16(640),
    .wHeight                = cpu_to_le16(360),
    .dwMinBitRate           = cpu_to_le32(18432000),
    .dwMaxBitRate           = cpu_to_le32(55296000),
    .dwMaxVideoFrameBufferSize = cpu_to_le32(460800),
    .dwDefaultFrameInterval = cpu_to_le32(666666),
    .bFrameIntervalType     = 3,
    .dwFrameInterval[0]     = cpu_to_le32(666666),
    .dwFrameInterval[1]     = cpu_to_le32(1000000),
    .dwFrameInterval[2]     = cpu_to_le32(5000000),
};

static const struct UVC_FRAME_MJPEG(1) uvc_frame_mjpg_720p = {
    .bLength                = UVC_DT_FRAME_MJPEG_SIZE(1),
    .bDescriptorType        = USB_DT_CS_INTERFACE,
    .bDescriptorSubType     = UVC_VS_FRAME_MJPEG,
    .bFrameIndex            = 1,
    .bmCapabilities         = 0,
    .wWidth                 = cpu_to_le16(1280),
    .wHeight                = cpu_to_le16(720),
    .dwMinBitRate           = cpu_to_le32(29491200),
    .dwMaxBitRate           = cpu_to_le32(29491200),
    .dwMaxVideoFrameBufferSize = cpu_to_le32(1843200),
    .dwDefaultFrameInterval = cpu_to_le32(5000000),
    .bFrameIntervalType     = 1,
    .dwFrameInterval[0]     = cpu_to_le32(5000000),
};

static const struct UVC_FRAME_MJPEG(1) uvc_frame_mjpg_1080p = {
    .bLength                = UVC_DT_FRAME_MJPEG_SIZE(1),
    .bDescriptorType        = USB_DT_CS_INTERFACE,
    .bDescriptorSubType     = UVC_VS_FRAME_MJPEG,
    .bFrameIndex            = 2,
    .bmCapabilities         = 0,
    .wWidth                 = cpu_to_le16(1920),
    .wHeight                = cpu_to_le16(1080),
    .dwMinBitRate           = cpu_to_le32(29491200),
    .dwMaxBitRate           = cpu_to_le32(29491200),
    .dwMaxVideoFrameBufferSize = cpu_to_le32(1843200),
    .dwDefaultFrameInterval = cpu_to_le32(5000000),
    .bFrameIntervalType     = 1,
    .dwFrameInterval[0]     = cpu_to_le32(5000000),
};

static const struct uvc_format2_h264 uvc_format_h264_ = {
    .bLength                = 28,
    .bDescriptorType        = USB_DT_CS_INTERFACE,
    .bDescriptorSubtype     = UVC_VS_FORMAT_FRAME_BASED,
    .bFormatIndex           = 2,
    .bNumFrameDescriptors   = 2,
    .guidFormat             = { 'H',  '2',  '6',  '4', 0x00, 0x00, 0x10, 0x00,
    0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71},//     {48323634-0000-1000-8000-00aa00389b71},
    .bBitsPerPixel          = 16,
    .bDefaultFrameIndex     = 1,
    .bAspectRatioX          = 16,
    .bAspectRatioY          = 9,
    .bmInterlaceFlags       = 0x00,
    .bCopyProtect           = 0,
    .bVariableSize          = 1,
};

DECLARE_UVC_FRAME_H2642(2);

static const struct UVC_FRAME_H2642(2) uvc_frame_h264_720p = {
    .bLength                = UVC_DT_FRAME_H264_SIZE2(2) ,
    .bDescriptorType        = USB_DT_CS_INTERFACE,
    .bDescriptorSubtype     = UVC_VS_FRAME_FRAME_BASED ,
    .bFrameIndex            = 1,
    .bmCapabilities         = 0x00,
    .wWidth                 = cpu_to_le16(1280),
    .wHeight                = cpu_to_le16(720),
    .dwMinBitRate           = cpu_to_le32(2000000),
    .dwMaxBitRate           = cpu_to_le32(16000000),
    .dwDefaultFrameInterval = cpu_to_le32(400000),
    .bFrameIntervalType     = 2,
    .dwBytesPerLine         = 0,
    .dwFrameInterval[0]     = cpu_to_le32(400000),
    .dwFrameInterval[1]     = cpu_to_le32(800000),
};

static const struct UVC_FRAME_H2642(2) uvc_frame_h264_1080p = {
    .bLength                = UVC_DT_FRAME_H264_SIZE2(2) ,
    .bDescriptorType        = USB_DT_CS_INTERFACE,
    .bDescriptorSubtype     = UVC_VS_FRAME_FRAME_BASED ,
    .bFrameIndex            = 2 ,
    .bmCapabilities         = 0x00,
    .wWidth                 = cpu_to_le32(1920),
    .wHeight                = cpu_to_le32(1080),
    .dwMinBitRate           = cpu_to_le32(2000000),
    .dwMaxBitRate           = cpu_to_le32(10000000),
    .dwDefaultFrameInterval = cpu_to_le32(400000),
    .bFrameIntervalType     = 2,
    .dwBytesPerLine         = 0,
    .dwFrameInterval[0]     = cpu_to_le32(400000),
    .dwFrameInterval[1]     = cpu_to_le32(800000),
};

static const struct UVC_FRAME_H2642(2) uvc_frame_h264_360p = {
    .bLength                = UVC_DT_FRAME_H264_SIZE2(2) ,
    .bDescriptorType        = 36,
    .bDescriptorSubtype     = 17 ,
    .bFrameIndex            = 1 ,
    .bmCapabilities         = 0x00,
    .wWidth                 = cpu_to_le32(640),
    .wHeight                = cpu_to_le32(360),
    .dwMinBitRate           = cpu_to_le32(497664000),
    .dwMaxBitRate           = cpu_to_le32(995328000),
    .dwDefaultFrameInterval = cpu_to_le32(333333),
    .bFrameIntervalType     = 2,
    .dwBytesPerLine         = 0,
    .dwFrameInterval[0]     = cpu_to_le32(333333),
    .dwFrameInterval[1]     = cpu_to_le32(666666),
};
#else
DECLARE_UVC_INPUT_HEADER_DESCRIPTOR(1, 2);

static const struct UVC_INPUT_HEADER_DESCRIPTOR(1, 2) uvc_input_header = {
    .bLength            = UVC_DT_INPUT_HEADER_SIZE(1, 2),
    .bDescriptorType    = USB_DT_CS_INTERFACE,
    .bDescriptorSubType = UVC_VS_INPUT_HEADER,
    .bNumFormats        = 2,
    .wTotalLength       = 0, /* dynamic */
    .bEndpointAddress   = 0, /* dynamic */
    .bmInfo             = 0,
    .bTerminalLink      = 3,
    .bStillCaptureMethod= 0,
    .bTriggerSupport    = 0,
    .bTriggerUsage      = 0,
    .bControlSize       = 1,
    .bmaControls[0][0]  = 0,
    .bmaControls[1][0]  = 4,
};

static const struct uvc_format_uncompressed uvc_format_yuv = {
    .bLength            = UVC_DT_FORMAT_UNCOMPRESSED_SIZE,
    .bDescriptorType    = USB_DT_CS_INTERFACE,
    .bDescriptorSubType = UVC_VS_FORMAT_UNCOMPRESSED,
    .bFormatIndex       = 1,
    .bNumFrameDescriptors = 2,
    .guidFormat         =
        { 'Y',  'U',  'Y',  '2', 0x00, 0x00, 0x10, 0x00,
         0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71},
    .bBitsPerPixel      = 16,
    .bDefaultFrameIndex = 1,
    .bAspectRatioX      = 0,
    .bAspectRatioY      = 0,
    .bmInterfaceFlags   = 0,
    .bCopyProtect       = 0,
};

DECLARE_UVC_FRAME_UNCOMPRESSED(1);
DECLARE_UVC_FRAME_UNCOMPRESSED(3);

static const struct UVC_FRAME_UNCOMPRESSED(3) uvc_frame_yuv_360p = {
    .bLength                = UVC_DT_FRAME_UNCOMPRESSED_SIZE(3),
    .bDescriptorType        = USB_DT_CS_INTERFACE,
    .bDescriptorSubType     = UVC_VS_FRAME_UNCOMPRESSED,
    .bFrameIndex            = 1,
    .bmCapabilities         = 0,
    .wWidth                 = cpu_to_le16(640),
    .wHeight                = cpu_to_le16(360),
    .dwMinBitRate           = cpu_to_le32(18432000),
    .dwMaxBitRate           = cpu_to_le32(55296000),
    .dwMaxVideoFrameBufferSize = cpu_to_le32(460800),
    .dwDefaultFrameInterval = cpu_to_le32(666666),
    .bFrameIntervalType     = 3,
    .dwFrameInterval[0]     = cpu_to_le32(666666),
    .dwFrameInterval[1]     = cpu_to_le32(1000000),
    .dwFrameInterval[2]     = cpu_to_le32(5000000),
};

static const struct UVC_FRAME_UNCOMPRESSED(1) uvc_frame_yuv_720p = {
    .bLength                = UVC_DT_FRAME_UNCOMPRESSED_SIZE(1),
    .bDescriptorType        = USB_DT_CS_INTERFACE,
    .bDescriptorSubType     = UVC_VS_FRAME_UNCOMPRESSED,
    .bFrameIndex            = 2,
    .bmCapabilities         = 0,
    .wWidth                 = cpu_to_le16(1280),
    .wHeight                = cpu_to_le16(720),
    .dwMinBitRate           = cpu_to_le32(29491200),
    .dwMaxBitRate           = cpu_to_le32(29491200),
    .dwMaxVideoFrameBufferSize = cpu_to_le32(1843200),
    .dwDefaultFrameInterval = cpu_to_le32(5000000),
    .bFrameIntervalType     = 1,
    .dwFrameInterval[0]     = cpu_to_le32(5000000),
};

static const struct uvc_format_mjpeg uvc_format_mjpg = {
    .bLength                = UVC_DT_FORMAT_MJPEG_SIZE,
    .bDescriptorType        = USB_DT_CS_INTERFACE,
    .bDescriptorSubType     = UVC_VS_FORMAT_MJPEG,
    .bFormatIndex           = 2,
    .bNumFrameDescriptors   = 1,
    .bmFlags                = 0,
    .bDefaultFrameIndex     = 1,
    .bAspectRatioX          = 0,
    .bAspectRatioY          = 0,
    .bmInterfaceFlags       = 0,
    .bCopyProtect           = 0,
};

DECLARE_UVC_FRAME_MJPEG(1);
DECLARE_UVC_FRAME_MJPEG(2);
DECLARE_UVC_FRAME_MJPEG(3);

static const struct UVC_FRAME_MJPEG(3) uvc_frame_mjpg_360p = {
    .bLength                = UVC_DT_FRAME_MJPEG_SIZE(3),
    .bDescriptorType        = USB_DT_CS_INTERFACE,
    .bDescriptorSubType     = UVC_VS_FRAME_MJPEG,
    .bFrameIndex            = 1,
    .bmCapabilities         = 0,
    .wWidth                 = cpu_to_le16(640),
    .wHeight                = cpu_to_le16(360),
    .dwMinBitRate           = cpu_to_le32(18432000),
    .dwMaxBitRate           = cpu_to_le32(55296000),
    .dwMaxVideoFrameBufferSize = cpu_to_le32(460800),
    .dwDefaultFrameInterval = cpu_to_le32(666666),
    .bFrameIntervalType     = 3,
    .dwFrameInterval[0]     = cpu_to_le32(666666),
    .dwFrameInterval[1]     = cpu_to_le32(1000000),
    .dwFrameInterval[2]     = cpu_to_le32(5000000),
};

static const struct UVC_FRAME_MJPEG(1) uvc_frame_mjpg_720p = {
    .bLength                = UVC_DT_FRAME_MJPEG_SIZE(1),
    .bDescriptorType        = USB_DT_CS_INTERFACE,
    .bDescriptorSubType     = UVC_VS_FRAME_MJPEG,
    .bFrameIndex            = 1,
    .bmCapabilities         = 0,
    .wWidth                 = cpu_to_le16(1280),
    .wHeight                = cpu_to_le16(720),
    .dwMinBitRate           = cpu_to_le32(29491200),
    .dwMaxBitRate           = cpu_to_le32(29491200),
    .dwMaxVideoFrameBufferSize = cpu_to_le32(1843200),
    .dwDefaultFrameInterval = cpu_to_le32(5000000),
    .bFrameIntervalType     = 1,
    .dwFrameInterval[0]     = cpu_to_le32(5000000),
};

static const struct UVC_FRAME_MJPEG(1) uvc_frame_mjpg_1080p = {
    .bLength                = UVC_DT_FRAME_MJPEG_SIZE(1),
    .bDescriptorType        = USB_DT_CS_INTERFACE,
    .bDescriptorSubType     = UVC_VS_FRAME_MJPEG,
    .bFrameIndex            = 1,
    .bmCapabilities         = 0,
    .wWidth                 = cpu_to_le16(1920),
    .wHeight                = cpu_to_le16(1080),
    .dwMinBitRate           = cpu_to_le32(29491200),
    .dwMaxBitRate           = cpu_to_le32(29491200),
    .dwMaxVideoFrameBufferSize = cpu_to_le32(1843200),
    .dwDefaultFrameInterval = cpu_to_le32(5000000),
    .bFrameIntervalType     = 1,
    .dwFrameInterval[0]     = cpu_to_le32(5000000),
};

static const struct uvc_format2_h264 uvc_format_h264_ = {
    .bLength                = 28,
    .bDescriptorType        = USB_DT_CS_INTERFACE,
    .bDescriptorSubtype     = UVC_VS_FORMAT_FRAME_BASED,
    .bFormatIndex           = 2,
    .bNumFrameDescriptors   = 1,
    .guidFormat             = { 'H',  '2',  '6',  '4', 0x00, 0x00, 0x10, 0x00,
    0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71},//     {48323634-0000-1000-8000-00aa00389b71},
    .bBitsPerPixel          = 16,
    .bDefaultFrameIndex     = 1,
    .bAspectRatioX          = 16,
    .bAspectRatioY          = 9,
    .bmInterlaceFlags       = 0x00,
    .bCopyProtect           = 0,
    .bVariableSize          = 1,
};

DECLARE_UVC_FRAME_H2642(2);

static const struct UVC_FRAME_H2642(2) uvc_frame_h264_720p = {
    .bLength                = UVC_DT_FRAME_H264_SIZE2(2) ,
    .bDescriptorType        = USB_DT_CS_INTERFACE,
    .bDescriptorSubtype     = UVC_VS_FRAME_FRAME_BASED ,
    .bFrameIndex            = 2,
    .bmCapabilities         = 0x00,
    .wWidth                 = cpu_to_le16(1280),
    .wHeight                = cpu_to_le16(720),
    .dwMinBitRate           = cpu_to_le32(2000000),
    .dwMaxBitRate           = cpu_to_le32(16000000),
    .dwDefaultFrameInterval = cpu_to_le32(400000),
    .bFrameIntervalType     = 2,
    .dwBytesPerLine         = 0,
    .dwFrameInterval[0]     = cpu_to_le32(400000),
    .dwFrameInterval[1]     = cpu_to_le32(800000),
};

static const struct UVC_FRAME_H2642(2) uvc_frame_h264_1080p = {
    .bLength                = UVC_DT_FRAME_H264_SIZE2(2) ,
    .bDescriptorType        = USB_DT_CS_INTERFACE,
    .bDescriptorSubtype     = UVC_VS_FRAME_FRAME_BASED ,
    .bFrameIndex            = 2 ,
    .bmCapabilities         = 0x00,
    .wWidth                 = cpu_to_le32(1920),
    .wHeight                = cpu_to_le32(1080),
    .dwMinBitRate           = cpu_to_le32(2000000),
    .dwMaxBitRate           = cpu_to_le32(10000000),
    .dwDefaultFrameInterval = cpu_to_le32(400000),
    .bFrameIntervalType     = 2,
    .dwBytesPerLine         = 0,
    .dwFrameInterval[0]     = cpu_to_le32(400000),
    .dwFrameInterval[1]     = cpu_to_le32(800000),
};

static const struct UVC_FRAME_H2642(2) uvc_frame_h264_360p = {
    .bLength                = UVC_DT_FRAME_H264_SIZE2(2) ,
    .bDescriptorType        = 36,
    .bDescriptorSubtype     = 17 ,
    .bFrameIndex            = 1 ,
    .bmCapabilities         = 0x00,
    .wWidth                 = cpu_to_le32(640),
    .wHeight                = cpu_to_le32(360),
    .dwMinBitRate           = cpu_to_le32(497664000),
    .dwMaxBitRate           = cpu_to_le32(995328000),
    .dwDefaultFrameInterval = cpu_to_le32(333333),
    .bFrameIntervalType     = 2,
    .dwBytesPerLine         = 0,
    .dwFrameInterval[0]     = cpu_to_le32(333333),
    .dwFrameInterval[1]     = cpu_to_le32(666666),
};

#endif

static const struct uvc_color_matching_descriptor uvc_color_matching = {
    .bLength                = UVC_DT_COLOR_MATCHING_SIZE,
    .bDescriptorType        = USB_DT_CS_INTERFACE,
    .bDescriptorSubType     = UVC_VS_COLORFORMAT,
    .bColorPrimaries        = 1,
    .bTransferCharacteristics = 1,
    .bMatrixCoefficients    = 4,
};

static const struct uvc_descriptor_header * const uvc_control_cls[] = {
    (const struct uvc_descriptor_header *) &uvc_control_header,
    (const struct uvc_descriptor_header *) &uvc_camera_terminal,
    (const struct uvc_descriptor_header *) &uvc_processing,
    (const struct uvc_descriptor_header *) &uvc_xud,
    (const struct uvc_descriptor_header *) &uvc_output_terminal,
    NULL,
};
#if defined(CONFIG_USB_G_WEBCAM_ALL_MODE)
static const struct uvc_descriptor_header * const uvc_fs_streaming_cls[] = {
    (const struct uvc_descriptor_header *) &uvc_input_header,
    (const struct uvc_descriptor_header *) &uvc_format_mjpg,
    (const struct uvc_descriptor_header *) &uvc_frame_mjpg_720p,
    (const struct uvc_descriptor_header *) &uvc_frame_mjpg_1080p,
    (const struct uvc_descriptor_header *) &uvc_format_h264_,
    (const struct uvc_descriptor_header *) &uvc_frame_h264_720p,
    (const struct uvc_descriptor_header *) &uvc_frame_h264_1080p,
    (const struct uvc_descriptor_header *) &uvc_color_matching,
    NULL,
};

static const struct uvc_descriptor_header * const uvc_hs_streaming_cls[] = {
    (const struct uvc_descriptor_header *) &uvc_input_header,
    (const struct uvc_descriptor_header *) &uvc_format_mjpg,
    (const struct uvc_descriptor_header *) &uvc_frame_mjpg_720p,
    (const struct uvc_descriptor_header *) &uvc_frame_mjpg_1080p,
    (const struct uvc_descriptor_header *) &uvc_format_h264_,
    (const struct uvc_descriptor_header *) &uvc_frame_h264_720p,
    (const struct uvc_descriptor_header *) &uvc_frame_h264_1080p,
    (const struct uvc_descriptor_header *) &uvc_color_matching,
    NULL,
};
#else
static const struct uvc_descriptor_header * const uvc_fs_streaming_cls[] = {
    (const struct uvc_descriptor_header *) &uvc_input_header,
#if defined(CONFIG_USB_G_WEBCAM_YUYV_360P)
    (const struct uvc_descriptor_header *) &uvc_format_yuv,
    (const struct uvc_descriptor_header *) &uvc_frame_yuv_360p,
#elif defined(CONFIG_USB_G_WEBCAM_YUYV_720P)
    (const struct uvc_descriptor_header *) &uvc_format_yuv,
    (const struct uvc_descriptor_header *) &uvc_frame_yuv_720p,
#elif defined(CONFIG_USB_G_WEBCAM_MJPG_360P)
    (const struct uvc_descriptor_header *) &uvc_format_mjpg,
    (const struct uvc_descriptor_header *) &uvc_frame_mjpg_360p,
#elif defined(CONFIG_USB_G_WEBCAM_MJPG_720P)
    (const struct uvc_descriptor_header *) &uvc_format_mjpg,
    (const struct uvc_descriptor_header *) &uvc_frame_mjpg_720p,
#elif defined(CONFIG_USB_G_WEBCAM_MJPG_1080P)
    (const struct uvc_descriptor_header *) &uvc_format_mjpg,
    (const struct uvc_descriptor_header *) &uvc_frame_mjpg_1080p,
#elif defined(CONFIG_USB_G_WEBCAM_H264_360P)
    (const struct uvc_descriptor_header *) &uvc_format_h264_,
    (const struct uvc_descriptor_header *) &uvc_frame_h264_360p,
#elif defined(CONFIG_USB_G_WEBCAM_H264_720P)
    (const struct uvc_descriptor_header *) &uvc_format_h264_,
    (const struct uvc_descriptor_header *) &uvc_frame_h264_720p,
#elif defined(CONFIG_USB_G_WEBCAM_H264_1080P)
    (const struct uvc_descriptor_header *) &uvc_format_h264_,
    (const struct uvc_descriptor_header *) &uvc_frame_h264_1080p,
#endif
    (const struct uvc_descriptor_header *) &uvc_color_matching,
    NULL,
};

static const struct uvc_descriptor_header * const uvc_hs_streaming_cls[] = {
    (const struct uvc_descriptor_header *) &uvc_input_header,
#if defined(CONFIG_USB_G_WEBCAM_YUYV_360P)
    (const struct uvc_descriptor_header *) &uvc_format_yuv,
    (const struct uvc_descriptor_header *) &uvc_frame_yuv_360p,
#elif defined(CONFIG_USB_G_WEBCAM_YUYV_720P)
    (const struct uvc_descriptor_header *) &uvc_format_yuv,
    (const struct uvc_descriptor_header *) &uvc_frame_yuv_720p,
#elif defined(CONFIG_USB_G_WEBCAM_MJPG_360P)
    (const struct uvc_descriptor_header *) &uvc_format_mjpg,
    (const struct uvc_descriptor_header *) &uvc_frame_mjpg_360p,
#elif defined(CONFIG_USB_G_WEBCAM_MJPG_720P)
    (const struct uvc_descriptor_header *) &uvc_format_mjpg,
    (const struct uvc_descriptor_header *) &uvc_frame_mjpg_720p,
#elif defined(CONFIG_USB_G_WEBCAM_MJPG_1080P)
    (const struct uvc_descriptor_header *) &uvc_format_mjpg,
    (const struct uvc_descriptor_header *) &uvc_frame_mjpg_1080p,
#elif defined(CONFIG_USB_G_WEBCAM_H264_360P)
    (const struct uvc_descriptor_header *) &uvc_format_h264_,
    (const struct uvc_descriptor_header *) &uvc_frame_h264_360p,
#elif defined(CONFIG_USB_G_WEBCAM_H264_720P)
    (const struct uvc_descriptor_header *) &uvc_format_h264_,
    (const struct uvc_descriptor_header *) &uvc_frame_h264_720p,
#elif defined(CONFIG_USB_G_WEBCAM_H264_1080P)
    (const struct uvc_descriptor_header *) &uvc_format_h264_,
    (const struct uvc_descriptor_header *) &uvc_frame_h264_1080p,
#endif
    (const struct uvc_descriptor_header *) &uvc_color_matching,
    NULL,
};
#endif

/* --------------------------------------------------------------------------
 * USB configuration
 */

static int __init
webcam_config_bind(struct usb_configuration *c)
{
    return uvc_bind_config(c, uvc_control_cls, uvc_fs_streaming_cls,
                   uvc_hs_streaming_cls);
}

static struct usb_configuration webcam_config_driver = {
    .label                  = webcam_config_label,
    .bConfigurationValue    = 1,
    .iConfiguration         = 0, /* dynamic */
    .bmAttributes           = USB_CONFIG_ATT_SELFPOWER,
    .bMaxPower              = CONFIG_USB_GADGET_VBUS_DRAW / 2,
};

static int /* __init_or_exit */
webcam_unbind(struct usb_composite_dev *cdev)
{
    return 0;
}

static int __init
webcam_bind(struct usb_composite_dev *cdev)
{
    int ret;

    /* Allocate string descriptor numbers ... note that string contents
     * can be overridden by the composite_dev glue.
     */
    if ((ret = usb_string_id(cdev)) < 0)
        goto error;
    webcam_strings[STRING_MANUFACTURER_IDX].id = ret;
    webcam_device_descriptor.iManufacturer = ret;

    if ((ret = usb_string_id(cdev)) < 0)
        goto error;
    webcam_strings[STRING_PRODUCT_IDX].id = ret;
    webcam_device_descriptor.iProduct = ret;

    if ((ret = usb_string_id(cdev)) < 0)
        goto error;
    webcam_strings[STRING_DESCRIPTION_IDX].id = ret;
    webcam_config_driver.iConfiguration = ret;

    /* Register our configuration. */
    if ((ret = usb_add_config(cdev, &webcam_config_driver,
                    webcam_config_bind)) < 0)
        goto error;

    INFO(cdev, "Webcam Video Gadget\n");
    return 0;

error:
    webcam_unbind(cdev);
    return ret;
}

/* --------------------------------------------------------------------------
 * Driver
 */

static struct usb_composite_driver webcam_driver = {
    .name           = "g_webcam",
    .dev            = &webcam_device_descriptor,
    .strings        = webcam_device_strings,
    .max_speed      = USB_SPEED_HIGH,
    .unbind         = webcam_unbind,
};

static int __init
webcam_init(void)
{
    return usb_composite_probe(&webcam_driver, webcam_bind);
}

static void __exit
webcam_cleanup(void)
{
    usb_composite_unregister(&webcam_driver);
}

module_init(webcam_init);
module_exit(webcam_cleanup);

MODULE_AUTHOR("Laurent Pinchart");
MODULE_DESCRIPTION("Webcam Video Gadget");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1.0");

