#include "../remote.h"
#include "../views.h"

#define TAG "RemoteSceneStart"

enum RemoteSubmenuIndex {
    RemoteSubmenuIndexKeynoteVertical,
    RemoteSubmenuIndexRemovePairing,
    RemoteSubmenuIndexLaser,
};

static void remote_scene_start_submenu_callback(void* context, uint32_t index) {
    furi_assert(context);
    Remote* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void remote_scene_start_on_enter(void* context) {
    Remote* app = context;
    submenu_add_item(
        app->submenu,
        "Keynote Vertical",
        RemoteSubmenuIndexKeynoteVertical,
        remote_scene_start_submenu_callback,
        app);
    /*char* laser_text = app->laser_enabled ? "Laser: Enabled" : "Laser: Disabled";
    submenu_add_item(
        app->submenu,
        laser_text,
        RemoteSubmenuIndexLaser,
        remote_scene_start_submenu_callback,
        app);*/
    submenu_add_item(
        app->submenu,
        "Bluetooth Unpairing",
        RemoteSubmenuIndexRemovePairing,
        remote_scene_start_submenu_callback,
        app);
    submenu_set_selected_item(
        app->submenu, scene_manager_get_scene_state(app->scene_manager, RemoteSceneStart));
    view_dispatcher_switch_to_view(app->view_dispatcher, RemoteViewSubmenu);
}

bool remote_scene_start_on_event(void* context, SceneManagerEvent event) {
    Remote* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == RemoteSubmenuIndexRemovePairing) {
            scene_manager_next_scene(app->scene_manager, RemoteSceneUnpair);
        } else if(event.event == RemoteSubmenuIndexLaser) {
            FURI_LOG_I(TAG, "Laser toggled");
            app->laser_enabled = !app->laser_enabled;
            scene_manager_next_scene(app->scene_manager, RemoteSceneStart);
            consumed = true;
        } else {
            RemoteView view_id;

            switch(event.event) {
            case RemoteSubmenuIndexKeynoteVertical:
                view_id = RemoteViewKeynote;
                remote_keynote_set_orientation(app->remote_keynote, true);
                break;
            default:
                furi_crash();
            }

            scene_manager_set_scene_state(app->scene_manager, RemoteSceneMain, view_id);
            scene_manager_next_scene(app->scene_manager, RemoteSceneMain);
        }

        scene_manager_set_scene_state(app->scene_manager, RemoteSceneStart, event.event);
        consumed = true;
    }

    return consumed;
}

void remote_scene_start_on_exit(void* context) {
    Remote* app = context;
    submenu_reset(app->submenu);
}
