/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include "zmk/keys.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(hid_io, CONFIG_ZMK_HID_IO_LOG_LEVEL);

#include <zmk/hid.h>
#include <dt-bindings/zmk/modifiers.h>
#include <zmk/keymap.h>

#include <zmk/hid-io/hid.h>

#if IS_ENABLED(CONFIG_ZMK_HID_IO_OUTPUT)
#if IS_ENABLED(CONFIG_ZMK_OUTPUT_BEHAVIOR_LISTENER)
#include <zmk/output/output_event.h>
#endif
#endif

#if IS_ENABLED(CONFIG_ZMK_HID_IO_JOYSTICK)

static struct zmk_hid_joystick_report_alt joystick_report_alt = {
    .report_id = ZMK_HID_REPORT_ID__IO_JOYSTICK,
    .body = { .d_x = 0, .d_y = 0, .d_z = 0,  .d_rx = 0, .d_ry = 0, .d_rz = 0,
              .buttons = 0 }};

// Keep track of how often a button was pressed.
// Only release the button if the count is 0.
static int explicit_joy2_btn_counts[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static zmk_mod_flags_t explicit_joy2_btns = 0;

#define SET_JOYSTICK_BUTTONS(btns)                                                                 \
    {                                                                                              \
        joystick_report_alt.body.buttons = btns;                                                   \
        LOG_DBG("JOYSTICK Buttons set to 0x%02X", joystick_report_alt.body.buttons);                        \
    }

int zmk_hid_joy2_button_press(zmk_joystick_button_t button) {
    if (button >= ZMK_HID_JOYSTICK_NUM_BUTTONS) {
        return -EINVAL;
    }

    explicit_joy2_btn_counts[button]++;
    LOG_DBG("JOYSTICK Button %d count %d", button, explicit_joy2_btn_counts[button]);
    WRITE_BIT(explicit_joy2_btns, button, true);
    SET_JOYSTICK_BUTTONS(explicit_joy2_btns);
    return 0;
}

int zmk_hid_joy2_button_release(zmk_joystick_button_t button) {
    if (button >= ZMK_HID_JOYSTICK_NUM_BUTTONS) {
        return -EINVAL;
    }

    if (explicit_joy2_btn_counts[button] <= 0) {
        LOG_ERR("Tried to release JOYSTICK button %d too often", button);
        return -EINVAL;
    }
    explicit_joy2_btn_counts[button]--;
    LOG_DBG("JOYSTICK Button %d count: %d", button, explicit_joy2_btn_counts[button]);
    if (explicit_joy2_btn_counts[button] == 0) {
        LOG_DBG("JOYSTICK Button %d released", button);
        WRITE_BIT(explicit_joy2_btns, button, false);
    }
    SET_JOYSTICK_BUTTONS(explicit_joy2_btns);
    return 0;
}

int zmk_hid_joy2_buttons_press(zmk_joystick_button_flags_t buttons) {
    for (zmk_joystick_button_t i = 0; i < ZMK_HID_JOYSTICK_NUM_BUTTONS; i++) {
        if (buttons & BIT(i)) {
            zmk_hid_joy2_button_press(i);
        }
    }
    return 0;
}

int zmk_hid_joy2_buttons_release(zmk_joystick_button_flags_t buttons) {
    for (zmk_joystick_button_t i = 0; i < ZMK_HID_JOYSTICK_NUM_BUTTONS; i++) {
        if (buttons & BIT(i)) {
            zmk_hid_joy2_button_release(i);
        }
    }
    return 0;
}

void zmk_hid_joy2_movement_set(int16_t x, int16_t y) {
    joystick_report_alt.body.d_x = x;
    joystick_report_alt.body.d_y = y;
    LOG_DBG("joy mov set to %d/%d", joystick_report_alt.body.d_x, joystick_report_alt.body.d_y);
}

void zmk_hid_joy2_movement_update(int16_t x, int16_t y) {
    joystick_report_alt.body.d_x += x;
    joystick_report_alt.body.d_y += y;
    LOG_DBG("joy mov updated to %d/%d", joystick_report_alt.body.d_x, joystick_report_alt.body.d_y);
}

// void zmk_hid_joy2_scroll_set(int8_t x, int8_t y) {
//     joystick_report_alt.body.d_scroll_x = x;
//     joystick_report_alt.body.d_scroll_y = y;
//     LOG_DBG("joy scl set to %d/%d", joystick_report_alt.body.d_scroll_x,
//             joystick_report_alt.body.d_scroll_y);
// }

// void zmk_hid_joy2_scroll_update(int8_t x, int8_t y) {
//     joystick_report_alt.body.d_scroll_x += x;
//     joystick_report_alt.body.d_scroll_y += y;
//     LOG_DBG("joy scl updated to X: %d/%d", joystick_report_alt.body.d_scroll_x,
//             joystick_report_alt.body.d_scroll_y);
// }

void zmk_hid_joy2_clear(void) {
    LOG_DBG("joy report cleared");
    memset(&joystick_report_alt.body, 0, sizeof(joystick_report_alt.body));
}

struct zmk_hid_joystick_report_alt *zmk_hid_get_joystick_report_alt(void) {
    return &joystick_report_alt;
}

#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_JOYSTICK)

#if IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)

static struct zmk_hid_mouse_report_alt mouse_report_alt = {
    .report_id = ZMK_HID_REPORT_ID__IO_MOUSE,
    .body = {.buttons = 0, .d_x = 0, .d_y = 0, .d_scroll_y = 0, .d_scroll_y = 0 }};

// Keep track of how often a button was pressed.
// Only release the button if the count is 0.
static int explicit_mou2_btn_counts[5] = {0, 0, 0, 0, 0};
static zmk_mod_flags_t explicit_mou2_btns = 0;

#define SET_MOUSE_BUTTONS(btns)                                                                    \
    {                                                                                              \
        mouse_report_alt.body.buttons = btns;                                                          \
        LOG_DBG("MOUSE buttons set to 0x%02X", mouse_report_alt.body.buttons);                         \
    }

int zmk_hid_mou2_button_press(zmk_mouse_button_t button) {
    if (button >= ZMK_HID_MOUSE_NUM_BUTTONS) {
        return -EINVAL;
    }

    explicit_mou2_btn_counts[button]++;
    LOG_DBG("MOUSE Button %d count %d", button, explicit_mou2_btn_counts[button]);
    WRITE_BIT(explicit_mou2_btns, button, true);
    SET_MOUSE_BUTTONS(explicit_mou2_btns);
    return 0;
}

int zmk_hid_mou2_button_release(zmk_mouse_button_t button) {
    if (button >= ZMK_HID_MOUSE_NUM_BUTTONS) {
        return -EINVAL;
    }

    if (explicit_mou2_btn_counts[button] <= 0) {
        LOG_ERR("Tried to release MOUSE button %d too often", button);
        return -EINVAL;
    }
    explicit_mou2_btn_counts[button]--;
    LOG_DBG("MOUSE Button %d count: %d", button, explicit_mou2_btn_counts[button]);
    if (explicit_mou2_btn_counts[button] == 0) {
        LOG_DBG("MOUSE Button %d released", button);
        WRITE_BIT(explicit_mou2_btns, button, false);
    }
    SET_MOUSE_BUTTONS(explicit_mou2_btns);
    return 0;
}

int zmk_hid_mou2_buttons_press(zmk_mouse_button_flags_t buttons) {
    for (zmk_mouse_button_t i = 0; i < ZMK_HID_MOUSE_NUM_BUTTONS; i++) {
        if (buttons & BIT(i)) {
            zmk_hid_mou2_button_press(i);
        }
    }
    return 0;
}

int zmk_hid_mou2_buttons_release(zmk_mouse_button_flags_t buttons) {
    for (zmk_mouse_button_t i = 0; i < ZMK_HID_MOUSE_NUM_BUTTONS; i++) {
        if (buttons & BIT(i)) {
            zmk_hid_mou2_button_release(i);
        }
    }
    return 0;
}

void zmk_hid_mou2_movement_set(int16_t x, int16_t y) {
    mouse_report_alt.body.d_x = x;
    mouse_report_alt.body.d_y = y;
    LOG_DBG("mou mov set to %d/%d", mouse_report_alt.body.d_x, mouse_report_alt.body.d_y);
}

void zmk_hid_mou2_movement_update(int16_t x, int16_t y) {
    mouse_report_alt.body.d_x += x;
    mouse_report_alt.body.d_y += y;
    LOG_DBG("mou mov updated to %d/%d", mouse_report_alt.body.d_x, mouse_report_alt.body.d_y);
}

void zmk_hid_mou2_scroll_set(int8_t x, int8_t y) {
    mouse_report_alt.body.d_scroll_x = x;
    mouse_report_alt.body.d_scroll_y = y;
    LOG_DBG("mou scl set to %d/%d", mouse_report_alt.body.d_scroll_x,
            mouse_report_alt.body.d_scroll_y);
}

void zmk_hid_mou2_scroll_update(int8_t x, int8_t y) {
    mouse_report_alt.body.d_scroll_x += x;
    mouse_report_alt.body.d_scroll_y += y;
    LOG_DBG("mou scl updated to X: %d/%d", mouse_report_alt.body.d_scroll_x,
            mouse_report_alt.body.d_scroll_y);
}

void zmk_hid_mou2_clear(void) {
    LOG_DBG("mou report cleared");
    memset(&mouse_report_alt.body, 0, sizeof(mouse_report_alt.body));
}

struct zmk_hid_mouse_report_alt *zmk_hid_get_mouse_report_alt(void) {
    return &mouse_report_alt;
}

#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)

#if IS_ENABLED(CONFIG_ZMK_HID_IO_OUTPUT)

K_MSGQ_DEFINE(hid_io_output_event_msgq, sizeof(struct hid_io_output_event),
              CONFIG_ZMK_HID_IO_OUTPUT_QUEUE_SIZE, 4);

void hid_io_output_event_work_callback(struct k_work *work) {
    struct hid_io_output_event ev;
    while (k_msgq_get(&hid_io_output_event_msgq, &ev, K_NO_WAIT) == 0) {
        LOG_DBG("Trigger output event: f/%d  d/%d", ev.force, ev.duration);

        #if IS_ENABLED(CONFIG_ZMK_OUTPUT_BEHAVIOR_LISTENER)
        raise_zmk_output_event((struct zmk_output_event){
            .source = OUTPUT_SOURCE_TRANSPORT,
            .layer = zmk_keymap_highest_layer_active(),
            .force = ev.force, .duration = ev.duration,
            .timestamp = k_uptime_get()
            });
        #endif

    }
}

K_WORK_DEFINE(hid_io_output_event_work, hid_io_output_event_work_callback);

void zmk_hid_io_output_process_report(struct zmk_hid_io_output_report_body *report,
                                      struct zmk_endpoint_instance endpoint) {
    struct hid_io_output_event ev = {
        .tansport = endpoint.transport,
        .force = report->force,
        .duration = report->duration,
    };
    k_msgq_put(&hid_io_output_event_msgq, &ev, K_NO_WAIT);
    k_work_submit(&hid_io_output_event_work);
}

#endif