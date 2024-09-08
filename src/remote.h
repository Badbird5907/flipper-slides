#pragma once

#include <furi.h>
#include <furi_hal_bt.h>
#include <furi_hal_usb.h> // includes hid_usage_keyboard

#include <extra_profiles/hid_profile.h>

#include <bt/bt_service/bt.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <notification/notification.h>
#include <storage/storage.h>

#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/popup.h>
#include "views/remote_keynote.h"

#include "scenes/remote_scene.h"

#define REMOTE_BT_KEYS_STORAGE_NAME ".bt_remote.keys"

typedef struct Remote Remote;

struct Remote {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    NotificationApp* notifications;
    SceneManager* scene_manager;
    Submenu* submenu;
    DialogEx* dialog;
    Popup* popup;

    Bt* bt;
    FuriHalBleProfileBase* ble_remote_profile;

    RemoteKeynote* remote_keynote;

    bool laser_enabled;
};

void bt_remote_remove_pairing(Remote* app);

void remote_hal_keyboard_press(Remote* instance, uint16_t event);
void remote_hal_keyboard_release(Remote* instance, uint16_t event);
void remote_hal_keyboard_release_all(Remote* instance);

void remote_hal_consumer_key_press(Remote* instance, uint16_t event);
void remote_hal_consumer_key_release(Remote* instance, uint16_t event);
void remote_hal_consumer_key_release_all(Remote* instance);

void remote_hal_mouse_move(Remote* instance, int8_t dx, int8_t dy);
void remote_hal_mouse_scroll(Remote* instance, int8_t delta);
void remote_hal_mouse_press(Remote* instance, uint16_t event);
void remote_hal_mouse_release(Remote* instance, uint16_t event);
void remote_hal_mouse_release_all(Remote* instance);
