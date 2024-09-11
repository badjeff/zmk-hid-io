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
#include <zmk/hid-io/hid_mouse.h>

#if IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)

static struct zmk_hid_mouse_report_alt mouse_report_alt = {
    .report_id = ZMK_HID_REPORT_ID__IO_MOUSE,
    .body = { .buttons = 0, .d_x = 0, .d_y = 0, .d_scroll_y = 0, .d_scroll_x = 0 }};

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

void zmk_hid_mou2_scroll_set(int16_t x, int16_t y) {
    mouse_report_alt.body.d_scroll_x = x;
    mouse_report_alt.body.d_scroll_y = y;
    LOG_DBG("mou scl set to %d/%d", mouse_report_alt.body.d_scroll_x,
            mouse_report_alt.body.d_scroll_y);
}

void zmk_hid_mou2_scroll_update(int16_t x, int16_t y) {
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
