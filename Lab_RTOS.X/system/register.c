#include <time.h>
#include "register.h"
#include "../plataform/LED_RGB.h"
#include "../utils/USB.h"
#include "FreeRTOS.h"
#include "../freeRTOS/include/semphr.h"
#include <stdio.h>

extern xSemaphoreHandle xTimer;
extern time_t timeStamp;
static app_register_t registro;

void setLatestLED(uint8_t latestLed) {
    registro.led = latestLed;
}

void setLatestColor(uint8_t latestColor) {
    registro.color = latestColor;
}

void setRGBLEDAndTime(void) {
    RGB_setLedColor(registro.led, registro.color);
    RGB_showLeds(AMOUNT_RGB_LED);
    xSemaphoreTake(xTimer, portMAX_DELAY);
    registro.time = timeStamp;
}

void getLatestUpdateTime(uint8_t* latestUpdateTime) {
    xSemaphoreTake(xTimer, portMAX_DELAY);

    time_t raw_time = (time_t) registro.time;
    struct tm *timeinfo = localtime(&raw_time);

    sprintf(latestUpdateTime, "%s\n", asctime(timeinfo));
}

/* *****************************************************************************
 End of File
 */
