#include "Driver_USART.h"

#include <math.h>

#include <errno.h>

#include <xmc4500.h>
#include <xmc_scu.h>

#include <arm_math.h>

#include "EventRecorder.h"

#include "RTE_Components.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

#include "custom_def.h"

#include "sine_generator.h"
#include "low_pass_filter.h"

//#include <jansson.h>
#include <cJSON.h>
#include <parson.h>

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

extern uint32_t asm_get_heap_base(void);
extern uint32_t asm_get_heap_size(void);

TaskHandle_t g_filter_task_handle;
TaskHandle_t g_disturb_gen_task_handle;
TaskHandle_t g_noise_gen_task_handle;
TaskHandle_t g_sine_gen_task_handle;
TaskHandle_t g_sync_task_handle;

//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------
//
// <e>Oscillator Configuration
// <i>This is a demonstration on how to use the Configuration Wizard to create an easy configuration menu.

#define ENABLE_CONFIG 1
// =============================
//   <o>Oscillator Sampling Frequency [Hz] <1000-10000>
//   <i> Set the oscillator sampling frequency.
//   <i> Default: 5000  (5 KHz)
#define SAMPLING_FREQ 1000  // generating task (5 KHz)

//   <o>Noise Frequency [Hz] <50-10000>
//   <i> Set the noise signal frequency.
//   <i> Default: 1500 Hz
#define NOISE_FREQ    50  // noise (2 KHz)

//   <o>Signal Frequency [Hz] <10-1000>
//   <i> Set the signal frequency.
//   <i> Default: 330 Hz
#define SIGNAL_FREQ    10  // disturbed signal (250 Hz)

// </e>
//------------- <<< end of configuration section >>> -----------------------


sine_generator_q15_t Signal_set;
sine_generator_q15_t Noise_set;

q15_t sine;
q15_t noise;
q15_t disturbed;
q15_t filtered;


//For Jasson test
/* Used by some code below as an example datatype. */
typedef struct str_poi_record {
    const char *precision;
    double lat;
    double lon;
    const char *address;
    const char *city;
    const char *state;
    const char *zip;
    const char *country;
}poi_record;

/* Our "days of the week" array: */
const char *str_weekdays[7] = {
        "Sunday",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday"
};

/* Our matrix: */
const int numbers[3][3] = {
        {0, -1, 0},
        {1, 0, 0},
        {0 ,0, 1}
};

/* Our "gallery" item: */
const int ids[4] = { 116, 943, 234, 38793 };
/* Our array of "records": */
const poi_record fields[2] = {
				{
                "zip",
                37.7668,
                -1.223959e+2,
                "",
                "SAN FRANCISCO",
                "CA",
                "94107",
                "US"
        },
        {
                "zip",
                37.371991,
                -1.22026e+2,
                "",
                "SUNNYVALE",
                "CA",
                "94085",
                "US"
        }
};

//void test_jansson(void) {
//    printf("Jansson Version:%s\n", JANSSON_VERSION);

//    json_t* root;
//    char* str_dump;

//    //Object Video Creating Test
//    {
//        json_t* fmt;
//        root = json_object();
//        json_object_set_new(root, "name", json_string("Jack (\"Bee\") Nimble"));
//        json_object_set_new(root, "format", fmt=json_object());
//        json_object_set_new(fmt,  "type", json_string("rect"));
//        json_object_set_new(fmt, "width", json_integer(1920));
//        json_object_set_new(fmt, "height", json_integer(1080));
//        json_object_set_new(fmt, "interlace", json_boolean(false));
//        json_object_set_new(fmt, "frame rate", json_integer(24));

//        str_dump = json_dumps( root, 0 );
//        printf( str_dump );
//        printf("\n");
//        ////flush_stdout();
//        free( str_dump );
//        json_decref( root );
//    }

//    //1-Dim Array of String Test
//    {
//        root = json_array();

//        for(size_t i=0; i<7; ++i )
//        {
//            json_array_append_new( root, json_string(str_weekdays[i]) );
//        }

//        str_dump = json_dumps( root, 0 );
//        printf( str_dump );
//        printf("\n");
//        ////flush_stdout();
//        free( str_dump );
//        json_decref( root );
//    }

//    //2-Dim Array of Integer Test
//    {
//        root = json_array();

//        for(size_t i=0; i<3; ++i )
//        {
//            json_t* jarr2 = json_array();
//            for(size_t j=0; j<3; ++j )
//            {
//                json_array_append_new( jarr2, json_integer((json_int_t)numbers[i][j]));
//            }
//            json_array_append_new( root, jarr2);
//        }

