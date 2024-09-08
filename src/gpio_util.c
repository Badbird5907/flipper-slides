#include "gpio_util.h"
#include <furi.h>
#include <furi_hal.h>

#define TAG "gpio_util"

bool attempt_set_5v_state(bool enable) {
    if(furi_hal_power_is_otg_enabled() == enable) {
        return true;
    }
    if (furi_hal_power_is_charging()) {
        FURI_LOG_W(TAG, "Charging detected, cannot change 5V state");
        return false;
    }
    bool success = false;
    uint8_t attempts = 5;

    while(attempts-- > 0) {
        FURI_LOG_T(TAG, "Setting 5V state to %s", enable ? "ON" : "OFF");
        if(enable) {
            FURI_LOG_T(TAG, "Enabling 5V state");
            if(furi_hal_power_enable_otg()) {
                FURI_LOG_T(TAG, "OTG enabled");
                success = true;
                break;
            }
        } else {
            FURI_LOG_T(TAG, "Disabling 5V state");
            furi_hal_power_disable_otg();
        }
    }

    if(!success) {
        FURI_LOG_E(TAG, "Failed to set 5V state to %s", enable ? "ON" : "OFF");
    }

    return success;
}
