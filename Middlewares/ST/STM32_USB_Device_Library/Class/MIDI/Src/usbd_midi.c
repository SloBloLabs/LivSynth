/**
  ******************************************************************************
  * @file    usbd_midi.c
  * @author  MCD Application Team
  * @brief   This file provides the Midi core functions.
  *
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  * @verbatim
  *
  *          ===================================================================
  *                                MIDI Class  Description
  *          ===================================================================
  *           This driver manages the Midi Class 1.0 following the "USB Device Class Definition for
  *           Midi Devices V1.0 Mar 18, 98".
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *
  *
  *  @endverbatim
  ******************************************************************************
  */

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
- "stm32xxxxx_{eval}{discovery}_midi.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbd_midi.h"
#include "usbd_ctlreq.h"


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_MIDI
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_MIDI_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_MIDI_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_MIDI_Private_Macros
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_MIDI_Private_FunctionPrototypes
  * @{
  */
static uint8_t USBD_MIDI_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_MIDI_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_MIDI_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
//static uint8_t USBD_MIDI_EP0_TxReady(USBD_HandleTypeDef *pdev);
//static uint8_t USBD_MIDI_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t USBD_MIDI_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_MIDI_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
#ifndef USE_USBD_COMPOSITE
static uint8_t *USBD_MIDI_GetCfgDesc(uint16_t *length);
static uint8_t *USBD_MIDI_GetDeviceQualifierDesc(uint16_t *length);
#endif /* USE_USBD_COMPOSITE  */

/**
  * @}
  */

/** @defgroup USBD_MIDI_Private_Variables
  * @{
  */

USBD_ClassTypeDef USBD_MIDI =
{
  USBD_MIDI_Init,
  USBD_MIDI_DeInit,
  USBD_MIDI_Setup,
  NULL, //USBD_MIDI_EP0_TxReady,
  NULL, //USBD_MIDI_EP0_RxReady,
  USBD_MIDI_DataIn,
  USBD_MIDI_DataOut,
  NULL, //USBD_MIDI_SOF,
  NULL, //USBD_MIDI_IsoINIncomplete,
  NULL, //USBD_MIDI_IsoOutIncomplete,
#ifdef USE_USBD_COMPOSITE
  NULL,
  NULL,
  NULL,
  NULL,
#else
  USBD_MIDI_GetCfgDesc,
  USBD_MIDI_GetCfgDesc,
  USBD_MIDI_GetCfgDesc,
  USBD_MIDI_GetDeviceQualifierDesc,
#endif /* USE_USBD_COMPOSITE  */
};

#ifndef USE_USBD_COMPOSITE
/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_MIDI_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

/* USB MIDI device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_MIDI_CfgDesc[USB_MIDI_CONFIG_DESC_SIZE] __ALIGN_END =
{
  /* Configuration 1 */
  0x09,                                 /* bLength */
  USB_DESC_TYPE_CONFIGURATION,          /* bDescriptorType */
  LOBYTE(USB_MIDI_CONFIG_DESC_SIZE),    /* wTotalLength */
  HIBYTE(USB_MIDI_CONFIG_DESC_SIZE),
  0x01,                                 /* bNumInterfaces */
  0x01,                                 /* bConfigurationValue */
  0x00,                                 /* iConfiguration */
#if (USBD_SELF_POWERED == 1U)
  0xC0,                                 /* bmAttributes: Self Powered according to user configuration */
#else
  0x80,                                 /* bmAttributes: Bus Powered according to user configuration */
#endif /* USBD_SELF_POWERED */
  USBD_MAX_POWER,                       /* MaxPower (mA) */
  /* 09 byte*/

  /************** MIDI Adapter Standard MS Interface Descriptor ****************/
  0x09,                         /*bLength: Interface Descriptor size*/
  USB_DESC_TYPE_INTERFACE,      /*bDescriptorType: Interface descriptor type*/
  0x00,                         /*bInterfaceNumber: Index of this interface.*/
  0x00,                         /*bAlternateSetting: Alternate setting*/
  0x02,                         /*bNumEndpoints*/
  USB_DEVICE_CLASS_AUDIO,       /*bInterfaceClass: AUDIO*/
  AUDIO_SUBCLASS_MIDISTREAMING, /*bInterfaceSubClass : MIDISTREAMING*/
  0x00,                         /*nInterfaceProtocol : Unused*/
  0x00,                         /*iInterface: 0*/

// https://www.usb.org/sites/default/files/midi10.pdf Page 21 Table 6-2: Class-Specific MS Interface Header Descriptor
  /******************** MIDI Adapter Class-specific MS Interface Descriptor ********************/
  0x07,                 /*bLength: Descriptor size*/
  0x24,                 /*bDescriptorType: CS_INTERFACE descriptor*/
  0x01,                 /*bDescriptorSubtype: MS_HEADER subtype*/
  0x00,
  0x01,                 /*BcdADC: Revision of this class specification*/
  USB_MIDI_REPORT_DESC_SIZE,
  0x00,                  /*wTotalLength: Total size of class-specific descriptors*/

// https://www.usb.org/sites/default/files/midi10.pdf Page 22 Table 6-3: MIDI IN Jack Descriptor
#if MIDI_IN_PORTS_NUM >= 1
  /******************** MIDI Adapter MIDI IN Jack Descriptor (External) ********************/
  0x06,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x02,                   /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
  0x02,                   /*bJackType: EXTERNAL.*/
  MIDI_JACK_1,            /*bJackID: ID of this Jack.*/
  0x00,                   /*iJack: Unused.*/

// https://www.usb.org/sites/default/files/midi10.pdf Page 22 Table 6-4: MIDI OUT Jack Descriptor
  /******************** MIDI Adapter MIDI OUT Jack Descriptor (Embedded) ********************/
  0x09,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x03,                   /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
  0x01,                   /*bJackType: EMBEDDED*/
  MIDI_JACK_2,            /*bJackID: ID of this Jack.*/
  0x01,                   /*bNrInputPins: Number of Input Pins of this Jack.*/
  MIDI_JACK_1,            /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
  0x01,                   /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
  0x00,                   /*iJack: Unused.*/
#endif

#if MIDI_IN_PORTS_NUM >= 2
  /******************** MIDI Adapter MIDI IN Jack Descriptor (External) ********************/
  0x06,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x02,                   /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
  0x02,                   /*bJackType: EXTERNAL.*/
  MIDI_JACK_3,            /*bJackID: ID of this Jack.*/
  0x00,                   /*iJack: Unused.*/

  /******************** MIDI Adapter MIDI OUT Jack Descriptor (Embedded) ********************/
  0x09,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x03,                   /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
  0x01,                   /*bJackType: EMBEDDED*/
  MIDI_JACK_4,            /*bJackID: ID of this Jack.*/
  0x01,                   /*bNrInputPins: Number of Input Pins of this Jack.*/
  MIDI_JACK_3,            /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
  0x01,                   /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
  0x00,                   /*iJack: Unused.*/
#endif

#if MIDI_IN_PORTS_NUM >= 3
  /******************** MIDI Adapter MIDI IN Jack Descriptor (External) ********************/
  0x06,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x02,                   /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
  0x02,                   /*bJackType: EXTERNAL.*/
  MIDI_JACK_5,            /*bJackID: ID of this Jack.*/
  0x00,                   /*iJack: Unused.*/

  /******************** MIDI Adapter MIDI OUT Jack Descriptor (Embedded) ********************/
  0x09,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x03,                   /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
  0x01,                   /*bJackType: EMBEDDED*/
  MIDI_JACK_6,            /*bJackID: ID of this Jack.*/
  0x01,                   /*bNrInputPins: Number of Input Pins of this Jack.*/
  MIDI_JACK_5,            /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
  0x01,                   /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
  0x00,                   /*iJack: Unused.*/
#endif

#if MIDI_IN_PORTS_NUM >= 4
  /******************** MIDI Adapter MIDI IN Jack Descriptor (External) ********************/
  0x06,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x02,                   /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
  0x02,                   /*bJackType: EXTERNAL.*/
  MIDI_JACK_7,            /*bJackID: ID of this Jack.*/
  0x00,                   /*iJack: Unused.*/

  /******************** MIDI Adapter MIDI OUT Jack Descriptor (Embedded) ********************/
  0x09,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x03,                   /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
  0x01,                   /*bJackType: EMBEDDED*/
  MIDI_JACK_8,            /*bJackID: ID of this Jack.*/
  0x01,                   /*bNrInputPins: Number of Input Pins of this Jack.*/
  MIDI_JACK_7,            /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
  0x01,                   /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
  0x00,                   /*iJack: Unused.*/
#endif

#if MIDI_IN_PORTS_NUM >= 5 
  /******************** MIDI Adapter MIDI IN Jack Descriptor (External) ********************/
  0x06,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x02,                   /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
  0x02,                   /*bJackType: EXTERNAL.*/
  MIDI_JACK_9,            /*bJackID: ID of this Jack.*/
  0x00,                   /*iJack: Unused.*/

  /******************** MIDI Adapter MIDI OUT Jack Descriptor (Embedded) ********************/
  0x09,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x03,                   /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
  0x01,                   /*bJackType: EMBEDDED*/
  MIDI_JACK_10,           /*bJackID: ID of this Jack.*/
  0x01,                   /*bNrInputPins: Number of Input Pins of this Jack.*/
  MIDI_JACK_9,            /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
  0x01,                   /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
  0x00,                   /*iJack: Unused.*/
#endif

#if MIDI_IN_PORTS_NUM >= 6 
  /******************** MIDI Adapter MIDI IN Jack Descriptor (External) ********************/
  0x06,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x02,                   /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
  0x02,                   /*bJackType: EXTERNAL.*/
  MIDI_JACK_11,           /*bJackID: ID of this Jack.*/
  0x00,                   /*iJack: Unused.*/

  /******************** MIDI Adapter MIDI OUT Jack Descriptor (Embedded) ********************/
  0x09,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x03,                   /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
  0x01,                   /*bJackType: EMBEDDED*/
  MIDI_JACK_12,           /*bJackID: ID of this Jack.*/
  0x01,                   /*bNrInputPins: Number of Input Pins of this Jack.*/
  MIDI_JACK_11,           /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
  0x01,                   /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
  0x00,                   /*iJack: Unused.*/
#endif

#if MIDI_IN_PORTS_NUM >= 7 
  /******************** MIDI Adapter MIDI IN Jack Descriptor (External) ********************/
  0x06,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x02,                   /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
  0x02,                   /*bJackType: EXTERNAL.*/
  MIDI_JACK_13,           /*bJackID: ID of this Jack.*/
  0x00,                   /*iJack: Unused.*/

  /******************** MIDI Adapter MIDI OUT Jack Descriptor (Embedded) ********************/
  0x09,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x03,                   /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
  0x01,                   /*bJackType: EMBEDDED*/
  MIDI_JACK_14,           /*bJackID: ID of this Jack.*/
  0x01,                   /*bNrInputPins: Number of Input Pins of this Jack.*/
  MIDI_JACK_13,           /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
  0x01,                   /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
  0x00,                   /*iJack: Unused.*/
#endif

#if MIDI_IN_PORTS_NUM >= 8
  /******************** MIDI Adapter MIDI IN Jack Descriptor (External) ********************/
  0x06,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x02,                   /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
  0x02,                   /*bJackType: EXTERNAL.*/
  MIDI_JACK_15,           /*bJackID: ID of this Jack.*/
  0x00,                   /*iJack: Unused.*/

  /******************** MIDI Adapter MIDI OUT Jack Descriptor (Embedded) ********************/
  0x09,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x03,                   /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
  0x01,                   /*bJackType: EMBEDDED*/
  MIDI_JACK_16,           /*bJackID: ID of this Jack.*/
  0x01,                   /*bNrInputPins: Number of Input Pins of this Jack.*/
  MIDI_JACK_15,           /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
  0x01,                   /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
  0x00,                   /*iJack: Unused.*/
#endif

// https://www.usb.org/sites/default/files/midi10.pdf Page 22 Table 6-3: MIDI IN Jack Descriptor
#if MIDI_OUT_PORTS_NUM >= 1
  /******************** MIDI Adapter MIDI IN Jack Descriptor (Embedded) ********************/
  0x06,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x02,                   /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
  0x01,                   /*bJackType: EMBEDDED*/
  MIDI_JACK_17,           /*bJackID: ID of this Jack.*/
  0x00,                   /*iJack: Unused.*/

// https://www.usb.org/sites/default/files/midi10.pdf Page 22 Table 6-4: MIDI OUT Jack Descriptor
  /******************** MIDI Adapter MIDI OUT Jack Descriptor (External) ********************/
  0x09,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x03,                   /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
  0x02,                   /*bJackType: EXTERNAL.*/
  MIDI_JACK_18,           /*bJackID: ID of this Jack.*/
  0x01,                   /*bNrInputPins: Number of Input Pins of this Jack.*/
  MIDI_JACK_17,           /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
  0x01,                   /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
  0x00,                   /*iJack: Unused.*/
#endif

#if MIDI_OUT_PORTS_NUM >= 2
  /******************** MIDI Adapter MIDI IN Jack Descriptor (Embedded) ********************/
  0x06,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x02,                   /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
  0x01,                   /*bJackType: EMBEDDED*/
  MIDI_JACK_19,           /*bJackID: ID of this Jack.*/
  0x00,                   /*iJack: Unused.*/

  /******************** MIDI Adapter MIDI OUT Jack Descriptor (External) ********************/
  0x09,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x03,                   /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
  0x02,                   /*bJackType: EXTERNAL.*/
  MIDI_JACK_20,           /*bJackID: ID of this Jack.*/
  0x01,                   /*bNrInputPins: Number of Input Pins of this Jack.*/
  MIDI_JACK_19,           /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
  0x01,                   /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
  0x00,                   /*iJack: Unused.*/
#endif

#if MIDI_OUT_PORTS_NUM >= 3
  /******************** MIDI Adapter MIDI IN Jack Descriptor (Embedded) ********************/
  0x06,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x02,                   /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
  0x01,                   /*bJackType: EMBEDDED*/
  MIDI_JACK_21,           /*bJackID: ID of this Jack.*/
  0x00,                   /*iJack: Unused.*/

  /******************** MIDI Adapter MIDI OUT Jack Descriptor (External) ********************/
  0x09,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x03,                   /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
  0x02,                   /*bJackType: EXTERNAL.*/
  MIDI_JACK_22,           /*bJackID: ID of this Jack.*/
  0x01,                   /*bNrInputPins: Number of Input Pins of this Jack.*/
  MIDI_JACK_21,           /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
  0x01,                   /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
  0x00,                   /*iJack: Unused.*/
#endif

#if MIDI_OUT_PORTS_NUM >= 4
  /******************** MIDI Adapter MIDI IN Jack Descriptor (Embedded) ********************/
  0x06,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x02,                   /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
  0x01,                   /*bJackType: EMBEDDED*/
  MIDI_JACK_23,           /*bJackID: ID of this Jack.*/
  0x00,                   /*iJack: Unused.*/

  /******************** MIDI Adapter MIDI OUT Jack Descriptor (External) ********************/
  0x09,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x03,                   /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
  0x02,                   /*bJackType: EXTERNAL.*/
  MIDI_JACK_24,           /*bJackID: ID of this Jack.*/
  0x01,                   /*bNrInputPins: Number of Input Pins of this Jack.*/
  MIDI_JACK_23,           /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
  0x01,                   /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
  0x00,                   /*iJack: Unused.*/
#endif

#if MIDI_OUT_PORTS_NUM >= 5
  /******************** MIDI Adapter MIDI IN Jack Descriptor (Embedded) ********************/
  0x06,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x02,                   /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
  0x01,                   /*bJackType: EMBEDDED*/
  MIDI_JACK_25,           /*bJackID: ID of this Jack.*/
  0x00,                   /*iJack: Unused.*/

  /******************** MIDI Adapter MIDI OUT Jack Descriptor (External) ********************/
  0x09,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x03,                   /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
  0x02,                   /*bJackType: EXTERNAL.*/
  MIDI_JACK_26,           /*bJackID: ID of this Jack.*/
  0x01,                   /*bNrInputPins: Number of Input Pins of this Jack.*/
  MIDI_JACK_25,           /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
  0x01,                   /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
  0x00,                   /*iJack: Unused.*/
#endif

#if MIDI_OUT_PORTS_NUM >= 6
  /******************** MIDI Adapter MIDI IN Jack Descriptor (Embedded) ********************/
  0x06,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x02,                   /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
  0x01,                   /*bJackType: EMBEDDED*/
  MIDI_JACK_27,           /*bJackID: ID of this Jack.*/
  0x00,                   /*iJack: Unused.*/

  /******************** MIDI Adapter MIDI OUT Jack Descriptor (External) ********************/
  0x09,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x03,                   /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
  0x02,                   /*bJackType: EXTERNAL.*/
  MIDI_JACK_28,           /*bJackID: ID of this Jack.*/
  0x01,                   /*bNrInputPins: Number of Input Pins of this Jack.*/
  MIDI_JACK_27,           /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
  0x01,                   /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
  0x00,                   /*iJack: Unused.*/
#endif

#if MIDI_OUT_PORTS_NUM >= 7
  /******************** MIDI Adapter MIDI IN Jack Descriptor (Embedded) ********************/
  0x06,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x02,                   /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
  0x01,                   /*bJackType: EMBEDDED*/
  MIDI_JACK_29,           /*bJackID: ID of this Jack.*/
  0x00,                   /*iJack: Unused.*/

  /******************** MIDI Adapter MIDI OUT Jack Descriptor (External) ********************/
  0x09,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x03,                   /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
  0x02,                   /*bJackType: EXTERNAL.*/
  MIDI_JACK_30,           /*bJackID: ID of this Jack.*/
  0x01,                   /*bNrInputPins: Number of Input Pins of this Jack.*/
  MIDI_JACK_29,           /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
  0x01,                   /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
  0x00,                   /*iJack: Unused.*/
#endif

#if MIDI_OUT_PORTS_NUM >= 8
  /******************** MIDI Adapter MIDI IN Jack Descriptor (Embedded) ********************/
  0x06,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x02,                   /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
  0x01,                   /*bJackType: EMBEDDED*/
  MIDI_JACK_31,           /*bJackID: ID of this Jack.*/
  0x00,                   /*iJack: Unused.*/

  /******************** MIDI Adapter MIDI OUT Jack Descriptor (External) ********************/
  0x09,                   /*bLength: Size of this descriptor, in bytes*/
  0x24,                   /*bDescriptorType: CS_INTERFACE descriptor.*/
  0x03,                   /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
  0x02,                   /*bJackType: EXTERNAL.*/
  MIDI_JACK_32,           /*bJackID: ID of this Jack.*/
  0x01,                   /*bNrInputPins: Number of Input Pins of this Jack.*/
  MIDI_JACK_31,           /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
  0x01,                   /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
  0x00,                   /*iJack: Unused.*/
#endif

// https://www.usb.org/sites/default/files/midi10.pdf Page 25 Table 6-6: Standard MS Bulk Data Endpoint Descriptor
  /******************** MIDI Adapter Standard Bulk OUT Endpoint Descriptor ********************/
  0x09,                   /*bLength: Size of this descriptor, in bytes*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType: ENDPOINT descriptor.*/
  MIDI_OUT_EP,            /*bEndpointAddress: OUT Endpoint 1.*/
  USBD_EP_TYPE_BULK,      /*bmAttributes: Bulk, not shared.*/
  USB_FS_MAX_PACKET_SIZE, 
  0x00,                   /*wMaxPacketSize*/
  0x00,                   /*bInterval: Ignored for Bulk. Set to zero.*/
  0x00,                   /*bRefresh: Unused.*/
  0x00,                   /*bSynchAddress: Unused.*/

// https://www.usb.org/sites/default/files/midi10.pdf Page 26 Table 6-7: Class-specific MS Bulk Data Endpoint Descriptor
  /******************** MIDI Adapter Class-specific Bulk OUT Endpoint Descriptor ********************/
  (4 + MIDI_OUT_PORTS_NUM), /*bLength: Size of this descriptor, in bytes*/
  0x25,                     /*bDescriptorType: CS_ENDPOINT descriptor*/
  0x01,                     /*bDescriptorSubtype: MS_GENERAL subtype.*/
  MIDI_OUT_PORTS_NUM,       /*bNumEmbMIDIJack: Number of embedded MIDI IN Jacks.*/
#if MIDI_OUT_PORTS_NUM >= 1
  MIDI_JACK_17,             /*BaAssocJackID(1): ID of the Embedded MIDI IN Jack.*/
#endif
#if MIDI_OUT_PORTS_NUM >= 2
  MIDI_JACK_19,             /*BaAssocJackID(2): ID of the Embedded MIDI IN Jack.*/
#endif
#if MIDI_OUT_PORTS_NUM >= 3
  MIDI_JACK_21,             /*BaAssocJackID(3): ID of the Embedded MIDI IN Jack.*/
#endif
#if MIDI_OUT_PORTS_NUM >= 4
  MIDI_JACK_23,             /*BaAssocJackID(4): ID of the Embedded MIDI IN Jack.*/
#endif
#if MIDI_OUT_PORTS_NUM >= 5
  MIDI_JACK_25,             /*BaAssocJackID(5): ID of the Embedded MIDI IN Jack.*/
#endif
#if MIDI_OUT_PORTS_NUM >= 6
  MIDI_JACK_27,             /*BaAssocJackID(6): ID of the Embedded MIDI IN Jack.*/
#endif
#if MIDI_OUT_PORTS_NUM >= 7
  MIDI_JACK_29,             /*BaAssocJackID(7): ID of the Embedded MIDI IN Jack.*/
#endif
#if MIDI_OUT_PORTS_NUM >= 8
  MIDI_JACK_31,             /*BaAssocJackID(8): ID of the Embedded MIDI IN Jack.*/
#endif

// https://www.usb.org/sites/default/files/midi10.pdf Page 25 Table 6-6: Standard MS Bulk Data Endpoint Descriptor
  /******************** MIDI Adapter Standard Bulk IN Endpoint Descriptor ********************/
  0x09,                    /*bLength: Size of this descriptor, in bytes*/
  USB_DESC_TYPE_ENDPOINT,  /*bDescriptorType: ENDPOINT descriptor.*/
  MIDI_IN_EP,              /*bEndpointAddress: IN Endpoint 1.*/
  0x02,                    /*bmAttributes: Bulk, not shared.*/
  USB_FS_MAX_PACKET_SIZE, 
  0x00,                    /*wMaxPacketSize*/
  0x00,                    /*bInterval: Ignored for Bulk. Set to zero.*/
  0x00,                    /*bRefresh: Unused.*/
  0x00,                    /*bSynchAddress: Unused.*/

// https://www.usb.org/sites/default/files/midi10.pdf Page 26 Table 6-7: Class-specific MS Bulk Data Endpoint Descriptor
  /******************** MIDI Adapter Class-specific Bulk IN Endpoint Descriptor ********************/
  (4 + MIDI_IN_PORTS_NUM), /*bLength: Size of this descriptor, in bytes*/
  0x25,                    /*bDescriptorType: CS_ENDPOINT descriptor*/
  0x01,                    /*bDescriptorSubtype: MS_GENERAL subtype.*/
  MIDI_IN_PORTS_NUM,       /*bNumEmbMIDIJack: Number of embedded MIDI OUT Jacks.*/
#if MIDI_IN_PORTS_NUM >= 1
  MIDI_JACK_2,             /*BaAssocJackID(1): ID of the Embedded MIDI OUT Jack.*/
#endif
#if MIDI_IN_PORTS_NUM >= 2
  MIDI_JACK_4,             /*BaAssocJackID(2): ID of the Embedded MIDI OUT Jack.*/
#endif
#if MIDI_IN_PORTS_NUM >= 3
  MIDI_JACK_6,             /*BaAssocJackID(3): ID of the Embedded MIDI OUT Jack.*/
#endif
#if MIDI_IN_PORTS_NUM >= 4
  MIDI_JACK_8,             /*BaAssocJackID(4): ID of the Embedded MIDI OUT Jack.*/
#endif
#if MIDI_IN_PORTS_NUM >= 5
  MIDI_JACK_10,            /*BaAssocJackID(5): ID of the Embedded MIDI OUT Jack.*/
#endif
#if MIDI_IN_PORTS_NUM >= 6
  MIDI_JACK_12,            /*BaAssocJackID(6): ID of the Embedded MIDI OUT Jack.*/
#endif
#if MIDI_IN_PORTS_NUM >= 7
  MIDI_JACK_14,            /*BaAssocJackID(7): ID of the Embedded MIDI OUT Jack.*/
#endif
#if MIDI_IN_PORTS_NUM >= 8
  MIDI_JACK_16,            /*BaAssocJackID(8): ID of the Embedded MIDI OUT Jack.*/
#endif
};
#endif /* USE_USBD_COMPOSITE  */