//        str_dump = json_dumps( root, 0 );
//        printf( str_dump );
//        printf("\n");
//        ////flush_stdout();
//        free( str_dump );
//        json_decref( root );
//    }

//    //Object Gallery Creating Test
//    {
//        json_t* img;
//        json_t* thm;

//        root = json_object();
//        json_object_set_new(root, "Image", img=json_object());
//        json_object_set_new(img, "Width", json_integer(800));
//        json_object_set_new(img, "Height", json_integer(600));
//        json_object_set_new(img,  "Title", json_string("View from 15th Floor"));
//        json_object_set_new(img, "Thumbnail", thm=json_object());
//        json_object_set_new(thm, "Url", json_string("http:/*www.example.com/image/481989943"));
//        json_object_set_new(thm, "Height", json_integer(125));
//        json_object_set_new(thm, "Width", json_string("100"));

//        json_t* jarr = json_array();
//        for(size_t i=0; i<sizeof(ids)/sizeof(ids[0]); ++i)
//        {
//            json_array_append_new( jarr, json_integer((json_int_t)ids[i]));
//        }
//        json_object_set_new(img, "IDs", jarr);

//        str_dump = json_dumps( root, 0 );
//        printf( str_dump );
//        printf("\n");
//        ////flush_stdout();
//        free( str_dump );
//        json_decref( root );
//    }

//    //Array of "records" Test
//    {
//        root = json_array();

//        for (size_t i = 0; i < 2; i++)
//        {
//            json_t* item = json_object();

//            json_object_set_new(item, "precision", json_string(fields[i].precision));
//            json_object_set_new(item, "Latitude", json_real(fields[i].lat));
//            json_object_set_new(item, "Longitude", json_real(fields[i].lon));
//            json_object_set_new(item, "Address", json_string(fields[i].address));
//            json_object_set_new(item, "City", json_string(fields[i].city));
//            json_object_set_new(item, "State", json_string(fields[i].state));
//            json_object_set_new(item, "Zip", json_string(fields[i].zip));
//            json_object_set_new(item, "Country", json_string(fields[i].country));

//            json_array_append_new( root, item);
//        }

//        str_dump = json_dumps( root, 0 );
//        printf( str_dump );
//        printf("\n");
//        ////flush_stdout();
//        free( str_dump );
//        json_decref( root );
//    }

//    //null Test
//    {
//        root = json_object();
//        volatile double zero = 0.0;

//        json_object_set_new(root, "number", json_null());

//        str_dump = json_dumps( root, 0 );
//        printf( str_dump );
//        printf("\n");
//        ////flush_stdout();
//        free( str_dump );
//        json_decref( root );
//    }
//}

//For cJSON test
/* Create a bunch of objects as demonstration. */
static int print_preallocated(cJSON *root)
{
    /* declarations */
    char *out = NULL;
    char *buf = NULL;
    char *buf_fail = NULL;
    size_t len = 0;
    size_t len_fail = 0;

    /* formatted print */
    out = cJSON_Print(root);

    /* create buffer to succeed */
    /* the extra 5 bytes are because of inaccuracies when reserving memory */
    len = strlen(out) + 5;
    buf = (char*)malloc(len);
    if (buf == NULL)
    {
        printf("Failed to allocate memory.\n");
        exit(1);
    }

    /* create buffer to fail */
    len_fail = strlen(out);
    buf_fail = (char*)malloc(len_fail);
    if (buf_fail == NULL)
    {
        printf("Failed to allocate memory.\n");
        exit(1);
    }

    /* Print to buffer */
    if (!cJSON_PrintPreallocated(root, buf, (int)len, 1)) {
        printf("cJSON_PrintPreallocated failed!\n");
        if (strcmp(out, buf) != 0) {
            printf("cJSON_PrintPreallocated not the same as cJSON_Print!\n");
            printf("cJSON_Print result:\n%s\n", out);
            printf("cJSON_PrintPreallocated result:\n%s\n", buf);
        }
        free(out);
        free(buf_fail);
        free(buf);
        return -1;
    }

    /* success */
    printf("%s\n", buf);

    /* force it to fail */
    if (cJSON_PrintPreallocated(root, buf_fail, (int)len_fail, 1)) {
        printf("cJSON_PrintPreallocated failed to show error with insufficient memory!\n");
        printf("cJSON_Print result:\n%s\n", out);
        printf("cJSON_PrintPreallocated result:\n%s\n", buf_fail);
        free(out);
        free(buf_fail);
        free(buf);
        return -1;
    }
    //flush_stdout();

    free(out);
    free(buf_fail);
    free(buf);
    return 0;
}

