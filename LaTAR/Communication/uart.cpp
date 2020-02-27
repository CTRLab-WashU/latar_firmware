#include <stm32f4xx_hal.h>
#include <stm32_hal_legacy.h>
#include "stm32f4xx_hal_uart.h"
#include <stdio.h>

#include "DataStructures/RingBuffer.h"
#include "Communication/wrapped_buffer.h"

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#include "uart.h"

#include <cstring>

static UART_HandleTypeDef uart_handle;
static bool uart_is_valid = false;

static void uart_rx_thread(void const * argument);
static void uart_tx_thread(void const * argument);

osSemaphoreId uart_tx_semaphore = NULL;
RingBuffer<wrapped_buffer, 64> uart_tx_queue;
bool uart_tx_blocked;


void(*process_byte)(uint8_t byte) = 0;

// gnd == black
// rx  == white
// tx  == grey
	
void uart_init()
{
	__HAL_RCC_USART2_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
    
	GPIO_InitTypeDef GPIO_InitStructure;
 
	GPIO_InitStructure.Pin = GPIO_PIN_5 | GPIO_PIN_6;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Alternate = GPIO_AF7_USART2;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
    
	uart_handle.Instance			= USART2;
	uart_handle.Init.BaudRate		= 1200;
	uart_handle.Init.WordLength		= UART_WORDLENGTH_8B;
	uart_handle.Init.StopBits		= UART_STOPBITS_2;
	uart_handle.Init.Parity			= UART_PARITY_NONE;
	uart_handle.Init.HwFlowCtl		= UART_HWCONTROL_NONE;
	uart_handle.Init.Mode			= UART_MODE_TX_RX;
	uart_handle.Init.OverSampling	= UART_OVERSAMPLING_16;
	
	if (HAL_UART_Init(&uart_handle) != HAL_OK) {
		printd("failed to initialize uart");
		return;
	}
	
	NVIC_EnableIRQ(USART2_IRQn);
	
	// create tx semaphore
	osSemaphoreDef(uart_tx_sem);
	uart_tx_semaphore = osSemaphoreCreate(osSemaphore(uart_tx_sem), 1);
	
	// create tx thread
	osThreadDef(uart_tx, uart_tx_thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(uart_tx), NULL);
	
	// create rx thread
	osThreadDef(uart_rx, uart_rx_thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(uart_rx), NULL);
	
	uart_tx_blocked = false;
	uart_is_valid = true;
}

void uart_register_callback(void(*process)(uint8_t byte))
{
	process_byte = process;
}

void uart_unregister_callback()
{
	process_byte = 0;
}

void uart_write(wrapped_buffer buffer)
{
	if (uart_tx_blocked || uart_tx_semaphore == NULL) {
		return;
	}
	
	uart_tx_queue.enqueue(buffer);
	
	osSemaphoreRelease(uart_tx_semaphore);
}

void uart_write(const char * buffer)
{
	wrapped_buffer wrap(buffer);
	uart_write(wrap);	
}

static void uart_rx_thread(void const * argument)
{ 
	uint8_t rx_temp_buffer[1];
	
	for (;;) {
		if (HAL_UART_Receive(&uart_handle, (uint8_t *)&rx_temp_buffer, 1, HAL_MAX_DELAY) == HAL_OK) {
			if (process_byte) {
				process_byte(rx_temp_buffer[0]);
			}
		}
	}
	
}

void uart_tx(wrapped_buffer buffer)
{ 
//	printf(buffer.data.data());
//	printf("\n");
	if (HAL_UART_Transmit(&uart_handle, (uint8_t *)buffer.data.data(), buffer.data.size(), HAL_MAX_DELAY) != HAL_OK) {
		printf("uart write failed\n");
	}
}

void uart_tx(uint8_t byte)
{
	uint8_t buff_t[1];
	buff_t[0] = byte;
	if (status != HAL_OK) {
		printd("uart write failed\n");
	}
}

static void uart_tx_thread(void const * argument)
{ 
	wrapped_buffer tx_temp_buffer;
	
	for (;;) {
		if (osSemaphoreWait(uart_tx_semaphore, osWaitForever) == osOK) {
			uart_tx_blocked = true;
			while (!uart_tx_queue.isEmpty()) {
				tx_temp_buffer = uart_tx_queue.dequeue();
				if (HAL_UART_Transmit(&uart_handle, (uint8_t *)tx_temp_buffer.data.data(), tx_temp_buffer.data.size(), HAL_MAX_DELAY) != HAL_OK) {
				} else {
//					printf("uart sent '");
//					printf(tx_temp_buffer.data.data());
//					printf("'\n");
				}				
					printd("uart write failed\n");
			}
			uart_tx_queue.clear();
			uart_tx_blocked = false;
		}
	}
}

extern "C" void USART2_IRQHandler()
{
	HAL_UART_IRQHandler(&uart_handle);
}
 
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (uart_tx_semaphore == NULL) {
		return;
	}
	osSemaphoreRelease(uart_tx_semaphore);
}

