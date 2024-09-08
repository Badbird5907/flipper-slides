#include "remote_keynote.h"
#include <gui/elements.h>
#include "../remote.h"
#include <furi_hal_usb_hid.h>
#include "remote_icons.h"
#include <furi_hal_rtc.h>

#include "../gpio_util.h"

#define TAG "RemoteKeynote"
#define SPAM_BACK_DELAY_MS \
    400 // if back is pressed 3 times, with each press less than 300ms apart, it will exit

typedef struct {
    uint8_t back_press_count;
    uint32_t last_back_press_time;
} ExitState;

struct RemoteKeynote {
    View* view;
    Remote* remote;
    ExitState* exit_state;
};

typedef struct {
    bool left_pressed;
    bool up_pressed;
    bool right_pressed;
    bool down_pressed;
    bool ok_pressed;
    bool back_pressed;
    bool connected;

    bool laser_on;
    bool laser_keep_on;
    bool short_expect_release;
    bool laser_disabled;
} RemoteKeynoteModel;

static void remote_keynote_draw_arrow(Canvas* canvas, uint8_t x, uint8_t y, CanvasDirection dir) {
    canvas_draw_triangle(canvas, x, y, 5, 3, dir);
    if(dir == CanvasDirectionBottomToTop) {
        canvas_draw_line(canvas, x, y + 6, x, y - 1);
    } else if(dir == CanvasDirectionTopToBottom) {
        canvas_draw_line(canvas, x, y - 6, x, y + 1);
    } else if(dir == CanvasDirectionRightToLeft) {
        canvas_draw_line(canvas, x + 6, y, x - 1, y);
    } else if(dir == CanvasDirectionLeftToRight) {
        canvas_draw_line(canvas, x - 6, y, x + 1, y);
    }
}

