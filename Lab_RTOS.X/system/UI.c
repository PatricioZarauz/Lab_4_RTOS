/*
    (c) 2017 Microchip Technology Inc. and its subsidiaries. You may use this
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

/*
    MCC USB CDC Demo Usage:
 
    Call the MCC_USB_CDC_DemoTasks() function from your main loop.
    It will read data sent from the host and echo it back +1.  If you open
    up a terminal window and type 'a' then 'b' will be echoed back.
 */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "../freeRTOS/include/FreeRTOS.h"
#include "UI.h"
#include "../mcc_generated_files/usb/usb.h"
#include "../utils/USB.h"
#include "../plataform/appTime.h"
#include "register.h"
#include "../freeRTOS/include/semphr.h"

extern xSemaphoreHandle sendData;
extern xSemaphoreHandle readData;
extern uint8_t textoNoValido[];
extern uint8_t rxData[24];
/*
const uint8_t textoBienvenida[] = "Laboratorio 3 - Equipo 10\n\t- Patricio Zarauz\n\t- Gast�n Salustio\n\t- Sebasti�n Reynosa";
const uint8_t textoOpciones[] = "Seleccione una opcion:\n\n\t1. Fijar Fecha y Hora\n\t2. Encender o apagar LED especifico\n\t3. Consultar modificacion del ultimo LED:\n";*/
/*const uint8_t textoSegundo[] = "Segundo:\n";
const uint8_t textoMinuto[] = "Minuto:\n";
const uint8_t textoHora[] = "Hora:\n";
const uint8_t textoDia[] = "Dia:\n";
const uint8_t textoMes[] = "Mes:\n";
const uint8_t textoAno[] = "A�o:\n";*/
//const uint8_t textoLed[] = "LED: (1-8)\n";
//const uint8_t textoColor[] = "Color: (0-4)\n";
//const uint8_t textoNoValido[] = "La opcion no es correcta, ingrese nuevamente \n";

static bool isValidYear(int yr) {
    return (yr >= 1900);
}
/*
void UI_showMenu() {
    static ui_menu_states_t menuState = UI_MENU_STATE_INIT;


    //if (IsUSBConected()) {
    switch (menuState) {
        case( UI_MENU_STATE_INIT):

            if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
                if ((UI_checkValidOption(rxData, 0, 0))) {
                    if (USBSend((uint8_t*) textoBienvenida)) {
                        //menuState = UI_MENU_STATE_OPTIONS;
                        memset(rxData, 0, sizeof (rxData));
                    }
                }
                menuState = EJEMPLO;
                xSemaphoreGive(xMutex);
            }
            break;
        case( UI_MENU_STATE_OPTIONS):
            if (USBSend((uint8_t*) textoOpciones)) {
                memset(rxData, 0, sizeof (rxData));
            }
            if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
                if ((UI_checkValidOption(rxData, 1, 3))) {
                    menuState = UI_MENU_STATE_OPTIONS + atoi(rxData);
                } else {
                    USBSend((uint8_t*) textoNoValido);
                    menuState = UI_MENU_STATE_OPTIONS;
                }
                xSemaphoreGive(xMutex);
            }
            break;

        case( UI_MENU_STATE_SET_TIMEDATE):
            if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
                if (UI_setTimedate(rxData)) {
                    menuState = UI_MENU_STATE_OPTIONS;
                }
                xSemaphoreGive(xMutex);
            }
            break;
        case( UI_MENU_STATE_SET_RGBLED):
            if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
                if (UI_setRGBLED(rxData)) {
                    menuState = UI_MENU_STATE_OPTIONS;
                }
                xSemaphoreGive(xMutex);
            }
            break;
        case( UI_MENU_STATE_GET_LAST_UPDATE):;
            uint8_t *res = getLatestUpdateTime();
            if (USBSend(res)) {
                memset(rxData, 0, sizeof (rxData));
            }
            menuState = UI_MENU_STATE_OPTIONS;
            break;
        default:
            break;
    }
    } else {
        menuState = UI_MENU_STATE_INIT;
    }
}
 */

/*
bool UI_waitForInput(uint8_t *dest) {
    uint8_t bytesReceived;

    bytesReceived = USBReceive(dest);
    if (bytesReceived > 0) {
        dest[bytesReceived] = '\0'; // End of String
        return true;
    }
    return false;
}*/

bool UI_checkValidOption(uint8_t *src, uint32_t min, uint32_t max) {
    uint32_t intValue;
    uint32_t i;

    for (i = 0; i < strlen(src); i++) {
        if (isdigit(src[i]) == 0) {
            return false;
        }
    }
    intValue = atoi(src);
    if ((intValue < min) || (intValue > max)) {
        return false;
    }
    return true;
}

