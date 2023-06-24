/**
  ******************************************************************************
  * @file    usbd_midi.h
  * @author  MCD Application Team
  * @brief   header file for the usbd_midi.c file.
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
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_MIDI_H
#define __USB_MIDI_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_ioreq.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_MIDI
  * @brief This file is the Header file for usbd_midi.c
  * @{
  */


/** @defgroup USBD_MIDI_Exported_Defines
  * @{
  */

#define MIDI_IN_PORTS_NUM              0x01
#define MIDI_OUT_PORTS_NUM             0x01

#define USB_MIDI_CLASS_DESC_SHIFT      18
#define USB_MIDI_DESC_SIZE             7
#define USB_MIDI_REPORT_DESC_SIZE      (MIDI_IN_PORTS_NUM * 16 + MIDI_OUT_PORTS_NUM * 16 + 33)
#define USB_MIDI_CONFIG_DESC_SIZE      (USB_MIDI_REPORT_DESC_SIZE + USB_MIDI_CLASS_DESC_SHIFT)

#define MIDI_DESCRIPTOR_TYPE           0x21
  
#define MIDI_REQ_SET_PROTOCOL          0x0B
#define MIDI_REQ_GET_PROTOCOL          0x03

#define MIDI_REQ_SET_IDLE              0x0A
#define MIDI_REQ_GET_IDLE              0x02

#define MIDI_REQ_SET_REPORT            0x09
#define MIDI_REQ_GET_REPORT            0x01

#define MIDI_JACK_1    0x01
#define MIDI_JACK_2    0x02
#define MIDI_JACK_3    0x03
#define MIDI_JACK_4    0x04
#define MIDI_JACK_5    0x05
#define MIDI_JACK_6    0x06
#define MIDI_JACK_7    0x07
#define MIDI_JACK_8    0x08
#define MIDI_JACK_9    0x09
#define MIDI_JACK_10    0x0a
#define MIDI_JACK_11    0x0b
#define MIDI_JACK_12    0x0c
#define MIDI_JACK_13    0x0d
#define MIDI_JACK_14    0x0e
#define MIDI_JACK_15    0x0f
#define MIDI_JACK_16    0x10
#define MIDI_JACK_17    0x11
#define MIDI_JACK_18    0x12
#define MIDI_JACK_19    0x13
#define MIDI_JACK_20    0x14
#define MIDI_JACK_21    0x15
#define MIDI_JACK_22    0x16
#define MIDI_JACK_23    0x17
#define MIDI_JACK_24    0x18
#define MIDI_JACK_25    0x19
#define MIDI_JACK_26    0x1a
#define MIDI_JACK_27    0x1b
#define MIDI_JACK_28    0x1c
#define MIDI_JACK_29    0x1d
#define MIDI_JACK_30    0x1e
#define MIDI_JACK_31    0x1f
#define MIDI_JACK_32    0x20
#define MIDI_JACK_33    0x21
#define MIDI_JACK_34    0x22
#define MIDI_JACK_35    0x23
#define MIDI_JACK_36    0x24
#define MIDI_JACK_37    0x25
#define MIDI_JACK_38    0x26
#define MIDI_JACK_39    0x27
#define MIDI_JACK_40    0x28
#define MIDI_JACK_41    0x29
#define MIDI_JACK_42    0x2a
#define MIDI_JACK_43    0x2b
#define MIDI_JACK_44    0x2c
#define MIDI_JACK_45    0x2d
#define MIDI_JACK_46    0x2e
#define MIDI_JACK_47    0x2f
#define MIDI_JACK_48    0x30
#define MIDI_JACK_49    0x31
#define MIDI_JACK_50    0x32
#define MIDI_JACK_51    0x33
#define MIDI_JACK_52    0x34
#define MIDI_JACK_53    0x35
#define MIDI_JACK_54    0x36
#define MIDI_JACK_55    0x37
#define MIDI_JACK_56    0x38
#define MIDI_JACK_57    0x39
#define MIDI_JACK_58    0x3a
#define MIDI_JACK_59    0x3b
#define MIDI_JACK_60    0x3c
#define MIDI_JACK_61    0x3d
#define MIDI_JACK_62    0x3e
#define MIDI_JACK_63    0x3f
#define MIDI_JACK_64    0x40

#ifndef MIDI_HS_BINTERVAL
#define MIDI_HS_BINTERVAL                            0x01U
#endif /* MIDI_HS_BINTERVAL */

#ifndef MIDI_FS_BINTERVAL
#define MIDI_FS_BINTERVAL                            0x01U
#endif /* MIDI_FS_BINTERVAL */

