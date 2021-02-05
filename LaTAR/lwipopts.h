/**
  ******************************************************************************
  * @file    LwIP/LwIP_HTTP_Server_Netconn_RTOS/Inc/lwipopts.h
  * @author  MCD Application Team
  * @brief   lwIP Options Configuration.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright � 2017 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#include "stm32f4xx.h"


#ifdef __cplusplus
extern "C" {
#endif


#define LWIP_DEBUG		1
#define DHCP_DEBUG		LWIP_DBG_ON
#define IP_DEBUG		LWIP_DBG_OFF
#define TCP_DEBUG		LWIP_DBG_ON
#define ETHARP_DEBUG	LWIP_DBG_OFF
#define PBUF_DEBUG		LWIP_DBG_OFF
#define TCPIP_DEBUG		LWIP_DBG_OFF
#define UDP_DEBUG		LWIP_DBG_OFF


#define NO_SYS                  0
#define SYS_LIGHTWEIGHT_PROT    0

#define LWIP_ICMP                       1
#define LWIP_DHCP					    1
#define LWIP_UDP						1
#define LWIP_TCP						1
#define LWIP_STATS						1
#define LWIP_NETIF_LINK_CALLBACK        1
#define LWIP_TIMERS						1
#define LWIP_NETCONN                    1
#define LWIP_SOCKET                     0
#define LWIP_CALLBACK_API				1
#define LWIP_ARP						1

#define ETHARP_TRUST_IP_MAC     0

#define IP_FORWARD              0
#define IP_OPTIONS              1
#define IP_REASSEMBLY           0
#define IP_FRAG                 0

#define ARP_TABLE_SIZE 10
#define ARP_QUEUEING 1

#define MEM_ALIGNMENT           4
#define MEM_SIZE                (10*1024)

#define MEMP_NUM_PBUF           100
#define MEMP_NUM_UDP_PCB        6
#define MEMP_NUM_TCP_PCB        10
#define MEMP_NUM_TCP_PCB_LISTEN 5
#define MEMP_NUM_TCP_SEG        12
#define MEMP_NUM_SYS_TIMEOUT    10

#define PBUF_POOL_SIZE          10
#define PBUF_POOL_BUFSIZE       1524

#define TCP_TTL                 255
#define TCP_QUEUE_OOSEQ         0
#define TCP_MSS                 (1500 - 40)	  /* TCP_MSS = (Ethernet MTU - IP header size - TCP header size) */
#define TCP_SND_BUF             (4*TCP_MSS)
#define TCP_SND_QUEUELEN        (2* TCP_SND_BUF/TCP_MSS)
#define TCP_WND                 (2*TCP_MSS)

#define UDP_TTL							255

#define CHECKSUM_GEN_IP                 0
#define CHECKSUM_GEN_UDP                0
#define CHECKSUM_GEN_TCP                0 
#define CHECKSUM_CHECK_IP               0
#define CHECKSUM_CHECK_UDP              0
#define CHECKSUM_CHECK_TCP              0 
#define CHECKSUM_GEN_ICMP               0

#define WITH_RTOS				1
#define LWIP_DNS				1

/*
   --------------------------------------
   ---------- Checksum options ----------
   --------------------------------------
*/

/* 
The STM32F4xx allows computing and verifying the IP, UDP, TCP and ICMP checksums by hardware:
 - To use this feature let the following define uncommented.
 - To disable it and process by CPU comment the  the checksum.
*/
#define CHECKSUM_BY_HARDWARE 


#ifdef CHECKSUM_BY_HARDWARE
  /* CHECKSUM_GEN_IP==0: Generate checksums by hardware for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 0
  /* CHECKSUM_GEN_UDP==0: Generate checksums by hardware for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                0
  /* CHECKSUM_GEN_TCP==0: Generate checksums by hardware for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                0 
  /* CHECKSUM_CHECK_IP==0: Check checksums by hardware for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               0
  /* CHECKSUM_CHECK_UDP==0: Check checksums by hardware for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              0
  /* CHECKSUM_CHECK_TCP==0: Check checksums by hardware for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              0
  /* CHECKSUM_CHECK_ICMP==0: Check checksums by hardware for incoming ICMP packets.*/  
  #define CHECKSUM_GEN_ICMP               0
#else
  /* CHECKSUM_GEN_IP==1: Generate checksums in software for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 1
  /* CHECKSUM_GEN_UDP==1: Generate checksums in software for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                1
  /* CHECKSUM_GEN_TCP==1: Generate checksums in software for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                1
  /* CHECKSUM_CHECK_IP==1: Check checksums in software for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               1
  /* CHECKSUM_CHECK_UDP==1: Check checksums in software for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              1
  /* CHECKSUM_CHECK_TCP==1: Check checksums in software for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              1
  /* CHECKSUM_CHECK_ICMP==1: Check checksums by hardware for incoming ICMP packets.*/  
  #define CHECKSUM_GEN_ICMP               1
#endif



/*
   ------------------------------------
   ---------- httpd options ----------
   ------------------------------------
*/
/** Set this to 1 to include "fsdata_custom.c" instead of "fsdata.c" for the
 * file system (to prevent changing the file included in CVS) */
#define HTTPD_USE_CUSTOM_FSDATA   0

/*
   ---------------------------------
   ---------- OS options ----------
   ---------------------------------
*/

#define TCPIP_THREAD_NAME              "TCP/IP"
#define TCPIP_THREAD_STACKSIZE          1000
#define TCPIP_MBOX_SIZE                 6
#define DEFAULT_UDP_RECVMBOX_SIZE       6
#define DEFAULT_TCP_RECVMBOX_SIZE       6
#define DEFAULT_ACCEPTMBOX_SIZE         6
#define DEFAULT_THREAD_STACKSIZE        500
#define TCPIP_THREAD_PRIO               osPriorityHigh


#ifdef __cplusplus
}
#endif

#endif /* __LWIPOPTS_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