bool UI_setTimedate() {
    /*static ui_date_states_t dateState = UI_DATE_STATE_SEC;
    static struct tm date;

    switch (dateState) {
        case UI_DATE_STATE_SEC:
            xSemaphoreTake(sendData, portMAX_DELAY);
            putsUSBUSART((uint8_t*) textoSegundo);

            memset(rxData, 0, sizeof (rxData));


            if (UI_checkValidOption(rxData, 0, 59)) {
                date.tm_sec = atoi(rxData);
                dateState = UI_DATE_STATE_MIN;
            } else {
                xSemaphoreTake(sendData, portMAX_DELAY);
                putsUSBUSART((uint8_t*) textoNoValido);
                dateState = UI_DATE_STATE_SEC;
            }
            break;

        case UI_DATE_STATE_MIN:
            xSemaphoreTake(sendData, portMAX_DELAY);
            putsUSBUSART((uint8_t*) textoMinuto);

            memset(rxData, 0, sizeof (rxData));


            if (UI_checkValidOption(rxData, 0, 59)) {
                date.tm_min = atoi(rxData);
                dateState = UI_DATE_STATE_HR;
            } else {
                xSemaphoreTake(sendData, portMAX_DELAY);
                putsUSBUSART((uint8_t*) textoNoValido);
                dateState = UI_DATE_STATE_MIN;
            }

            break;
        case UI_DATE_STATE_HR:

            xSemaphoreTake(sendData, portMAX_DELAY);
            putsUSBUSART((uint8_t*) textoHora);
            memset(rxData, 0, sizeof (rxData));


            if (UI_checkValidOption(rxData, 0, 23)) {
                date.tm_hour = atoi(rxData);
                dateState = UI_DATE_STATE_DAY;
            } else {
                xSemaphoreTake(sendData, portMAX_DELAY);
                putsUSBUSART((uint8_t*) textoNoValido);
                dateState = UI_DATE_STATE_HR;
            }


            break;
        case UI_DATE_STATE_DAY:

            xSemaphoreTake(sendData, portMAX_DELAY);
            putsUSBUSART((uint8_t*) textoDia);
            memset(rxData, 0, sizeof (rxData));

            if (UI_checkValidOption(rxData, 1, 31)) {
                date.tm_mday = atoi(rxData);
                dateState = UI_DATE_STATE_MON;
            } else {
                xSemaphoreTake(sendData, portMAX_DELAY);
                putsUSBUSART((uint8_t*) textoNoValido);
                dateState = UI_DATE_STATE_DAY;
            }


            break;
        case UI_DATE_STATE_MON:
            xSemaphoreTake(sendData, portMAX_DELAY);
            putsUSBUSART((uint8_t*) textoMes);
            memset(rxData, 0, sizeof (rxData));

            if (UI_checkValidOption(rxData, 1, 12)) {
                date.tm_mon = atoi(rxData) - 1;
                dateState = UI_DATE_STATE_YR;
            } else {
                xSemaphoreTake(sendData, portMAX_DELAY);
                putsUSBUSART((uint8_t*) textoNoValido);
                dateState = UI_DATE_STATE_MON;
            }


            break;
        case UI_DATE_STATE_YR:

            xSemaphoreTake(sendData, portMAX_DELAY);
            putsUSBUSART((uint8_t*) textoAno);
            memset(rxData, 0, sizeof (rxData));


            if (isValidYear(atoi(rxData))) {
                date.tm_year = atoi(rxData) - 1900;
                dateState = UI_DATE_STATE_SEC;
                setTime(&date);
                return true;
            } else {
                xSemaphoreTake(sendData, portMAX_DELAY);
                putsUSBUSART((uint8_t*) textoNoValido);
                dateState = UI_DATE_STATE_YR;
            }


            break;
    }*/
    return false;
}

bool UI_setRGBLED(uint8_t *src) {
    /*static ui_rgb_led_states_t rgbLedState = UI_RGB_LED_STATE_LED;

    switch (rgbLedState) {
        case UI_RGB_LED_STATE_LED:
            if (USBSend((uint8_t*) textoLed)) {
                memset(src, 0, sizeof (src));
            }
            if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
                if (UI_checkValidOption(src, 1, 8)) {
                    setLatestLED(atoi(src) - 1);
                    rgbLedState = UI_RGB_LED_STATE_COLOR;
                } else {
                    USBSend((uint8_t*) textoNoValido);
                    rgbLedState = UI_RGB_LED_STATE_LED;
                }
            }
            break;
        case UI_RGB_LED_STATE_COLOR:
            if (USBSend((uint8_t*) textoColor)) {
                memset(src, 0, sizeof (src));
            }
            if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
                if (UI_checkValidOption(src, 0, 4)) {
                    setLatestColor(atoi(src));
                    setRGBLEDAndTime();
                    rgbLedState = UI_RGB_LED_STATE_LED;
                    return true;
                } else {
                    USBSend((uint8_t*) textoNoValido);
                    rgbLedState = UI_RGB_LED_STATE_COLOR;
                }
            }
            break;
    }
    return false;*/
}