/* Create a bunch of objects as demonstration. */
static void create_objects(void)
{
    /* declare a few. */
    cJSON *root = NULL;
    cJSON *fmt = NULL;
    cJSON *img = NULL;
    cJSON *thm = NULL;
    cJSON *fld = NULL;
    int i = 0;

    volatile double zero = 0.0;

    /* Here we construct some JSON standards, from the JSON site. */

    /* Our "Video" datatype: */
    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "name", cJSON_CreateString("Jack (\"Bee\") Nimble"));
    cJSON_AddItemToObject(root, "format", fmt = cJSON_CreateObject());
    cJSON_AddStringToObject(fmt, "type", "rect");
    cJSON_AddNumberToObject(fmt, "width", 1920);
    cJSON_AddNumberToObject(fmt, "height", 1080);
    cJSON_AddFalseToObject (fmt, "interlace");
    cJSON_AddNumberToObject(fmt, "frame rate", 24);

    /* Print to text */
    if (print_preallocated(root) != 0) {
        cJSON_Delete(root);
        exit(EXIT_FAILURE);
    }
    cJSON_Delete(root);

    /* Our "days of the week" array: */
    root = cJSON_CreateStringArray(str_weekdays, 7);

    if (print_preallocated(root) != 0) {
        cJSON_Delete(root);
        exit(EXIT_FAILURE);
    }
    cJSON_Delete(root);

    /* Our matrix: */
    root = cJSON_CreateArray();
    for (i = 0; i < 3; i++)
    {
        cJSON_AddItemToArray(root, cJSON_CreateIntArray(numbers[i], 3));
    }

    /* cJSON_ReplaceItemInArray(root, 1, cJSON_CreateString("Replacement")); */

    if (print_preallocated(root) != 0) {
        cJSON_Delete(root);
        exit(EXIT_FAILURE);
    }
    cJSON_Delete(root);

    /* Our "gallery" item: */
    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "Image", img = cJSON_CreateObject());
    cJSON_AddNumberToObject(img, "Width", 800);
    cJSON_AddNumberToObject(img, "Height", 600);
    cJSON_AddStringToObject(img, "Title", "View from 15th Floor");
    cJSON_AddItemToObject(img, "Thumbnail", thm = cJSON_CreateObject());
    cJSON_AddStringToObject(thm, "Url", "http:/*www.example.com/image/481989943");
    cJSON_AddNumberToObject(thm, "Height", 125);
    cJSON_AddStringToObject(thm, "Width", "100");
    cJSON_AddItemToObject(img, "IDs", cJSON_CreateIntArray(ids, 4));

    if (print_preallocated(root) != 0) {
        cJSON_Delete(root);
        exit(EXIT_FAILURE);
    }
    cJSON_Delete(root);

    /* Our array of "records": */
    root = cJSON_CreateArray();
    for (i = 0; i < 2; i++)
    {
        cJSON_AddItemToArray(root, fld = cJSON_CreateObject());
        cJSON_AddStringToObject(fld, "precision", fields[i].precision);
        cJSON_AddNumberToObject(fld, "Latitude", fields[i].lat);
        cJSON_AddNumberToObject(fld, "Longitude", fields[i].lon);
        cJSON_AddStringToObject(fld, "Address", fields[i].address);
        cJSON_AddStringToObject(fld, "City", fields[i].city);
        cJSON_AddStringToObject(fld, "State", fields[i].state);
        cJSON_AddStringToObject(fld, "Zip", fields[i].zip);
        cJSON_AddStringToObject(fld, "Country", fields[i].country);
    }

    /* cJSON_ReplaceItemInObject(cJSON_GetArrayItem(root, 1), "City", cJSON_CreateIntArray(ids, 4)); */

    if (print_preallocated(root) != 0) {
        cJSON_Delete(root);
        exit(EXIT_FAILURE);
    }
    cJSON_Delete(root);

    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "number", 1.0 / zero);

    if (print_preallocated(root) != 0) {
        cJSON_Delete(root);
        exit(EXIT_FAILURE);
    }
    cJSON_Delete(root);
}

void test_cJSON(void)
{
    printf("Version: %s\n", cJSON_Version());
    //flush_stdout();

    /* Now some samplecode for building objects concisely: */
    create_objects();
}

