#ifndef _UI_H    /* Guard against multiple inclusion */
#define _UI_H


typedef enum {
    UI_MENU_STATE_INIT,
    UI_MENU_STATE_OPTIONS,
    UI_MENU_STATE_SET_TIMEDATE,
    UI_MENU_STATE_SET_RGBLED,
    UI_MENU_STATE_GET_LAST_UPDATE,
    EJEMPLO,
} ui_menu_states_t;

typedef enum {
    UI_DATE_STATE_SEC,
    UI_DATE_STATE_MIN,
    UI_DATE_STATE_HR,
    UI_DATE_STATE_DAY,
    UI_DATE_STATE_MON,
    UI_DATE_STATE_YR,
} ui_date_states_t;

typedef enum {
    UI_RGB_LED_STATE_LED,
    UI_RGB_LED_STATE_COLOR,
} ui_rgb_led_states_t;




void UI_showMenu();

bool UI_waitForInput(uint8_t *dest);

bool UI_checkValidOption(uint8_t *src, uint32_t min, uint32_t max);

bool UI_setTimedate(uint8_t *src);

bool UI_setRGBLED(uint8_t *src);

#endif /* _UI_H */

/* *****************************************************************************
 End of File
 */
