/*
 * Copyright (C) 2015 BlueKitchen GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. Any redistribution, use, or modification is done solely for
 *    personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MATTHIAS
 * RINGWALD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Please inquire about commercial licensing options at 
 * contact@bluekitchen-gmbh.com
 *
 */

/*
 *  hci_h4_transport_wiced.c
 *
 *  HCI Transport API implementation for basic H4 protocol for use with run_loop_wiced.c
 */

#include "btstack-config.h"
#include "run_loop_wiced.h"

#include "source/debug.h"
#include "source/hci.h"
#include "source/hci_transport.h"
//#include "source/platform_bluetooth.h"
//#include "wiced.h"

#include <stdio.h>
#include <string.h>

#include "hci_usart_hal.h"
#include "gpio_hal.h"
#include "delay_hal.h"

// priority higher than WIFI to make sure RTS is set
#define WICED_BT_UART_THREAD_PRIORITY        (WICED_NETWORK_WORKER_PRIORITY - 2)
#define WICED_BT_UART_THREAD_STACK_SIZE      300

// assert pre-buffer for packet type is available
#if !defined(HCI_OUTGOING_PRE_BUFFER_SIZE) || (HCI_OUTGOING_PRE_BUFFER_SIZE == 0)
#error HCI_OUTGOING_PRE_BUFFER_SIZE not defined. Please update hci.h
#endif

/* Default of 512 bytes should be fine. Only needed if WICED_BT_UART_MANUAL_CTS_RTS */
#ifndef RX_RING_BUFFER_SIZE
#define RX_RING_BUFFER_SIZE 512
#endif

#define  PACKET_RECEIVE_TIMEOUT  0x0000FFFF

static void dummy_handler(uint8_t packet_type, uint8_t *packet, uint16_t size); 

typedef struct hci_transport_h4 {
    hci_transport_t transport;
    data_source_t *ds;
    int uart_fd;    // different from ds->fd for HCI reader thread
    /* power management support, e.g. used by iOS */
    timer_source_t sleep_timer;
} hci_transport_h4_t;

typedef enum {
  PKT_TYPE = 1,
  PKT_HEAD_ACL,
  PKT_HEAD_EVENT,
  PKT_PAYLOAD,
  PKT_FINISHED
}pkt_status_t;

// single instance
static hci_transport_h4_t * hci_transport_h4 = NULL;

static void (*packet_handler)(uint8_t packet_type, uint8_t *packet, uint16_t size) = dummy_handler;

static const uint8_t *       tx_worker_data_buffer;
static uint16_t              tx_worker_data_size;

static int                   rx_worker_read_pos;

static uint8_t hci_packet_with_pre_buffer[HCI_INCOMING_PRE_BUFFER_SIZE + 1 + HCI_PACKET_BUFFER_SIZE]; // packet type + max(acl header + acl payload, event header + event data)
static uint8_t * hci_packet = &hci_packet_with_pre_buffer[HCI_INCOMING_PRE_BUFFER_SIZE];

HCI_USART_Ring_Buffer   ring_tx_buffer;
HCI_USART_Ring_Buffer   ring_rx_buffer;

static pkt_status_t     pkt_status = PKT_TYPE;
static uint8_t          packet_offset;
static uint16_t         packet_payload_len;
static uint32_t         timeout;
static uint8_t          send_packet_finish;