#ifndef MIDI_IN_EP
#define MIDI_IN_EP                                  0x81U
#endif /* MIDI_IN_EP */

#ifndef MIDI_OUT_EP
#define MIDI_OUT_EP                                  0x01U
#endif /* MIDI_OUT_EP */

#define USB_DEVICE_CLASS_AUDIO                       0x01U
#define AUDIO_SUBCLASS_MIDISTREAMING                 0x03U
#define AUDIO_PROTOCOL_UNDEFINED                     0x00U
#define AUDIO_HEADER_MIDISTREAMING                   0x01U

#define JACK_TYPE_EMBEDDED                           0x01U
#define JACK_TYPE_EXTERNAL                           0x02U

#define MIDI_IN_JACK                                 0x02U
#define MIDI_OUT_JACK                                0x03U
#define CS_INTERFACE                                 0x24U
#define CS_ENDPOINT                                  0x25U
#define EP_MIDISTREAMING_GENERAL                     0x01U

/**
  * @}
  */


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */

typedef struct
{
  uint8_t rx_buffer[USB_FS_MAX_PACKET_SIZE];
  uint8_t tx_buffer[USB_FS_MAX_PACKET_SIZE];
  uint8_t tx_busy;
  uint8_t tx_length;
} USBD_MIDI_HandleTypeDef;

typedef struct
{
  int8_t (*Init)    (USBD_HandleTypeDef *pdev, uint8_t cfgidx);
  int8_t (*DeInit)  (USBD_HandleTypeDef *pdev, uint8_t cfgidx);
  int8_t (*Receive) (uint8_t* pbuf, uint32_t size);
  int8_t (*Send)    (uint8_t* pbuf, uint32_t size);
  void   (*TransmitComplete)(uint8_t *Buf, uint32_t Len, uint8_t epnum);
} USBD_MIDI_ItfTypeDef;


/*
 * MIDI Class specification release 1.0
 * https://www.usb.org/sites/default/files/midi10.pdf
 */

// Table 6-2: Class-Specific MS Interface Header Descriptor
typedef struct
{
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;
  uint16_t bcdMSC;
  uint16_t wTotalLength;
} __PACKED USBD_MidiIfHeaderDesc;

// Table 6-3: MIDI IN Jack Descriptor
typedef struct
{
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;
  uint8_t bJackType;
  uint8_t bJackID;
  uint8_t iJack;
} __PACKED USBD_MidiInJackDesc;

// Table 6-4: MIDI OUT Jack Descriptor
typedef struct
{
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;
  uint8_t bJackType;
  uint8_t bJackID;
  uint8_t bNrInputPins;
  uint8_t baSourceID1;
  uint8_t BaSourcePin1;
  uint8_t iJack;
} __PACKED USBD_MidiOutJackDesc;

// Table 6-6: Standard MidiStreaming Bulk Data Endpoint Descriptor
typedef struct
{
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint8_t  bEndpointAddress;
  uint8_t  bmAttributes;
  uint16_t wMaxPacketSize;
  uint8_t  bInterval;
  uint8_t  bRefresh;
  uint8_t  bSynchAddress;
} __PACKED USBD_MidiStreamingStandartEPDesc;

// Table 6-7: Class-specific MidiStreaming Bulk Data Endpoint Descriptor
typedef struct
{
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubType;
  uint8_t bNumEmbMIDIJack;
  uint8_t baAssocJackID1;
} __PACKED USBD_MidiStreamingClassEPDesc;


/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */

extern USBD_ClassTypeDef USBD_MIDI;
#define USBD_MIDI_CLASS &USBD_MIDI

/**
  * @}
  */

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */
uint8_t USBD_MIDI_RegisterInterface(USBD_HandleTypeDef *pdev,
                                     USBD_MIDI_ItfTypeDef *fops);

#ifdef USE_USBD_COMPOSITE
uint8_t USBD_MIDI_SetTxBuffer(USBD_HandleTypeDef *pdev,
                              uint8_t  *buff,
                              uint16_t length,
                              uint8_t ClassId);
uint8_t USBD_MIDI_TransmitPacket(USBD_HandleTypeDef *pdev, uint8_t ClassId);
#else
uint8_t USBD_MIDI_SetTxBuffer(USBD_HandleTypeDef *pdev,
                              uint8_t  *buff,
                              uint16_t length);
uint8_t USBD_MIDI_TransmitPacket(USBD_HandleTypeDef *pdev);
#endif /* USE_USBD_COMPOSITE */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif  /* __USB_MIDI_H */
/**
  * @}
  */

/**
  * @}
  */
