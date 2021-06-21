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
#include "register.h"
#include "../freeRTOS/include/semphr.h"
#include "../utils/text.h"

extern xSemaphoreHandle sendData;
extern xSemaphoreHandle readData;
extern xSemaphoreHandle xTimer;
extern uint8_t rxData[24];

bool isValidYear(int yr) {
    return (yr >= 1900);
}

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
    static ui_date_states_t dateState = UI_DATE_STATE_SEC;
    static struct tm date;

    switch (dateState) {
        case UI_DATE_STATE_SEC:
            xSemaphoreTake(sendData, portMAX_DELAY);
            putsUSBUSART((uint8_t*) textoSegundo);

            memset(rxData, 0, sizeof (rxData));

            xSemaphoreTake(readData, portMAX_DELAY);
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

            xSemaphoreTake(readData, portMAX_DELAY);
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

            xSemaphoreTake(readData, portMAX_DELAY);
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

            xSemaphoreTake(readData, portMAX_DELAY);
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

            xSemaphoreTake(readData, portMAX_DELAY);
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

            xSemaphoreTake(readData, portMAX_DELAY);
            if (isValidYear(atoi(rxData))) {
                date.tm_year = atoi(rxData) - 1900;
                dateState = UI_DATE_STATE_SEC;
                xSemaphoreTake(xTimer, portMAX_DELAY);
                RTCC_TimeSet(&date);
                return true;
            } else {
                xSemaphoreTake(sendData, portMAX_DELAY);
                putsUSBUSART((uint8_t*) textoNoValido);
                dateState = UI_DATE_STATE_YR;
            }

            break;
    }
    return false;
}

bool UI_setRGBLED() {
    static ui_rgb_led_states_t rgbLedState = UI_RGB_LED_STATE_LED;

    switch (rgbLedState) {
        case UI_RGB_LED_STATE_LED:
            xSemaphoreTake(sendData, portMAX_DELAY);
            putsUSBUSART((uint8_t*) textoLed);
            memset(rxData, 0, sizeof (rxData));

            xSemaphoreTake(readData, portMAX_DELAY);
            if (UI_checkValidOption(rxData, 1, 8)) {
                setLatestLED(atoi(rxData) - 1);
                rgbLedState = UI_RGB_LED_STATE_COLOR;
            } else {
                xSemaphoreTake(sendData, portMAX_DELAY);
                putsUSBUSART((uint8_t*) textoNoValido);
                rgbLedState = UI_RGB_LED_STATE_LED;
            }

            break;
        case UI_RGB_LED_STATE_COLOR:
            xSemaphoreTake(sendData, portMAX_DELAY);
            putsUSBUSART((uint8_t*) textoColor);
            memset(rxData, 0, sizeof (rxData));

            xSemaphoreTake(readData, portMAX_DELAY);
            if (UI_checkValidOption(rxData, 0, 4)) {
                setLatestColor(atoi(rxData));
                setRGBLEDAndTime();
                rgbLedState = UI_RGB_LED_STATE_LED;
                return true;
            } else {
                xSemaphoreTake(sendData, portMAX_DELAY);
                putsUSBUSART((uint8_t*) textoNoValido);
                rgbLedState = UI_RGB_LED_STATE_COLOR;
            }

            break;
    }
    return false;
}
