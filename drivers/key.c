#include "key.h"

#define KEY_DEBOUNCE_TIME 2

static uint8_t k1_cnt = 0, k1_trig = 0;
static uint8_t k2_cnt = 0, k2_trig = 0;
static uint8_t k3_cnt = 0, k3_trig = 0;
static uint8_t k4_cnt = 0, k4_trig = 0;

void KEY_Scan(void)
{
    if (DL_GPIO_readPins(GPIO_KEY_PIN_KEY1_PORT, GPIO_KEY_PIN_KEY1_PIN) == 0) {
        if (k1_cnt < KEY_DEBOUNCE_TIME) k1_cnt++;
        if (k1_cnt >= KEY_DEBOUNCE_TIME && !k1_trig) {
            key = 1;
            k1_trig = 1;
        }
    } else {
        k1_cnt = 0;
        k1_trig = 0;
    }

    if (DL_GPIO_readPins(GPIO_KEY_PIN_KEY2_PORT, GPIO_KEY_PIN_KEY2_PIN) == 0) {
        if (k2_cnt < KEY_DEBOUNCE_TIME) k2_cnt++;
        if (k2_cnt >= KEY_DEBOUNCE_TIME && !k2_trig) {
            key = 2;
            k2_trig = 1;
        }
    } else {
        k2_cnt = 0;
        k2_trig = 0;
    }

    if (DL_GPIO_readPins(GPIO_KEY_PIN_KEY3_PORT, GPIO_KEY_PIN_KEY3_PIN) == 0) {
        if (k3_cnt < KEY_DEBOUNCE_TIME) k3_cnt++;
        if (k3_cnt >= KEY_DEBOUNCE_TIME && !k3_trig) {
            key = 3;
            k3_trig = 1;
        }
    } else {
        k3_cnt = 0;
        k3_trig = 0;
    }

    if (DL_GPIO_readPins(GPIO_KEY_PIN_KEY4_PORT, GPIO_KEY_PIN_KEY4_PIN) == 0) {
        if (k4_cnt < KEY_DEBOUNCE_TIME) k4_cnt++;
        if (k4_cnt >= KEY_DEBOUNCE_TIME && !k4_trig) {
            key = 4;
            k4_trig = 1;
        }
    } else {
        k4_cnt = 0;
        k4_trig = 0;
    }
}