void message_handle(void)
{
    if(send_packet_finish)
    {
        send_packet_finish = 0;
        tx_worker_data_size = 0;
        //notify upper stack that it might be possible to send again
        uint8_t event[] = { DAEMON_EVENT_HCI_PACKET_SENT, 0};
        packet_handler(HCI_EVENT_PACKET, &event[0], sizeof(event));
    }
    while(HAL_HCI_USART_Available_Data(HAL_HCI_USART_SERIAL6))
	{
		if(pkt_status == PKT_TYPE){
			packet_offset = 0;
			packet_payload_len = 0;
			if(HAL_HCI_USART_Available_Data(HAL_HCI_USART_SERIAL6)){
				hci_packet[0] = HAL_HCI_USART_Read_Data(HAL_HCI_USART_SERIAL6);
                packet_offset++;
				if(hci_packet[0] == HCI_ACL_DATA_PACKET)
                    pkt_status = PKT_HEAD_ACL;
                else if(hci_packet[0] == HCI_EVENT_PACKET)
                    pkt_status = PKT_HEAD_EVENT;
                else {
                    // Invalid packet type.
                    pkt_status = PKT_TYPE;
                    packet_offset = 0;
                }
			}
		}
		if(pkt_status == PKT_HEAD_ACL) {
			// Wait for received ACL HEADER.
			if(HAL_HCI_USART_Available_Data(HAL_HCI_USART_SERIAL6) >= HCI_ACL_HEADER_SIZE) {
				uint8_t index;
				for(index=0; index<HCI_ACL_HEADER_SIZE; index++) {
					hci_packet[packet_offset] = HAL_HCI_USART_Read_Data(HAL_HCI_USART_SERIAL6);
					packet_offset++;
				}

				packet_payload_len = (hci_packet[packet_offset-1]<<8) + hci_packet[packet_offset-2];
				pkt_status = PKT_PAYLOAD;
				timeout = 0;
			}
			else {
				timeout++;
				if(timeout >= PACKET_RECEIVE_TIMEOUT){
					while(HAL_HCI_USART_Available_Data(HAL_HCI_USART_SERIAL6))
						HAL_HCI_USART_Read_Data(HAL_HCI_USART_SERIAL6);
					pkt_status=PKT_TYPE;
				}
			}
		}
		if(pkt_status == PKT_HEAD_EVENT) {
			if(HAL_HCI_USART_Available_Data(HAL_HCI_USART_SERIAL6) >= HCI_EVENT_HEADER_SIZE) {
				uint8_t index;
				for(index=0; index<HCI_EVENT_HEADER_SIZE; index++) {
					hci_packet[packet_offset] = HAL_HCI_USART_Read_Data(HAL_HCI_USART_SERIAL6);
					packet_offset++;
				}

				packet_payload_len = hci_packet[packet_offset-1];
				pkt_status = PKT_PAYLOAD;
				timeout = 0;
			}
			else {
				timeout++;
				if(timeout >= PACKET_RECEIVE_TIMEOUT){
					while(HAL_HCI_USART_Available_Data(HAL_HCI_USART_SERIAL6))
						HAL_HCI_USART_Read_Data(HAL_HCI_USART_SERIAL6);
					pkt_status=PKT_TYPE;
				}
			}
		}

		if(pkt_status == PKT_PAYLOAD) {
			if(HAL_HCI_USART_Available_Data(HAL_HCI_USART_SERIAL6) >= packet_payload_len){
				uint8_t index;
				for(index=0; index<packet_payload_len; index++) {
					hci_packet[packet_offset] = HAL_HCI_USART_Read_Data(HAL_HCI_USART_SERIAL6);
					packet_offset++;
				}
				pkt_status = PKT_FINISHED;
				timeout = 0;
			}
			else {
				timeout++;
				if(timeout >= PACKET_RECEIVE_TIMEOUT){
					while(HAL_HCI_USART_Available_Data(HAL_HCI_USART_SERIAL6))
						HAL_HCI_USART_Read_Data(HAL_HCI_USART_SERIAL6);
					pkt_status=PKT_TYPE;
				}
			}
		}
        if(pkt_status == PKT_FINISHED) {
            // When handle packet, set RTS HIGH.
        	HAL_GPIO_Write(BT_RTS, 1);
        	//log_info("PACEKT: 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x ", hci_packet[0],hci_packet[1],hci_packet[2],hci_packet[3],hci_packet[4],hci_packet[5]);
            
        	packet_handler(hci_packet[0], &hci_packet[1], (packet_offset-1));
            
            //log_info("packet handler over");
            // Start a new packet received.
            pkt_status = PKT_TYPE;
            packet_offset = 0;
            // After handle, set RTS LOW.
            HAL_GPIO_Write(BT_RTS, 0);
        }
	}
    HAL_HCI_USART_RestartSend(HAL_HCI_USART_SERIAL6);
}

static int h4_set_baudrate(uint32_t baudrate){
    log_info("h4_set_baudrate");
    HAL_GPIO_Write(BT_RTS, 1);
    HAL_Delay_Milliseconds(5);
    
    //HAL_HCI_USART_Set_BaudRate(HAL_HCI_USART_SERIAL6, baudrate);
    
    // Wait for CTS is LOW.
    HAL_GPIO_Write(BT_RTS, 0);
    HAL_Delay_Milliseconds(1);
    while(HAL_GPIO_Read(BT_CTS) == 1)
    {
        HAL_Delay_Milliseconds(1);
    }
    return 0;
}

