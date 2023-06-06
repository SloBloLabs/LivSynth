/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_midi_if.c
  * @version        : v1.0_Cube
  * @brief          : Generic media access layer.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
 /* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_midi_if.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_MIDI_IF
  * @{
  */

/** @defgroup USBD_MIDI_IF_Private_TypesDefinitions USBD_MIDI_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_MIDI_IF_Private_Defines USBD_MIDI_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */

/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_MIDI_IF_Private_Macros USBD_MIDI_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_MIDI_IF_Private_Variables USBD_MIDI_IF_Private_Variables
  * @brief Private variables.
  * @{
  */

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_MIDI_IF_Exported_Variables USBD_MIDI_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;
extern uint32_t USBD_CMPSIT_GetClassID(USBD_HandleTypeDef *pdev, USBD_CompositeClassTypeDef Class, uint32_t Instance);

extern void enqueueIncomingMidi(uint8_t *data);
extern void midiTrxSentCallback();

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_MIDI_IF_Private_FunctionPrototypes USBD_MIDI_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t MIDI_Init_FS(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static int8_t MIDI_DeInit_FS(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static int8_t MIDI_Receive_FS(uint8_t* pbuf, uint32_t length);
static int8_t MIDI_Send_FS(uint8_t* pbuf, uint32_t length);
static void   MIDI_TrxComplete_FS(uint8_t* pbuf, uint32_t length, uint8_t epnum);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_MIDI_ItfTypeDef USBD_MIDI_fops_FS =
{
  MIDI_Init_FS,
  MIDI_DeInit_FS,
  MIDI_Receive_FS,
  MIDI_Send_FS,
  MIDI_TrxComplete_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the MIDI media low layer over USB FS IP
  * @param  MidiFreq: Midi frequency used to play the midi stream.
  * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
  * @param  options: Reserved for future use
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t MIDI_Init_FS(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  /* USER CODE BEGIN 0 */
  //printf("Midi IF Init\n");
  return (USBD_OK);
  /* USER CODE END 0 */
}

/**
  * @brief  De-Initializes the MIDI media low layer
  * @param  options: Reserved for future use
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t MIDI_DeInit_FS(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  /* USER CODE BEGIN 1 */
  //printf("Midi IF DeInit\n");
  return (USBD_OK);
  /* USER CODE END 1 */
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t MIDI_Receive_FS(uint8_t* Buf, uint32_t Len)
{
  /* USER CODE BEGIN 6 */
  
  //uint8_t chan = Buf[1] & 0xf;
  //uint8_t msgtype = Buf[1] & 0xf0;
  //uint8_t b1 =  Buf[2];
  //uint8_t b2 =  Buf[3];
  //uint16_t b = ((b2 & 0x7f) << 7) | (b1 & 0x7f);
  //UNUSED(b);
  //printf("MIDI_Receive_FS: chan = 0x%02x, msgtype = 0x%02x, b1 = 0x%02x, b2 = 0x%02x\n", chan, msgtype, b1, b2);

  enqueueIncomingMidi(Buf);

  return (USBD_OK);
  /* USER CODE END 6 */
}

/**
  * @brief  MIDI_Send
  *
  * @param  buffer: bufferfer of data to be received
  * @param  length: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t MIDI_Send_FS(uint8_t* buffer, uint32_t length)
{
  uint8_t ret = USBD_OK;
  
  //uint8_t cable = buffer[0];
  //uint8_t message = buffer[1];
  //uint8_t param1 = buffer[2];
  //uint8_t param2 = buffer[3];
  //printf("MIDI_Send_FS: chan = 0x%02x, msgtype = 0x%02x, b1 = 0x%02x, b2 = 0x%02x\n", cable, message, param1, param2);
#ifdef USE_USBD_COMPOSITE
  uint32_t classId = USBD_CMPSIT_GetClassID(&hUsbDeviceFS, CLASS_TYPE_AUDIO, 0);

  USBD_MIDI_SetTxBuffer(&hUsbDeviceFS, buffer, length, classId);

  ret = USBD_MIDI_TransmitPacket(&hUsbDeviceFS, classId);
#else
  USBD_MIDI_SetTxBuffer(&hUsbDeviceFS, buffer, length);

  ret = USBD_MIDI_TransmitPacket(&hUsbDeviceFS);
#endif /* USE_USBD_COMPOSITE */
  return (ret);
}

static void MIDI_TrxComplete_FS(uint8_t* pbuf, uint32_t length, uint8_t epnum) {
  //printf("TRX complete, len = %ld\n", length);
  midiTrxSentCallback();
}

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */
