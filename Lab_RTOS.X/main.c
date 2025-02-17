/**
  Generated main.c file from MPLAB Code Configurator

  @Company
    Microchip Technology Inc.

  @File Name
    main.c

  @Summary
    This is the generated main.c using PIC24 / dsPIC33 / PIC32MM MCUs.

  @Description
    This source file provides main entry point for system initialization and application code development.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.167.0
        Device            :  PIC32MM0256GPM064
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.50
        MPLAB 	          :  MPLAB X v5.35
 */

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
 */

/**
  Section: Included Files
 */

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "freeRTOS/include/semphr.h"
#include "mcc_generated_files/system.h"
#include "mcc_generated_files/pin_manager.h"
#include "mcc_generated_files/usb/usb_device.h"
#include "mcc_generated_files/usb/usb.h"
#include "utils/USB.h"
#include "system/UI.h"
#include "plataform/LED_RGB.h"
#include "utils/text.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdio.h>


void blinkLED(void *p_param);
void updateTime(void *p_param);
void userInterface(void *p_param);
void manageUSB(void *p_param);

/*
                         Main application
 */
struct tm timeDate;
time_t timeStamp;
uint8_t rxData[24];
xSemaphoreHandle sendData;
xSemaphoreHandle readData;
xSemaphoreHandle xTimer;

int main(void) {
    // initialize the device
    SYSTEM_Initialize();

    /* Create the tasks defined within this file. */
    xTaskCreate(blinkLED, "task1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

    vSemaphoreCreateBinary(sendData);
    vSemaphoreCreateBinary(readData);
    vSemaphoreCreateBinary(xTimer);


    xTaskCreate(manageUSB, "task2", configMINIMAL_STACK_SIZE, rxData, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(userInterface, "task3", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(updateTime, "task4", configMINIMAL_STACK_SIZE, (uint32_t*) timeStamp, tskIDLE_PRIORITY + 2, NULL);

    /* Finally start the scheduler. */
    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following line
    will never be reached.  If the following line does execute, then there was
    insufficient FreeRTOS heap memory available for the idle and/or timer tasks
    to be created.  See the memory management section on the FreeRTOS web site
    for more details. */
    for (;;);
}

void blinkLED(void *p_param) {
    for (;;) {
        LEDA_SetHigh();
        vTaskDelay(pdMS_TO_TICKS(400));
        LEDA_SetLow();
        vTaskDelay(pdMS_TO_TICKS(800));
    }
    vTaskDelete(NULL);
}

void updateTime(void *p_param) {
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        if (RTCC_TimeGet(&timeDate)) {
            timeStamp = mktime(&timeDate);
            xSemaphoreGive(xTimer);
        }
    }
    vTaskDelete(NULL);
}

void manageUSB(void *p_param) {
    for (;;) {
        if ((USBGetDeviceState() >= CONFIGURED_STATE) && !USBIsDeviceSuspended()) {
            uint8_t bytesReceived;
            bytesReceived = USBReceive(rxData);
            if (bytesReceived > 0) {
                rxData[bytesReceived] = '\0'; // End of String
                xSemaphoreGive(readData);
            }

        }

        if (USBUSARTIsTxTrfReady()) {
            xSemaphoreGive(sendData);
        }

        CDCTxService();

        vTaskDelay(pdMS_TO_TICKS(1));
    }
    vTaskDelete(NULL);
}

void userInterface(void *p_param) {
    static ui_menu_states_t menuState = UI_MENU_STATE_INIT;
    for (;;) {
        switch (menuState) {
            case( UI_MENU_STATE_INIT):;

                xSemaphoreTake(readData, portMAX_DELAY);
                memset(rxData, 0, sizeof (rxData));

                xSemaphoreTake(sendData, portMAX_DELAY);
                putsUSBUSART((uint8_t*) textoVacio);

                //Espero que el usuario presione una tecla
                xSemaphoreTake(readData, portMAX_DELAY);
                memset(rxData, 0, sizeof (rxData));
                xSemaphoreTake(sendData, portMAX_DELAY);
                putsUSBUSART((uint8_t*) textoBienvenida);


                menuState = UI_MENU_STATE_OPTIONS;

                break;
            case( UI_MENU_STATE_OPTIONS):

                xSemaphoreTake(sendData, portMAX_DELAY);
                putsUSBUSART((uint8_t*) textoOpciones);

                xSemaphoreTake(readData, portMAX_DELAY);
                if ((UI_checkValidOption(rxData, 1, 3))) {
                    menuState = UI_MENU_STATE_OPTIONS + atoi(rxData);
                    memset(rxData, 0, sizeof (rxData));
                } else {
                    xSemaphoreTake(sendData, portMAX_DELAY);
                    putsUSBUSART((uint8_t*) textoNoValido);
                    menuState = UI_MENU_STATE_OPTIONS;
                }

                break;

            case( UI_MENU_STATE_SET_TIMEDATE):
                if (UI_setTimedate()) {
                    menuState = UI_MENU_STATE_OPTIONS;
                }
                break;
            case( UI_MENU_STATE_SET_RGBLED):
                if (UI_setRGBLED()) {
                    menuState = UI_MENU_STATE_OPTIONS;
                }
                break;
            case( UI_MENU_STATE_GET_LAST_UPDATE):;

                uint8_t latestTime[64];
                getLatestUpdateTime(&latestTime);
                xSemaphoreTake(sendData, portMAX_DELAY);
                putsUSBUSART((uint8_t*) latestTime);
                memset(rxData, 0, sizeof (rxData));

                menuState = UI_MENU_STATE_OPTIONS;
                break;
        }
    }
    vTaskDelete(NULL);
}

void vApplicationMallocFailedHook(void) {
    /* vApplicationMallocFailedHook() will only be called if
    configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
    function that will get called if a call to pvPortMalloc() fails.
    pvPortMalloc() is called internally by the kernel whenever a task, queue,
    timer or semaphore is created.  It is also called by various parts of the
    demo application.  If heap_1.c or heap_2.c are used, then the size of the
    heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
    FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
    to query the size of free heap space that remains (although it does not
    provide information on how the remaining heap might be fragmented). */
    taskDISABLE_INTERRUPTS();
    for (;;);
}

/*-----------------------------------------------------------*/

void vApplicationIdleHook(void) {
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
    to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
    task.  It is essential that code added to this hook function never attempts
    to block in any way (for example, call xQueueReceive() with a block time
    specified, or call vTaskDelay()).  If the application makes use of the
    vTaskDelete() API function (as this demo application does) then it is also
    important that vApplicationIdleHook() is permitted to return to its calling
    function, because it is the responsibility of the idle task to clean up
    memory allocated by the kernel to any task that has since been deleted. */
}

/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
    (void) pcTaskName;
    (void) pxTask;

    /* Run time task stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook	function is 
    called if a task stack overflow is detected.  Note the system/interrupt
    stack is not checked. */
    taskDISABLE_INTERRUPTS();
    for (;;);
}

/*-----------------------------------------------------------*/

void vApplicationTickHook(void) {
    /* This function will be called by each tick interrupt if
    configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
    added here, but the tick hook is called from an interrupt context, so
    code must not attempt to block, and only the interrupt safe FreeRTOS API
    functions can be used (those that end in FromISR()). */
}

/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

void vAssertCalled(const char * pcFile, unsigned long ulLine) {
    volatile unsigned long ul = 0;

    (void) pcFile;
    (void) ulLine;

    __asm volatile( "di");
    {
        /* Set ul to a non-zero value using the debugger to step out of this
        function. */
        while (ul == 0) {
            portNOP();
        }
    }
    __asm volatile( "ei");
}

/**
 End of File
 */

