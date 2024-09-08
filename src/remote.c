#include "remote.h"
#include "gpio_util.h"

#include <extra_profiles/hid_profile.h>
#include <profiles/serial_profile.h>
#include <notification/notification_messages.h>

#include "views.h"

#define TAG "prez_remote"

bool remote_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    Remote* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

bool remote_back_event_callback(void* context) {
    furi_assert(context);
    Remote* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

void bt_remote_remove_pairing(Remote* app) {
    Bt* bt = app->bt;
    bt_disconnect(bt);

    // Wait 2nd core to update nvm storage
    furi_delay_ms(200);

    furi_hal_bt_stop_advertising();

    bt_forget_bonded_devices(bt);

    furi_hal_bt_start_advertising();
}

static void bt_remote_connection_status_changed_callback(BtStatus status, void* context) {
    furi_assert(context);
    Remote* app = context;
    const bool connected = (status == BtStatusConnected);
    notification_internal_message(
        app->notifications, connected ? &sequence_set_blue_255 : &sequence_reset_blue);
    // hid_keynote_set_connected_status(app->hid_keynote, connected);
}

Remote* remote_alloc() {
    Remote* app = malloc(sizeof(Remote));
    
    app->laser_enabled = true;

    app->gui = furi_record_open(RECORD_GUI);
    app->bt = furi_record_open(RECORD_BT);

    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, remote_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, remote_back_event_callback);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->scene_manager = scene_manager_alloc(&remote_scene_handlers, app);

    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, RemoteViewSubmenu, submenu_get_view(app->submenu));

    app->dialog = dialog_ex_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, RemoteViewDialog, dialog_ex_get_view(app->dialog));

    app->popup = popup_alloc();
    view_dispatcher_add_view(app->view_dispatcher, RemoteViewPopup, popup_get_view(app->popup));

    app->remote_keynote = remote_keynote_alloc(app);
    view_dispatcher_add_view(
        app->view_dispatcher, RemoteViewKeynote, remote_keynote_get_view(app->remote_keynote)); 
    return app;
}

void remote_free(Remote* app) {
    furi_assert(app);

    notification_internal_message(app->notifications, &sequence_reset_blue);
    view_dispatcher_remove_view(app->view_dispatcher, RemoteViewSubmenu);
    submenu_free(app->submenu);
    view_dispatcher_remove_view(app->view_dispatcher, RemoteViewDialog);
    dialog_ex_free(app->dialog);
    view_dispatcher_remove_view(app->view_dispatcher, RemoteViewPopup);
    popup_free(app->popup);

    view_dispatcher_remove_view(app->view_dispatcher, RemoteViewKeynote);
    remote_keynote_free(app->remote_keynote);

    scene_manager_free(app->scene_manager);
    view_dispatcher_free(app->view_dispatcher);

    // Close records
    furi_record_close(RECORD_GUI);
    app->gui = NULL;
    furi_record_close(RECORD_NOTIFICATION);
    app->notifications = NULL;
    furi_record_close(RECORD_BT);
    app->bt = NULL;

    // Free rest
    free(app);
}

int32_t presentation_remote_app(void* p) {
    UNUSED(p);
    Remote* app = remote_alloc();
    FURI_LOG_I(TAG, "Starting presentation remote app");

    attempt_set_5v_state(false);

    bt_disconnect(app->bt);
    furi_delay_ms(200);
    bt_keys_storage_set_storage_path(app->bt, APP_DATA_PATH(REMOTE_BT_KEYS_STORAGE_NAME));

    app->ble_remote_profile = bt_profile_start(app->bt, ble_profile_hid, NULL);
    furi_check(app->ble_remote_profile);

    furi_hal_bt_start_advertising();
    bt_set_status_changed_callback(app->bt, bt_remote_connection_status_changed_callback, app);

    scene_manager_next_scene(app->scene_manager, RemoteSceneStart);

    view_dispatcher_run(app->view_dispatcher);

    bt_set_status_changed_callback(app->bt, NULL, NULL);

    // cleanup
    bt_disconnect(app->bt);
    furi_delay_ms(200);

    bt_keys_storage_set_default_path(app->bt);
    furi_check(bt_profile_restore_default(app->bt));
    remote_free(app);
    return 0;
}