static uint8_t MIDIInEpAdd  = MIDI_IN_EP;
static uint8_t MIDIOutEpAdd = MIDI_OUT_EP;

/**
  * @}
  */

/** @defgroup USBD_MIDI_Private_Functions
  * @{
  */

/**
  * @brief  USBD_MIDI_Init
  *         Initialize the MIDI interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_MIDI_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  USBD_MIDI_HandleTypeDef *hmidi;

  //printf("MIDI INIT, class id = %ld\n", pdev->classId);

  /* Allocate Midi structure */
  hmidi = (USBD_MIDI_HandleTypeDef *)USBD_malloc(sizeof(USBD_MIDI_HandleTypeDef));

  if(hmidi == NULL)
  {
    pdev->pClassDataCmsit[pdev->classId] = NULL;
    return (uint8_t)USBD_EMEM;
  }

  (void)(USBD_memset(hmidi, 0, sizeof(USBD_MIDI_HandleTypeDef)));

  pdev->pClassDataCmsit[pdev->classId] = (void *)hmidi;
  pdev->pClassData = pdev->pClassDataCmsit[pdev->classId];

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  MIDIInEpAdd  = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
  MIDIOutEpAdd  = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  //printf("MIDI In EP: %02x, Out EP: %02x\n", MIDIInEpAdd, MIDIOutEpAdd);

  if (pdev->dev_speed == USBD_SPEED_HIGH)
  {
    pdev->ep_in[MIDIInEpAdd & 0xFU].bInterval = MIDI_HS_BINTERVAL;
    pdev->ep_out[MIDIOutEpAdd & 0xFU].bInterval = MIDI_HS_BINTERVAL;
  }
  else   /* LOW and FULL-speed endpoints */
  {
    // Open EP IN
    (void)USBD_LL_OpenEP(pdev, MIDIInEpAdd, USBD_EP_TYPE_BULK, USB_FS_MAX_PACKET_SIZE);
    pdev->ep_in[MIDIInEpAdd & 0xFU].is_used = 1U;

    /* Open EP OUT */
    (void)USBD_LL_OpenEP(pdev, MIDIOutEpAdd, USBD_EP_TYPE_BULK, USB_FS_MAX_PACKET_SIZE);
    pdev->ep_out[MIDIOutEpAdd & 0xFU].is_used = 1U;

    //pdev->ep_in[MIDIInEpAdd & 0xFU].bInterval = MIDI_FS_BINTERVAL;
    //pdev->ep_out[MIDIOutEpAdd & 0xFU].bInterval = MIDI_FS_BINTERVAL;
  }

  /* Initialize the Midi output Hardware layer */
  if(((USBD_MIDI_ItfTypeDef *)pdev->pUserData[pdev->classId])->Init(pdev, cfgidx))
  {
    return (uint8_t)USBD_FAIL;
  }

  /* Prepare Out endpoint to receive 1st packet */
  //(void)USBD_LL_PrepareReceive(pdev, MIDIOutEpAdd, hmidi->buffer,
  //                             MIDI_OUT_PACKET_SIZE);
  (void)USBD_LL_PrepareReceive(pdev, MIDIOutEpAdd, hmidi->rx_buffer, USB_FS_MAX_PACKET_SIZE);

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_MIDI_Init
  *         DeInitialize the MIDI layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_MIDI_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  
  //printf("MIDI DEINIT\n");

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  MIDIInEpAdd  = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
  MIDIOutEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  /* Close EP IN */
  (void)USBD_LL_CloseEP(pdev, MIDIInEpAdd);
  pdev->ep_in[MIDIInEpAdd & 0xFU].is_used = 0U;
  //pdev->ep_in[MIDIInEpAdd & 0xFU].bInterval = 0U;

  /* Close EP OUT */
  (void)USBD_LL_CloseEP(pdev, MIDIOutEpAdd);
  pdev->ep_out[MIDIOutEpAdd & 0xFU].is_used = 0U;
  //pdev->ep_out[MIDIOutEpAdd & 0xFU].bInterval = 0U;

  /* DeInit physical Interface components */
  if (pdev->pClassDataCmsit[pdev->classId] != NULL)
  {
    ((USBD_MIDI_ItfTypeDef *)pdev->pUserData[pdev->classId])->DeInit(pdev, cfgidx);
    (void)USBD_free(pdev->pClassDataCmsit[pdev->classId]);
    pdev->pClassDataCmsit[pdev->classId] = NULL;
    pdev->pClassData = NULL;
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_MIDI_Setup
  *         Handle the MIDI specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t USBD_MIDI_Setup(USBD_HandleTypeDef *pdev,
                                USBD_SetupReqTypedef *req)
{
  USBD_StatusTypeDef ret = USBD_OK;

  //printf("MIDI SETUP\n");

  return (uint8_t)ret;
}

#ifndef USE_USBD_COMPOSITE
/**
  * @brief  USBD_MIDI_GetCfgDesc
  *         return configuration descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_MIDI_GetCfgDesc(uint16_t *length)
{
  
  //printf("MIDI GET CFG DESC\n");

  USBD_EpDescTypeDef *pEpOutDesc = USBD_GetEpDesc(USBD_MIDI_CfgDesc, MIDI_OUT_EP);
  USBD_EpDescTypeDef *pEpInDesc = USBD_GetEpDesc(USBD_MIDI_CfgDesc, MIDI_IN_EP);

  if (pEpOutDesc != NULL)
  {
    pEpOutDesc->wMaxPacketSize = USB_FS_MAX_PACKET_SIZE;
  }

  if (pEpInDesc != NULL)
  {
    pEpInDesc->wMaxPacketSize = USB_FS_MAX_PACKET_SIZE;
  }

  *length = (uint16_t)sizeof(USBD_MIDI_CfgDesc);
  return USBD_MIDI_CfgDesc;
}
#endif /* USE_USBD_COMPOSITE  */

/**
  * @brief  USBD_MIDI_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_MIDI_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{

  USBD_MIDI_HandleTypeDef *hmidi;

  hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  if (hmidi == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  //printf("MIDI DATA IN, hmidi=%ld, midi busy = %d\n", (uint32_t)hmidi, hmidi->tx_busy);

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  MIDIInEpAdd  = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  /* Ensure that the FIFO is empty before a new transfer, this condition could 
     be caused by a new transfer before the end of the previous transfer */
  if(epnum == (MIDIInEpAdd & 0x7F)) // TODO: check why epnum does not come as 0x81 as expected
  {
    hmidi->tx_busy = 0;
    
    ((USBD_MIDI_ItfTypeDef *)pdev->pUserData[pdev->classId])->TransmitComplete(hmidi->tx_buffer, hmidi->tx_length, epnum);
    //printf("MIDI DATA IN, hmidi=%ld, midi busy = %d\n", (uint32_t)hmidi, hmidi->tx_busy);
  }

  return (uint8_t)USBD_OK;
}

