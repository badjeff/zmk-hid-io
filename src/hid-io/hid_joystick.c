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
#include <zmk/hid-io/hid_joystick.h>

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
