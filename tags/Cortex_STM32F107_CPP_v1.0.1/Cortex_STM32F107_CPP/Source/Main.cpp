/**
 * @file       : Main.cpp
 * @author     : Francesco Lettich
 * @author     : Stefano Oliveri
 * @version    : v0.2.0
 * @date       : 6/8/2010
 * Copyright   :
 * Description : Hello World in C++, Ansi-style
 */


/******* STM Library Include **********************************/
// Specifies the MCU peripheral to use
#include "stm32f10x_conf.h"
// MCU specific settings
#include "stm32f10x.h"

#include "CFreeRTOS.h"
#include "CTask.h"

#include "CCheckTask.h"
#include "CBlockQ.h"
#include "CInteger.h"
#include "CPollQ.h"
#include "CSemTest.h"

#include "CHelloWorld.h"
#include "CLcdTask.h"
#include "CGuardTestTask.h"

#include "CHeapTestTask.h"


/* Task priorities. */
#define mainBLOCK_Q_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainFLASH_TASK_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainINTEGER_TASK_PRIORITY           ( tskIDLE_PRIORITY )
#define mainQUEUE_POLL_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainSEM_TEST_PRIORITY				( tskIDLE_PRIORITY + 1 )



/**
 * Configure the hardware for the demo.
 */
static void prvSetupHardware( void );


/**
 * Heap allocated task object
 */
CHelloWorld *g_pLed1Task;

/**
 * Global task objects.
 */
CHelloWorld g_Led2Task(GPIOE, GPIO_Pin_14, 1000);
CCheckTask g_checkTask(4000/portTICK_RATE_MS);

CHeapTestTask g_HeapTestTask;
CHeapTestTaskMonitor g_HeapTestMonitorTask;

/**
 * @brief: Main programma.
 */
int main() {

	prvSetupHardware();

	// While usually dynamic memory is not a good practice in an embedded program,
	// this task is allocated on the heap only to test if the low level layer works fine.
	// (linker script and runtime support)
	g_pLed1Task = new CHelloWorld(GPIOD, GPIO_Pin_13, 2000);
	g_pLed1Task->Create("Led1", configMINIMAL_STACK_SIZE, mainFLASH_TASK_PRIORITY);

	g_Led2Task.Create("Led2", configMINIMAL_STACK_SIZE, 0);//mainFLASH_TASK_PRIORITY);
	g_HeapTestTask.Create("HT", configMINIMAL_STACK_SIZE, 3);
	g_HeapTestMonitorTask.Create("HTM", 3*configMINIMAL_STACK_SIZE, 5);


	// Static task object
	static CHelloWorld led3Task(GPIOD, GPIO_Pin_3 | GPIO_Pin_4, 3000);
	led3Task.Create("Led3", configMINIMAL_STACK_SIZE, mainFLASH_TASK_PRIORITY);

	CSharedResourceEditor::StartGuardTestClass();

	g_checkTask.Create("Check", configMINIMAL_STACK_SIZE, configMAX_PRIORITIES-1);
	ABlockQ::StartBlockingQueueTasks(&g_checkTask, mainBLOCK_Q_PRIORITY);
	CInteger::StartIntegerMathTasks(&g_checkTask, mainINTEGER_TASK_PRIORITY);
	APollQ::StartPolledQueueTasks(&g_checkTask, mainQUEUE_POLL_PRIORITY);
	CSemTest::StartSemTestTasks(&g_checkTask, mainSEM_TEST_PRIORITY);


	CFreeRTOS::InitHardwareForManagedTasks();
	CFreeRTOS::StartScheduler();

    while(1);

    return 0;
}

static void prvSetupHardware( void )
{
	/* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE and AFIO clocks */
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOC
							| RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE );

	/* Set the Vector Table base address at 0x08000000 */
	NVIC_SetVectorTable( NVIC_VectTab_FLASH, 0x0 );

	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

	/* Configure HCLK clock as SysTick clock source. */
	SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );
}

extern "C"
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
	{
		/* This function will get called if a task overflows its stack.   If the
		parameters are corrupt then inspect pxCurrentTCB to find which was the
		offending task. */

		( void ) pxTask;
		( void ) pcTaskName;

		for( ;; );
	}

