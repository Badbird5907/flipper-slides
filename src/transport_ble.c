#include "remote.h"

void remote_hal_keyboard_press(Remote* instance, uint16_t event) {
    furi_assert(instance);
    ble_profile_hid_kb_press(instance->ble_remote_profile, event);
}

void remote_hal_keyboard_release(Remote* instance, uint16_t event) {
    furi_assert(instance);
    ble_profile_hid_kb_release(instance->ble_remote_profile, event);
}

void remote_hal_keyboard_release_all(Remote* instance) {
    furi_assert(instance);
    ble_profile_hid_kb_release_all(instance->ble_remote_profile);
}

void remote_hal_consumer_key_press(Remote* instance, uint16_t event) {
    furi_assert(instance);
    ble_profile_hid_consumer_key_press(instance->ble_remote_profile, event);
}

void remote_hal_consumer_key_release(Remote* instance, uint16_t event) {
    furi_assert(instance);
    ble_profile_hid_consumer_key_release(instance->ble_remote_profile, event);
}

void remote_hal_consumer_key_release_all(Remote* instance) {
    furi_assert(instance);
    ble_profile_hid_consumer_key_release_all(instance->ble_remote_profile);
}

void remote_hal_mouse_move(Remote* instance, int8_t dx, int8_t dy) {
    furi_assert(instance);
    ble_profile_hid_mouse_move(instance->ble_remote_profile, dx, dy);
}

void remote_hal_mouse_scroll(Remote* instance, int8_t delta) {
    furi_assert(instance);
    ble_profile_hid_mouse_scroll(instance->ble_remote_profile, delta);
}

void remote_hal_mouse_press(Remote* instance, uint16_t event) {
    furi_assert(instance);
    ble_profile_hid_mouse_press(instance->ble_remote_profile, event);
}

void remote_hal_mouse_release(Remote* instance, uint16_t event) {
    furi_assert(instance);
    ble_profile_hid_mouse_release(instance->ble_remote_profile, event);
}

void remote_hal_mouse_release_all(Remote* instance) {
    furi_assert(instance);
    ble_profile_hid_mouse_release_all(instance->ble_remote_profile);
}
