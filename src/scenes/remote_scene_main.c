#include "../remote.h"
#include "../views.h"

void remote_scene_main_on_enter(void* context) {
    Remote* app = context;
    view_dispatcher_switch_to_view(
        app->view_dispatcher, scene_manager_get_scene_state(app->scene_manager, RemoteSceneMain));
}

bool remote_scene_main_on_event(void* context, SceneManagerEvent event) {
    Remote* app = context;
    bool consumed = false;
    UNUSED(app);
    UNUSED(event);

    return consumed;
}

void remote_scene_main_on_exit(void* context) {
    Remote* app = context;
    UNUSED(app);
}