static void remote_keynote_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    RemoteKeynoteModel* model = context;

    // Header
    if(model->connected) {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_connected_15x15);
    } else {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_disconnected_15x15);
    }

    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 17, 3, AlignLeft, AlignTop, "Keynote");

    canvas_draw_icon(canvas, 68, 2, &I_Pin_back_arrow_10x8);
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(canvas, 127, 3, AlignRight, AlignTop, "x3 to exit");

    // Up
    canvas_draw_icon(canvas, 21, 24, &I_Button_18x18);
    if(model->up_pressed) {
        elements_slightly_rounded_box(canvas, 24, 26, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    remote_keynote_draw_arrow(canvas, 30, 30, CanvasDirectionBottomToTop);
    canvas_set_color(canvas, ColorBlack);

    // Down
    canvas_draw_icon(canvas, 21, 45, &I_Button_18x18);
    if(model->down_pressed) {
        elements_slightly_rounded_box(canvas, 24, 47, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    remote_keynote_draw_arrow(canvas, 30, 55, CanvasDirectionTopToBottom);
    canvas_set_color(canvas, ColorBlack);

    // Left
    canvas_draw_icon(canvas, 0, 45, &I_Button_18x18);
    if(model->left_pressed) {
        elements_slightly_rounded_box(canvas, 3, 47, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    remote_keynote_draw_arrow(canvas, 7, 53, CanvasDirectionRightToLeft);
    canvas_set_color(canvas, ColorBlack);

    // Right
    canvas_draw_icon(canvas, 42, 45, &I_Button_18x18);
    if(model->right_pressed) {
        elements_slightly_rounded_box(canvas, 45, 47, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    remote_keynote_draw_arrow(canvas, 53, 53, CanvasDirectionLeftToRight);
    canvas_set_color(canvas, ColorBlack);

    // Ok
    canvas_draw_icon(canvas, 63, 24, &I_Space_65x18);
    if(model->ok_pressed) {
        elements_slightly_rounded_box(canvas, 66, 26, 60, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 74, 28, &I_Ok_btn_9x9);
    elements_multiline_text_aligned(canvas, 91, 36, AlignLeft, AlignBottom, "Space");
    canvas_set_color(canvas, ColorBlack);

    // Back
    canvas_draw_icon(canvas, 63, 45, &I_Space_65x18);
    if(model->back_pressed || model->laser_keep_on) {
        elements_slightly_rounded_box(canvas, 66, 47, 60, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 74, 49, &I_Pin_back_arrow_10x8);
    elements_multiline_text_aligned(
        canvas, 91, 57, AlignLeft, AlignBottom, model->laser_disabled ? "Back" : "Laser");
}

static void remote_keynote_draw_vertical_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    RemoteKeynoteModel* model = context;

    // Header
    if(model->connected) {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_connected_15x15);
    } else {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_disconnected_15x15);
    }
    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 20, 3, AlignLeft, AlignTop, "Keynote");

    canvas_draw_icon(canvas, 2, 18, &I_Pin_back_arrow_10x8);
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(canvas, 15, 19, AlignLeft, AlignTop, "x3 to exit");

    const uint8_t x_2 = 23;
    const uint8_t x_1 = 2;
    const uint8_t x_3 = 44;

    const uint8_t y_1 = 44;
    const uint8_t y_2 = 65;

    // Up
    canvas_draw_icon(canvas, x_2, y_1, &I_Button_18x18);
    if(model->up_pressed) {
        elements_slightly_rounded_box(canvas, x_2 + 3, y_1 + 2, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    remote_keynote_draw_arrow(canvas, x_2 + 9, y_1 + 6, CanvasDirectionBottomToTop);
    canvas_set_color(canvas, ColorBlack);

    // Down
    canvas_draw_icon(canvas, x_2, y_2, &I_Button_18x18);
    if(model->down_pressed) {
        elements_slightly_rounded_box(canvas, x_2 + 3, y_2 + 2, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    remote_keynote_draw_arrow(canvas, x_2 + 9, y_2 + 10, CanvasDirectionTopToBottom);
    canvas_set_color(canvas, ColorBlack);

    // Left
    canvas_draw_icon(canvas, x_1, y_2, &I_Button_18x18);
    if(model->left_pressed) {
        elements_slightly_rounded_box(canvas, x_1 + 3, y_2 + 2, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    remote_keynote_draw_arrow(canvas, x_1 + 7, y_2 + 8, CanvasDirectionRightToLeft);
    canvas_set_color(canvas, ColorBlack);

    // Right
    canvas_draw_icon(canvas, x_3, y_2, &I_Button_18x18);
    if(model->right_pressed) {
        elements_slightly_rounded_box(canvas, x_3 + 3, y_2 + 2, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    remote_keynote_draw_arrow(canvas, x_3 + 11, y_2 + 8, CanvasDirectionLeftToRight);
    canvas_set_color(canvas, ColorBlack);

    // Ok
    canvas_draw_icon(canvas, 2, 86, &I_Space_6018); // 60x18
    if(model->ok_pressed) {
        elements_slightly_rounded_box(canvas, 5, 88, 55, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 11, 90, &I_Ok_btn_9x9);
    elements_multiline_text_aligned(canvas, 26, 98, AlignLeft, AlignBottom, "Space");
    canvas_set_color(canvas, ColorBlack);

    // Back
    canvas_draw_icon(canvas, 2, 107, &I_Space_6018); // 60x18
    if(model->back_pressed || model->laser_keep_on) {
        elements_slightly_rounded_box(canvas, 5, 109, 55, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 11, 111, &I_Pin_back_arrow_10x8);
    elements_multiline_text_aligned(
        canvas, 26, 119, AlignLeft, AlignBottom, model->laser_disabled ? "Back" : "Laser");
}
static void remote_keynote_process(RemoteKeynote* remote_keynote, InputEvent* event) {
    with_view_model(
        remote_keynote->view,
        RemoteKeynoteModel * model,
        {
            if(event->type == InputTypePress) {
                if(event->key == InputKeyUp) {
                    model->up_pressed = true;
                    remote_hal_keyboard_press(remote_keynote->remote, HID_KEYBOARD_UP_ARROW);
                } else if(event->key == InputKeyDown) {
                    model->down_pressed = true;
                    remote_hal_keyboard_press(remote_keynote->remote, HID_KEYBOARD_DOWN_ARROW);
                } else if(event->key == InputKeyLeft) {
                    model->left_pressed = true;
                    remote_hal_keyboard_press(remote_keynote->remote, HID_KEYBOARD_LEFT_ARROW);
                } else if(event->key == InputKeyRight) {
                    model->right_pressed = true;
                    remote_hal_keyboard_press(remote_keynote->remote, HID_KEYBOARD_RIGHT_ARROW);
                } else if(event->key == InputKeyOk) {
                    model->ok_pressed = true;
                    remote_hal_keyboard_press(remote_keynote->remote, HID_KEYBOARD_SPACEBAR);
                } else if(event->key == InputKeyBack) {
                    model->back_pressed = true;
                }
            } else if(event->type == InputTypeRelease) {
                if(event->key == InputKeyUp) {
                    model->up_pressed = false;
                    remote_hal_keyboard_release(remote_keynote->remote, HID_KEYBOARD_UP_ARROW);
                } else if(event->key == InputKeyDown) {
                    model->down_pressed = false;
                    remote_hal_keyboard_release(remote_keynote->remote, HID_KEYBOARD_DOWN_ARROW);
                } else if(event->key == InputKeyLeft) {
                    model->left_pressed = false;
                    remote_hal_keyboard_release(remote_keynote->remote, HID_KEYBOARD_LEFT_ARROW);
                } else if(event->key == InputKeyRight) {
                    model->right_pressed = false;
                    remote_hal_keyboard_release(remote_keynote->remote, HID_KEYBOARD_RIGHT_ARROW);
                } else if(event->key == InputKeyOk) {
                    model->ok_pressed = false;
                    remote_hal_keyboard_release(remote_keynote->remote, HID_KEYBOARD_SPACEBAR);
                } else if(event->key == InputKeyBack) {
                    model->back_pressed = false;
                }
            } else if(event->type == InputTypeShort) {
                if(event->key == InputKeyBack) {
                    if(!remote_keynote->remote->laser_enabled) {
                        remote_hal_keyboard_press(remote_keynote->remote, HID_KEYBOARD_DELETE);
                        remote_hal_keyboard_release(remote_keynote->remote, HID_KEYBOARD_DELETE);
                        remote_hal_consumer_key_press(
                            remote_keynote->remote, HID_CONSUMER_AC_BACK);
                        remote_hal_consumer_key_release(
                            remote_keynote->remote, HID_CONSUMER_AC_BACK);
                    }
                }
            }

            // laser handling
            if(event->key == InputKeyBack) {
                if(event->type == InputTypePress) {
                    model->laser_on = true;
                } else if(event->type == InputTypeShort) {
                    model->laser_keep_on = !model->laser_keep_on;
                    model->laser_on =
                        false; // keep_on will still keep it in, we're just resetting the long press state
                    model->short_expect_release = true;
                } else if(event->type == InputTypeLong || event->type == InputTypeRelease) {
                    if(model->short_expect_release) {
                        model->short_expect_release = false;
                    } else {
                        model->laser_on = event->type == InputTypeLong; // long press
                    }
                }
            }
            attempt_set_5v_state(model->laser_on || model->laser_keep_on);
        },
        true);
}

static bool remote_keynote_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    FURI_LOG_I(TAG, "Input event: %d %d", event->type, event->key);
    RemoteKeynote* remote_keynote = context;
    bool consumed = false;

    /*if(event->type == InputTypeLong && event->key == InputKeyBack) {
        remote_hal_keyboard_release_all(remote_keynote->remote);
    } else {
        remote_keynote_process(remote_keynote, event);
        consumed = true;
    }
    */
    // check if back button and ok button is pressed at the same time, if so return false
    if(event->type == InputTypeShort && event->key == InputKeyBack) {
        ExitState* exit_state = remote_keynote->exit_state;
        uint32_t now = furi_get_tick();
        FURI_LOG_I(TAG, "Now: %ld", now);
        uint32_t delta = now - exit_state->last_back_press_time;
        FURI_LOG_I(
            TAG,
            "Back pressed, count: %d, last: %lu, delta: %lu",
            exit_state->back_press_count,
            exit_state->last_back_press_time,
            delta);
        if(delta > SPAM_BACK_DELAY_MS) {
            FURI_LOG_I(TAG, "Back pressed after delay, resetting");
            exit_state->back_press_count = 0; // reset
        } else { // valid
            if(exit_state->back_press_count >= 2) {
                consumed = false;
                FURI_LOG_I(TAG, "Back pressed 3 times, exiting");
                exit_state->back_press_count = 0;
                remote_hal_keyboard_release_all(remote_keynote->remote);
                attempt_set_5v_state(false);
                return false;
            }
        }
        exit_state->last_back_press_time = now;
        exit_state->back_press_count++;
    }

    remote_keynote_process(remote_keynote, event);
    consumed = true;

    return consumed;
}

ExitState* exit_state_alloc() {
    ExitState* exit_state = malloc(sizeof(ExitState));
    exit_state->back_press_count = 0;
    exit_state->last_back_press_time = 0;
    return exit_state;
}

RemoteKeynote* remote_keynote_alloc(Remote* remote) {
    RemoteKeynote* remote_keynote = malloc(sizeof(RemoteKeynote));
    remote_keynote->view = view_alloc();
    remote_keynote->exit_state = exit_state_alloc();
    remote_keynote->remote = remote;
    view_set_context(remote_keynote->view, remote_keynote);
    view_allocate_model(remote_keynote->view, ViewModelTypeLocking, sizeof(RemoteKeynoteModel));
    view_set_draw_callback(remote_keynote->view, remote_keynote_draw_callback);
    view_set_input_callback(remote_keynote->view, remote_keynote_input_callback);
    return remote_keynote;
}

void remote_keynote_free(RemoteKeynote* remote_keynote) {
    furi_assert(remote_keynote);
    FURI_LOG_I(TAG, "Freeing remote keynote");
    view_free(remote_keynote->view);
    free(remote_keynote->exit_state);
    free(remote_keynote);
}

View* remote_keynote_get_view(RemoteKeynote* remote_keynote) {
    furi_assert(remote_keynote);
    return remote_keynote->view;
}

void remote_keynote_set_connected_status(RemoteKeynote* remote_keynote, bool connected) {
    furi_assert(remote_keynote);
    with_view_model(
        remote_keynote->view, RemoteKeynoteModel * model, { model->connected = connected; }, true);
}

void remote_keynote_set_orientation(RemoteKeynote* remote_keynote, bool vertical) {
    furi_assert(remote_keynote);

    if(vertical) {
        view_set_draw_callback(remote_keynote->view, remote_keynote_draw_vertical_callback);
        view_set_orientation(remote_keynote->view, ViewOrientationVerticalFlip);

    } else {
        view_set_draw_callback(remote_keynote->view, remote_keynote_draw_callback);
        view_set_orientation(remote_keynote->view, ViewOrientationHorizontal);
    }
}
