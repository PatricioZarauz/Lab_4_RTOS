#include "FreeRTOS.h"
#include "task.h"
#include "../mcc_generated_files/rtcc.h"

static struct tm timeDate;
static time_t timeStamp;

void updateTime(void) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    if (RTCC_TimeGet(&timeDate)) {
        timeStamp = mktime(&timeDate);
    }
}

time_t getTimestamp(void) {
    return timeStamp;
}

void setTime(struct tm *newTimeDate){
    RTCC_TimeSet(newTimeDate);
}

/* *****************************************************************************
 End of File
 */
