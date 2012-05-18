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

typedef struct _USBEndpoint
{
	volatile uint8_t status;
	volatile uint8_t ctrl;
	volatile uint16_t cnt;
	volatile void *dataBufferP;
	volatile uint16_t auxData;
	
} USBEndpoint_t;

typedef struct _USBEndpointConfiguration
{
	uint8_t type;
	uint8_t bufferSize;
	uint8_t bufferType;
} USBEndpointConfiguration_t;

typedef struct _USBEndpointTableConfiguration
{
	uint8_t endpointCount;
	uint8_t maxPacketSize;
	USBEndpointConfiguration_t endpointConfiguration[1];
} USBEndpointTableConfiguration_t;

typedef struct _USBEndpointTable
{
	/*
		pointer to block of memory used for FIFO and the endpoint table. This should never be used directly
		except to free it.
	 */
	void *baseP;
	
	/*
		pointer to the block of memory used as the endpoint FIFO. Note that it will never to be used in code,
		it just serves as a reminder that the USB module expects this memory to be allocated.  
	 */ 
	void *fifoP;
	
	/* 
		pointer to the endpoint table memory used to hold the USB endpoint configuration etc.
	 */
	USBEndpoint_t *usbEndpointP;
	
	const USBEndpointTableConfiguration_t const *usbEndpointTableConfigurationP;

} USBEndpointTable_t;

typedef enum _EndpointDirection
{
	OUT,
	IN
} EndpointDirection;


bool_t USBEndpointTableAlloc(const USBEndpointTableConfiguration_t const *usbEndpointTableConfigurationP);

USBEndpointTable_t* USBEndpointTableGet(void);
USBEndpoint_t* USBEndpointGet(uint8_t endpointNumber, EndpointDirection endpointDirection);
USBEndpoint_t* USBEndpointGetDefault(EndpointDirection endpointDirection);
USBEndpoint_t* USBEndpointTxQueueGetNext(void);

void USBEndpointReset(USBEndpoint_t *usbEndpointP, const USBEndpointConfiguration_t const *usbEndpointConfigurationP);
void USBEndpointResetAll(void);

#endif /* USBEP_H_ */