/***********************************************************************************************************************
 * 
 * > QuadroCore <
 * 
 * Copyright (C) 2012 by Chris Channing
 *
 ***********************************************************************************************************************
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 ***********************************************************************************************************************/

#ifndef USBEP_H_
#define USBEP_H_

typedef enum _USBEndpointType
{
	USB_ENDPOINT_TYPE_CONTROL = USB_EP_TYPE_CONTROL_gc,
	USB_ENDPOINT_TYPE_INTERRUPT = USB_EP_TYPE_BULK_gc
} USBEndpointType_t;

typedef struct _USBEndpointPipe
{
	volatile uint8_t status;
	volatile uint8_t ctrl;
	volatile uint16_t cnt;
	volatile void *dataBufferP;
	volatile uint16_t auxData;
	
} USBEndpointPipe_t;

typedef struct _USBEndpointConfiguration
{
	uint8_t type;
	uint8_t bufferSize;
	uint8_t bufferType;
	uint8_t maxPacketSize;
} USBEndpointConfiguration_t;

typedef struct _USBEndpoint
{
	uint8_t endpointNumber;
	const USBEndpointConfiguration_t *usbEndpointConfigurationP;
	USBEndpointType_t endpointType;
	USBEndpointPipe_t *usbEndpointOutPipeP;
	USBEndpointPipe_t *usbEndpointInPipeP;
} USBEndpoint_t;

typedef struct _USBEndpointTableConfiguration
{
	uint8_t endpointCount;
	uint8_t maxPacketSize;
	USBEndpointConfiguration_t endpointConfiguration[2];
} USBEndpointTableConfiguration_t;

typedef struct _USBEndpointTable
{
	/*
		pointer to block of memory used for FIFO and the endpoint table (which are really pipes). This should never be used directly
		except to free it.
	 */
	void *blockP;
	
	/*
		pointer to the vector holding each endpoint
	 */
	Vector_t *usbEndpointListP;
	
	const USBEndpointTableConfiguration_t const *usbEndpointTableConfigurationP;

} USBEndpointTable_t;

bool_t USBEndpointTableInit(const USBEndpointTableConfiguration_t const *usbEndpointTableConfigurationP);
uint16_t USBEndpointTableGetBaseAddress(void);

USBEndpoint_t* USBEndpointGetByPipe(USBEndpointPipe_t *usbEndpointPipeP);
USBEndpoint_t* USBEndpointGetByNumber(uint8_t endpointNumber);
USBEndpoint_t* USBEndpointGetDefault(void);
USBEndpoint_t* USBEndpointTxQueueGetNext(void);

void USBEndpointReset(USBEndpoint_t *usbEndpointP);
void USBEndpointResetAll(void);

#endif /* USBEP_H_ */