///**
//  * @brief  USBD_MIDI_EP0_RxReady
//  *         handle EP0 Rx Ready event
//  * @param  pdev: device instance
//  * @retval status
//  */
//static uint8_t USBD_MIDI_EP0_RxReady (USBD_HandleTypeDef *pdev)
//{
//  
//  printf("MIDI EP0 RX READY\n");
//
//  return USBD_OK;
//}
///**
//  * @brief  USBD_MIDI_EP0_TxReady
//  *         handle EP0 TRx Ready event
//  * @param  pdev: device instance
//  * @retval status
//  */
//static uint8_t  USBD_MIDI_EP0_TxReady (USBD_HandleTypeDef *pdev)
//{
//  
//  printf("MIDI EP0 TX READY\n");
//
//  /* Only OUT control data are processed */
//  return USBD_OK;
//}

/**
  * @brief  USBD_MIDI_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_MIDI_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_MIDI_HandleTypeDef *hmidi;
  
  //printf("MIDI DATA OUT\n");

  hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  if(hmidi == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }
  
#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  MIDIOutEpAdd  = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  if(epnum == MIDIOutEpAdd)
  {
    // Get data length and actual data
    size_t length = USBD_LL_GetRxDataSize(pdev, epnum);
    uint8_t *rx_buffer = hmidi->rx_buffer;

    USBD_MIDI_ItfTypeDef* fops = ((USBD_MIDI_ItfTypeDef *)pdev->pUserData[pdev->classId]);
    if(fops != NULL) {
      /* Pass data to Receive() */
      for(uint32_t i = 0; i < length; i += 4) {
          //((USBD_MIDI_ItfTypeDef *)pdev->pUserData)->Receive(rx_buffer + i, length);
          fops->Receive((uint8_t*)(rx_buffer + i), length);
      }
    }

    USBD_LL_PrepareReceive(pdev, MIDIOutEpAdd, hmidi->rx_buffer, USB_FS_MAX_PACKET_SIZE); 
  }

  return (uint8_t)USBD_OK;
}