static int h4_open(void *transport_config){
    HAL_HCI_USART_Init(HAL_HCI_USART_SERIAL6, &ring_rx_buffer, &ring_tx_buffer);
    // configure HOST and DEVICE WAKE PINs
    HAL_Pin_Mode(BT_HOST_WK, INPUT_PULLUP);
    HAL_Pin_Mode(BT_DEVICE_WK, OUTPUT);
    HAL_GPIO_Write(BT_DEVICE_WK, 0);
    HAL_Delay_Milliseconds(100);
    
#ifdef WICED_BT_UART_MANUAL_CTS_RTS    
    // configure RTS pin as output and set to high
    HAL_Pin_Mode(BT_RTS, OUTPUT);
    HAL_GPIO_Write(BT_RTS, 1);
    // configure CTS to input, pull-up
    HAL_Pin_Mode(BT_CTS, INPUT_PULLUP);
#endif

    HAL_HCI_USART_Begin(HAL_HCI_USART_SERIAL6, 115200);
    // reset Bluetooth
    HAL_Pin_Mode(BT_POWER, OUTPUT);
    HAL_GPIO_Write(BT_POWER, 0);
    HAL_Delay_Milliseconds(50);
    HAL_GPIO_Write(BT_POWER, 1);
    
    run_message_handler_register( message_handle );
    
    pkt_status = PKT_TYPE;
    // tx is ready
    send_packet_finish = 0;
    tx_worker_data_size = 0;
    
    // Wait for CTS is LOW.
    HAL_GPIO_Write(BT_RTS, 0);
    HAL_Delay_Milliseconds(1);
    while(HAL_GPIO_Read(BT_CTS) == 1)
    {
        HAL_Delay_Milliseconds(1);
    }
    
    return 0;
}

static int h4_close(void *transport_config){
    // not implementd
    return 0;
}

static int h4_send_packet(uint8_t packet_type, uint8_t * data, int size){
    
    // After handle, set RTS LOW.
    HAL_GPIO_Write(BT_RTS, 0);
    while(HAL_GPIO_Read(BT_CTS) == 1)
    {
        HAL_Delay_Milliseconds(1);
    }
    // store packet type before actual data and increase size
    size++;
    data--;
    *data = packet_type;

    // store in request
    tx_worker_data_buffer = data;
    tx_worker_data_size = size;
    // send packet as single block
    //wiced_rtos_send_asynchronous_event(&tx_worker_thread, &h4_tx_worker_send_packet, NULL);    
    //log_info("data: 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x ", tx_worker_data_buffer[0],tx_worker_data_buffer[1],tx_worker_data_buffer[2],tx_worker_data_buffer[3],tx_worker_data_buffer[4],tx_worker_data_buffer[5]);
    HAL_HCI_USART_Write_Buffer(HAL_HCI_USART_SERIAL6, tx_worker_data_buffer, tx_worker_data_size);
    //log_info("data finish");
    //HAL_Delay_Milliseconds(10);
    //tx_worker_data_size = 0;
    send_packet_finish = 1;
    // notify upper stack that it might be possible to send again
	//uint8_t event[] = { DAEMON_EVENT_HCI_PACKET_SENT, 0};
	//packet_handler(HCI_EVENT_PACKET, &event[0], sizeof(event));
    
    return 0;
}

static void h4_register_packet_handler(void (*handler)(uint8_t packet_type, uint8_t *packet, uint16_t size)){
    packet_handler = handler;
}

static int h4_can_send_packet_now(uint8_t packet_type){
    return tx_worker_data_size == 0;
}

static const char * h4_get_transport_name(void){
    return "H4";
}

static void dummy_handler(uint8_t packet_type, uint8_t *packet, uint16_t size){
}

// get h4 singleton
hci_transport_t * hci_transport_h4_wiced_instance() {
    if (hci_transport_h4 == NULL) {
        hci_transport_h4 = (hci_transport_h4_t*)malloc( sizeof(hci_transport_h4_t));
        hci_transport_h4->ds                                      = NULL;
        hci_transport_h4->transport.open                          = h4_open;
        hci_transport_h4->transport.close                         = h4_close;
        hci_transport_h4->transport.send_packet                   = h4_send_packet;
        hci_transport_h4->transport.register_packet_handler       = h4_register_packet_handler;
        hci_transport_h4->transport.get_transport_name            = h4_get_transport_name;
        hci_transport_h4->transport.set_baudrate                  = h4_set_baudrate;
        hci_transport_h4->transport.can_send_packet_now           = h4_can_send_packet_now;
    }
    return (hci_transport_t *) hci_transport_h4;
}
