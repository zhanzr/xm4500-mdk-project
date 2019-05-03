#include "Driver_USART.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <xmc4500.h>
#include <xmc_scu.h>

#ifndef HZ
#define	HZ	1000
#endif

/* USART Driver */
extern ARM_DRIVER_USART Driver_USART2;
static ARM_DRIVER_USART *UARTdrv = &Driver_USART2; 
static uint8_t cmd;

uint8_t g_tmp_uart_rx_buf;
__IO uint32_t g_ticks;

void SysTick_Handler(void) {
	g_ticks ++;
}

void myUART_callback(uint32_t event)
{
    switch (event)
    {
    case ARM_USART_EVENT_RECEIVE_COMPLETE:  
//		 if (cmd == 0x0D)        /* Send back the message if Enter is recived*/
//		 {
//			 UARTdrv->Send("\nHello World!\r\n", 15);
//			 cmd = 0;   
//   		 UARTdrv->Receive(&cmd, 1);
//		 }
		   		 
		UARTdrv->Receive(&g_tmp_uart_rx_buf, 1);
		 break;
		 
    case ARM_USART_EVENT_TRANSFER_COMPLETE:
    case ARM_USART_EVENT_SEND_COMPLETE:
    case ARM_USART_EVENT_TX_COMPLETE:
     
        break;
 
    case ARM_USART_EVENT_RX_TIMEOUT:
         __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
 
    case ARM_USART_EVENT_RX_OVERFLOW:
    case ARM_USART_EVENT_TX_UNDERFLOW:
        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
    }
}
  
int stdout_putchar (int ch) {
	UARTdrv->Send((uint8_t*)&ch, 1);
	return ch;
}

int main(void) {
	/*Initialize the UART driver */
	UARTdrv->Initialize(myUART_callback);
	/*Power up the UART peripheral */
	UARTdrv->PowerControl(ARM_POWER_FULL);
	/*Configure the USART to 9600 Bits/sec */
	UARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
									 ARM_USART_DATA_BITS_8 |
									 ARM_USART_PARITY_NONE |
									 ARM_USART_STOP_BITS_1 , 921600);
	 
	/* Enable the Transmitter line */
	UARTdrv->Control (ARM_USART_CONTROL_TX, 1);

	XMC_SCU_EnableTemperatureSensor();
	XMC_SCU_StartTemperatureMeasurement();
	
	SysTick_Config(SystemCoreClock / HZ);

	printf("XMC4500 ARMCC Test @ %u Hz\n", SystemCoreClock);
  UARTdrv->Receive(&cmd, 1);

	printf("%u Hz, %08X, CM:%d, FPU_USED:%d\n",
			SystemCoreClock, SCB->CPUID,
			__CORTEX_M, __FPU_USED);
	printf("Boot Mode:%u\n", XMC_SCU_GetBootMode());
	
  while (1) {
		printf("ticks: %u\n", g_ticks);
		//T_DTS = (RESULT - 605) / 2.05 [°C]
		uint32_t tmpDts = XMC_SCU_GetTemperatureMeasurement();
		float tmpCel = (tmpDts-605)/2.05;
		printf("%f\n", tmpCel);

		float tmpV13 = XMC_SCU_POWER_GetEVR13Voltage();
		float tmpV33 = XMC_SCU_POWER_GetEVR33Voltage();
		printf("%f %f\n", tmpV13, tmpV33);
		
		uint32_t tmp_ticks = g_ticks;
		while((tmp_ticks + 2000) > g_ticks) {
			__nop();
		}
		
		XMC_SCU_StartTemperatureMeasurement();		
	}
}