#ifndef USE_USBD_COMPOSITE
/**
  * @brief  DeviceQualifierDescriptor
  *         return Device Qualifier descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_MIDI_GetDeviceQualifierDesc(uint16_t *length)
{
  // printf("MIDI GET DEVICE QUALIFIER\n");

  *length = (uint16_t)sizeof(USBD_MIDI_DeviceQualifierDesc);
  return USBD_MIDI_DeviceQualifierDesc;
}
#endif /* USE_USBD_COMPOSITE  */

/**
  * @brief  USBD_MIDI_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: Midi interface callback
  * @retval status
  */
uint8_t USBD_MIDI_RegisterInterface(USBD_HandleTypeDef *pdev,
                                     USBD_MIDI_ItfTypeDef *fops)
{
  
  //printf("MIDI REGISTER INTERFACE\n");

  if (fops == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  pdev->pUserData[pdev->classId] = fops;

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_MIDI_SetTxBuffer
  * @param  pdev: instance
  * @param  buffer
  * @retval status
  */
#ifdef USE_USBD_COMPOSITE
uint8_t USBD_MIDI_SetTxBuffer(USBD_HandleTypeDef *pdev,
                              uint8_t *buff,
                              uint16_t length,
                              uint8_t ClassId)
{
  USBD_MIDI_HandleTypeDef *hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassDataCmsit[ClassId];
#else
uint8_t USBD_MIDI_SetTxBuffer(USBD_HandleTypeDef *pdev,
                              uint8_t *buff,
                              uint16_t length)
{
  USBD_MIDI_HandleTypeDef *hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
#endif

  //printf("MIDI SET TX BUFFER, *hmidi=%ld\n", (uint32_t)hmidi);

  if (hmidi == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }
  
//#ifdef USE_USBD_COMPOSITE
//  /* Get the Endpoints addresses allocated for this class instance */
//  MIDIOutEpAdd  = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_BULK, ClassId);
//#endif /* USE_USBD_COMPOSITE */

  //hmidi->tx_buffer = buff;
  memcpy(hmidi->tx_buffer, buff, length);
  hmidi->tx_length = length;

  return USBD_OK;
}

/**
  * @brief  USBD_MIDI_TransmitPacket
  *         Transmit packet on IN endpoint
  * @param  pdev: device instance
  * @retval status
  */
#ifdef USE_USBD_COMPOSITE
uint8_t USBD_MIDI_TransmitPacket(USBD_HandleTypeDef *pdev, uint8_t ClassId)
{
  USBD_MIDI_HandleTypeDef *hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassDataCmsit[ClassId];
#else
uint8_t USBD_MIDI_TransmitPacket(USBD_HandleTypeDef *pdev)
{
  USBD_MIDI_HandleTypeDef *hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
#endif
  USBD_StatusTypeDef ret = USBD_BUSY;

  //printf("MIDI TRANSMIT PACKET, *hmidi=%ld\n", (uint32_t)hmidi);

  if (hmidi == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  MIDIInEpAdd  = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_BULK, ClassId);
#endif /* USE_USBD_COMPOSITE */

  if(hmidi->tx_busy == 0)
  {
    /* Tx Transfer in progress */
    hmidi->tx_busy = 1;

    /* Update the packet total length */
    pdev->ep_in[MIDIInEpAdd & 0xFU].total_length = hmidi->tx_length;

    (void)USBD_LL_Transmit(pdev, MIDIInEpAdd, hmidi->tx_buffer, hmidi->tx_length);

    ret = USBD_OK;
  }

  return (uint8_t)ret;
}

/**
  * @}
  */


/**
  * @}
  */


/**
  * @}
  */
