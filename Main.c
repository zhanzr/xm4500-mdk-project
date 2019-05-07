#include "Driver_USART.h"

#include <math.h>

#include <xmc4500.h>
#include <xmc_scu.h>

#include "EventRecorder.h"

#include "custom_def.h"

/* USART Driver */
extern ARM_DRIVER_USART Driver_USART2;
static ARM_DRIVER_USART *UARTdrv = &Driver_USART2; 
static uint8_t cmd;

uint8_t g_tmp_uart_rx_buf;
__IO uint32_t g_ticks;

void myUART_callback(uint32_t event) {
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

static uint32_t tmpDts;
static float tmpCel;
static float tmpV13;
static float tmpV33;

static double M_PI;

#define SIGNAL_AMPLIT	5.0
static __IO float tmp_ms_cycle;
static __IO float signal_1;
static __IO float signal_2;
static __IO float signal_3;
static __IO float signal_4;

static double special_value;

void test_div_flash(void)
{
//	printf("%s %p\n", __func__, test_div_flash);

	for(uint32_t i=0; i<TEST_LOOP_N; ++i)
	{
		__nop();
	}
}

extern void test_div_sram(void);

int main(void) {
	uint32_t tmp_cyccnt = 0;
	uint32_t tmp_cpicnt = 0;
	uint32_t tmp_exccnt = 0;
	uint32_t tmp_sleepcnt = 0;
	uint32_t tmp_lsucnt = 0;
	uint32_t tmp_foldcnt = 0;

	M_PI = acos(-1);
//5 * math.tan((499/1000)*math.pi)*(1000/2-499)/(1000/2)
	special_value = SIGNAL_AMPLIT * tan(M_PI*(HZ/2-1)/HZ)*(HZ/2-(HZ/2-1))/(HZ/2);
	
  EventRecorderInitialize(EventRecordAll, 1);
	
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
	
//	SysTick_Config(SystemCoreClock / HZ);

	printf("XMC4500 ARMCC Test @ %u Hz\n", SystemCoreClock);
  UARTdrv->Receive(&cmd, 1);

	printf("%u Hz, %08X, CM:%d, FPU_USED:%d\n",
			SystemCoreClock, SCB->CPUID,
			__CORTEX_M, __FPU_USED);
	printf("Boot Mode:%u\n", XMC_SCU_GetBootMode());
	
  while (1) {
		printf("ticks: %u\n", g_ticks);
		//T_DTS = (RESULT - 605) / 2.05 [°C]
		tmpDts = XMC_SCU_GetTemperatureMeasurement();
		tmpCel = (tmpDts-605)/2.05;
		printf("%f\n", tmpCel);

		tmpV13 = XMC_SCU_POWER_GetEVR13Voltage();
		tmpV33 = XMC_SCU_POWER_GetEVR33Voltage();
		printf("%f %f\n", tmpV13, tmpV33);
		
		printf("%u MHz PI=%f\n", SystemCoreClock/1000000, M_PI);

		tmp_cyccnt = DWT->CYCCNT;
		tmp_cpicnt = DWT->CPICNT;
		tmp_exccnt = DWT->EXCCNT;
		tmp_sleepcnt = DWT->SLEEPCNT;
		tmp_lsucnt = DWT->LSUCNT;
		tmp_foldcnt = DWT->FOLDCNT;
		
		//CYCCNT - CPICNT - EXCCNT - SLEEPCNT - LSUCNT + FOLDCNT
		printf("DWT CYCCNT:%u\n", tmp_cyccnt);
		printf("DWT CPICNT:%u\n", tmp_cpicnt);
		printf("DWT EXCCNT:%u\n", tmp_exccnt);
		printf("DWT SLEEPCNT:%u\n", tmp_sleepcnt);
		printf("DWT LSUCNT:%u\n", tmp_lsucnt);
		printf("DWT FOLDCNT:%u\n", tmp_foldcnt);
		printf("CYCCNT - CPICNT - EXCCNT - SLEEPCNT - LSUCNT + FOLDCNT = %u\n",
		tmp_cyccnt - tmp_cpicnt -tmp_exccnt - tmp_sleepcnt - tmp_lsucnt + tmp_foldcnt);
		
		test_div_flash();
		
		test_div_sram();
		
		uint32_t tmp_ticks = g_ticks;
		while((tmp_ticks + 1000) > g_ticks) {
			__wfi();
		}
		
		XMC_SCU_StartTemperatureMeasurement();		
	}
}

void SysTick_Handler(void) {
	g_ticks ++;

	tmp_ms_cycle++;
	if((float)HZ == tmp_ms_cycle) {
		tmp_ms_cycle = 0;
	}
	
	//Simply increase
	signal_1 = SIGNAL_AMPLIT * (tmp_ms_cycle/HZ);
	
	//Triangle 
	signal_2 = (tmp_ms_cycle<(HZ/2))?
	((SIGNAL_AMPLIT/(HZ/2)) * tmp_ms_cycle):
	(((SIGNAL_AMPLIT/(HZ/2)) * (HZ/2+HZ-tmp_ms_cycle))-SIGNAL_AMPLIT);
	
	//Sine
	signal_3 = SIGNAL_AMPLIT * (sinf((2*M_PI) * tmp_ms_cycle/HZ) + 1)/2;
	
	//tangium
	if(tmp_ms_cycle == (HZ/2)) {
		signal_4 = special_value;
	} else {
		signal_4 = (tmp_ms_cycle<(HZ/2))?
		(SIGNAL_AMPLIT * tanf((M_PI) * tmp_ms_cycle/HZ) * (HZ/2-tmp_ms_cycle))/(HZ/2):
		(SIGNAL_AMPLIT * tanf(M_PI - (M_PI) * tmp_ms_cycle/HZ) * (tmp_ms_cycle-HZ/2))/(HZ/2);
	}
}