//For parsson test
void serialization_example(void) {
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char *serialized_string = NULL;
    json_object_set_string(root_object, "name", "John Smith");
    json_object_set_number(root_object, "age", 25);
    json_object_dotset_string(root_object, "address.city", "Cupertino");
    json_object_dotset_value(root_object, "contact.emails", json_parse_string("[\"email@example.com\",\"email2@example.com\"]"));
    serialized_string = json_serialize_to_string_pretty(root_value);
    puts(serialized_string);
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
}

void test_parsson(void) {
   serialization_example();
}	
/*
*********************************************************************
*
* Sine Generator task
*
*********************************************************************
*/

void sine_gen(void) {
  while(1) { 
		xTaskNotifyWait( 0x00,      /* Don't clear any notification bits on entry. */
                         UINT32_MAX, /* Reset the notification value to 0 on exit. */
                         NULL,
                         portMAX_DELAY );  /* Block indefinitely. */
    sine = sine_calc_sample_q15(&Signal_set) / 2;

//		xTaskNotify( g_noise_gen_task_handle, 0, eNoAction );
  }
}

/*
*********************************************************************
*
* Noise Sine Generator task
*
*********************************************************************
*/

void noise_gen(void) {
  while(1) {
		xTaskNotifyWait( 0x00,      /* Don't clear any notification bits on entry. */
                         UINT32_MAX, /* Reset the notification value to 0 on exit. */
                         NULL,
                         portMAX_DELAY );  /* Block indefinitely. */
    noise = sine_calc_sample_q15(&Noise_set) / 6;
		xTaskNotify( g_disturb_gen_task_handle, 0, eNoAction );
  }
}

/*
*********************************************************************
*
* 1st Disturb Generator task
*
* Adds Noise to Signal
*
*********************************************************************
*/

void disturb_gen(void) {
  while(1) {
		xTaskNotifyWait( 0x00,      /* Don't clear any notification bits on entry. */
                         UINT32_MAX, /* Reset the notification value to 0 on exit. */
                         NULL,
                         portMAX_DELAY );  /* Block indefinitely. */
    disturbed = sine + noise;
		xTaskNotify( g_filter_task_handle, 0, eNoAction );
  }
}

/*
*********************************************************************
*
* Filter task
*
* Extract Signal from disturbed signal by filtering out Noise
*
*********************************************************************
*/

void filter_tsk(void) {
  while(1) {
		xTaskNotifyWait( 0x00,      /* Don't clear any notification bits on entry. */
                         UINT32_MAX, /* Reset the notification value to 0 on exit. */
                         NULL,
                         portMAX_DELAY );  /* Block indefinitely. */
    filtered = low_pass_filter(&disturbed); 
  }
}

/*
*********************************************************************
*
* Synchronization task
*
* Periodically starts the signal generation process
*
*********************************************************************
*/

void sync_tsk(void) {
	TickType_t xLastWakeTime;
  while(1) {			
		XMC_SCU_StartTemperatureMeasurement();		
		
//		xLastWakeTime = xTaskGetTickCount ();    
//		xTaskNotify( g_sine_gen_task_handle, 0, eNoAction );
//    vTaskDelayUntil( &xLastWakeTime, 10 / portTICK_PERIOD_MS );
		
		printf("t:%u\n", xTaskGetTickCount());
		//T_DTS = (RESULT - 605) / 2.05 [°C]
		tmpDts = XMC_SCU_GetTemperatureMeasurement();
		tmpCel = (tmpDts-605)/2.05;
		printf("%.1f\n", tmpCel);

		tmpV13 = XMC_SCU_POWER_GetEVR13Voltage();
		tmpV33 = XMC_SCU_POWER_GetEVR33Voltage();
		printf("%.1f %.1f\n", tmpV13, tmpV33);	
	
		//Jasson test
//		test_jansson();
		
		//cJSON test
		test_cJSON();
		
		//parson test
		test_parsson();
		
		//parsson test
    vTaskDelay(5000 / portTICK_PERIOD_MS);	
				
		xTaskNotify( g_sine_gen_task_handle, 0, eNoAction );
  }
}

#define TEST_BAUDRATE	(5529600)
int main(void) {
  EventRecorderInitialize(EventRecordAll, 1);
	
	/*Initialize the UART driver */
	UARTdrv->Initialize(myUART_callback);
	/*Power up the UART peripheral */
	UARTdrv->PowerControl(ARM_POWER_FULL);
	/*Configure the USART to 9600 Bits/sec */
	UARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
									 ARM_USART_DATA_BITS_8 |
									 ARM_USART_PARITY_NONE |
									 ARM_USART_STOP_BITS_1 , TEST_BAUDRATE);
	 
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
	
	  // compute coefficients for IIR sine generators
  sine_generator_init_q15(&Signal_set, SIGNAL_FREQ, SAMPLING_FREQ);
  sine_generator_init_q15(&Noise_set, NOISE_FREQ, SAMPLING_FREQ);
  printf ("Sine Generator Initialised\n\r");

  // initialize low pass filter
