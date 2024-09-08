#include "../remote.h"
#include "../views.h"
#include "remote_icons.h"

static void remote_scene_unpair_dialog_callback(DialogExResult result, void* context) {
    Remote* app = context;

    if(result == DialogExResultRight) {
        // Unpair all devices
        bt_remote_remove_pairing(app);

        // Show popup
        view_dispatcher_switch_to_view(app->view_dispatcher, RemoteViewPopup);
    } else if(result == DialogExResultLeft) {
        scene_manager_previous_scene(app->scene_manager);
    }
}

void remote_scene_unpair_popup_callback(void* context) {
    Remote* app = context;

    scene_manager_previous_scene(app->scene_manager);
}

void remote_scene_unpair_on_enter(void* context) {
    Remote* app = context;

    // Un-pair dialog view
    dialog_ex_reset(app->dialog);
    dialog_ex_set_result_callback(app->dialog, remote_scene_unpair_dialog_callback);
    dialog_ex_set_context(app->dialog, app);
    dialog_ex_set_header(app->dialog, "Unpair the Device?", 64, 3, AlignCenter, AlignTop);
    dialog_ex_set_left_button_text(app->dialog, "Back");
    dialog_ex_set_right_button_text(app->dialog, "Unpair");

    // Un-pair success popup view
    popup_set_icon(app->popup, 48, 6, &I_DolphinDone_80x58);
    popup_set_header(app->popup, "Done", 14, 15, AlignLeft, AlignTop);
    popup_set_timeout(app->popup, 1500);
    popup_set_context(app->popup, app);
    popup_set_callback(app->popup, remote_scene_unpair_popup_callback);
    popup_enable_timeout(app->popup);

    view_dispatcher_switch_to_view(app->view_dispatcher, RemoteViewDialog);
}

bool remote_scene_unpair_on_event(void* context, SceneManagerEvent event) {
    Remote* app = context;
    bool consumed = false;
    UNUSED(app);
    UNUSED(event);

    return consumed;
}

void remote_scene_unpair_on_exit(void* context) {
    Remote* app = context;

    dialog_ex_reset(app->dialog);
    popup_reset(app->popup);
}
