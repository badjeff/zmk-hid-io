/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zmk/keys.h>
#include <zmk/hid.h>
#include <zmk/endpoints_types.h>

#if IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)

#include <zmk/hid-io/mouse.h>

struct zmk_hid_mouse_report_body_alt {
    zmk_mouse_button_flags_t buttons;
    int16_t d_x;
    int16_t d_y;
    int16_t d_scroll_y;
    int16_t d_scroll_x;
} __packed;
struct zmk_hid_mouse_report_alt {
    uint8_t report_id;
    struct zmk_hid_mouse_report_body_alt body;
} __packed;
int zmk_hid_mou2_button_press(zmk_mouse_button_t button);
int zmk_hid_mou2_button_release(zmk_mouse_button_t button);
int zmk_hid_mou2_buttons_press(zmk_mouse_button_flags_t buttons);
int zmk_hid_mou2_buttons_release(zmk_mouse_button_flags_t buttons);
void zmk_hid_mou2_movement_set(int16_t x, int16_t y);
void zmk_hid_mou2_scroll_set(int16_t x, int16_t y);
void zmk_hid_mou2_movement_update(int16_t x, int16_t y);
void zmk_hid_mou2_scroll_update(int16_t x, int16_t y);
void zmk_hid_mou2_clear(void);
struct zmk_hid_mouse_report_alt *zmk_hid_get_mouse_report_alt();

#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)