//  low_pass_filter_init();
//  printf ("Low Pass Filter Initialised\n\r");

  // initialize the timing system to activate the four tasks 
  // of the application program
//		xTaskCreate((TaskFunction_t)filter_tsk,
//							(const portCHAR *)"filter_tsk",
//							384,
//							NULL,
//							2,
//							&g_filter_task_handle);							
//	printf ("filter_tsk Task Initialised\n\r");

//		xTaskCreate((TaskFunction_t)disturb_gen,
//							(const portCHAR *)"disturb_gen",
//							384,
//							NULL,
//							2,
//							&g_disturb_gen_task_handle);
//	printf ("disturb_gen Task Initialised\n\r");
//								xTaskCreate((TaskFunction_t)noise_gen,
//							(const portCHAR *)"noise_gen",
//							384,
//							NULL,
//							2,
//							&g_noise_gen_task_handle);
//  printf ("noise_gen Task Initialised\n\r");
												
								xTaskCreate((TaskFunction_t)sine_gen,
							(const portCHAR *)"sine_gen",
							256,
							NULL,
							tskIDLE_PRIORITY+1,
							&g_sine_gen_task_handle);							
  printf ("sine_gen Task Initialised\n\r");
							
								xTaskCreate((TaskFunction_t)sync_tsk,
							(const portCHAR *)"sync_tsk",
							256,
							NULL,
							tskIDLE_PRIORITY+2,
							&g_sync_task_handle);							
  printf ("sync_tsk Task Initialised\n\r");
  printf ("Application Running\n\r");
	
  /* Start scheduler */  
	vTaskStartScheduler();
	
//Should never come here
							
  while (1) {
		printf("ticks: %u\n", g_ticks);
		//T_DTS = (RESULT - 605) / 2.05 [°C]
		tmpDts = XMC_SCU_GetTemperatureMeasurement();
		tmpCel = (tmpDts-605)/2.05;
		printf("%f\n", tmpCel);

		tmpV13 = XMC_SCU_POWER_GetEVR13Voltage();
		tmpV33 = XMC_SCU_POWER_GetEVR33Voltage();
		printf("%f %f\n", tmpV13, tmpV33);
		
		printf("%u MHz\n", SystemCoreClock/1000000);

//		tmp_cyccnt = DWT->CYCCNT;
//		tmp_cpicnt = DWT->CPICNT;
//		tmp_exccnt = DWT->EXCCNT;
//		tmp_sleepcnt = DWT->SLEEPCNT;
//		tmp_lsucnt = DWT->LSUCNT;
//		tmp_foldcnt = DWT->FOLDCNT;
		
		//CYCCNT - CPICNT - EXCCNT - SLEEPCNT - LSUCNT + FOLDCNT
//		printf("DWT CYCCNT:%u\n", tmp_cyccnt);
//		printf("DWT CPICNT:%u\n", tmp_cpicnt);
//		printf("DWT EXCCNT:%u\n", tmp_exccnt);
//		printf("DWT SLEEPCNT:%u\n", tmp_sleepcnt);
//		printf("DWT LSUCNT:%u\n", tmp_lsucnt);
//		printf("DWT FOLDCNT:%u\n", tmp_foldcnt);
//		printf("CYCCNT - CPICNT - EXCCNT - SLEEPCNT - LSUCNT + FOLDCNT = %u\n",
//		tmp_cyccnt - tmp_cpicnt -tmp_exccnt - tmp_sleepcnt - tmp_lsucnt + tmp_foldcnt);
		
//		test_div_flash();
//		
//		test_div_sram();
//		test_alloca();

  #ifdef RTE_Compiler_IO_STDOUT_User
	printf("RTE_Compiler_IO_STDOUT_User\n");
	#endif
	
  #ifdef RTE_Compiler_IO_STDOUT_EVR
	printf("RTE_Compiler_IO_STDOUT_EVR\n");
	#endif

  #ifdef RTE_Compiler_IO_STDOUT_ITM
	printf("RTE_Compiler_IO_STDOUT_ITM\n");
	#endif
	
		uint32_t tmp_ticks = g_ticks;
		while((tmp_ticks + 1000) > g_ticks) {
//			__wfi();
		}
		
		XMC_SCU_StartTemperatureMeasurement();		
	}
}

