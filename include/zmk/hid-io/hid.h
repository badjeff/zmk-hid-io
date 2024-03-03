/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/class/usb_hid.h>

#include <zmk/keys.h>
#include <zmk/hid.h>
#include <zmk/endpoints_types.h>

#if IS_ENABLED(CONFIG_ZMK_HID_IO_JOYSTICK)
#include <zmk/hid-io/joystick.h>
#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_JOYSTICK)

#if IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)
#include <zmk/hid-io/mouse.h>
#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)

#include <dt-bindings/zmk/hid_usage.h>
#include <dt-bindings/zmk/hid_usage_pages.h>

#if IS_ENABLED(CONFIG_ZMK_HID_KEYBOARD_NKRO_EXTENDED_REPORT)
#define ZMK_HID_KEYBOARD_NKRO_MAX_USAGE HID_USAGE_KEY_KEYBOARD_LANG8
#else
#define ZMK_HID_KEYBOARD_NKRO_MAX_USAGE HID_USAGE_KEY_KEYPAD_EQUAL
#endif

#define ZMK_HID_JOYSTICK_NUM_BUTTONS 0x08
#define ZMK_HID_MOUSE_NUM_BUTTONS 0x05

// See https://www.usb.org/sites/default/files/hid1_11.pdf section 6.2.2.4 Main Items

#define ZMK_HID_REPORT_ID__IO_JOYSTICK 0x02
#define ZMK_HID_REPORT_ID__IO_MOUSE 0x03
#define ZMK_HID_REPORT_ID__IO_OUTPUT 0x04

#ifndef HID_USAGE_PAGE16
#define HID_USAGE_PAGE16(page, page2)                                                              \
    HID_ITEM(HID_ITEM_TAG_USAGE_PAGE, HID_ITEM_TYPE_GLOBAL, 2), page, page2
#endif

// Needed until Zephyr offers a 2 byte usage macro
#define HID_USAGE16(idx)                                                                           \
    HID_ITEM(HID_ITEM_TAG_USAGE, HID_ITEM_TYPE_LOCAL, 2), (idx & 0xFF), (idx >> 8 & 0xFF)

static const uint8_t zmk_hid_report_desc_alt[] = {
    
    HID_USAGE_PAGE16(0x0C, 0xFF),
    HID_USAGE(0x01),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),

#if IS_ENABLED(CONFIG_ZMK_HID_IO_JOYSTICK)
    HID_USAGE_PAGE(HID_USAGE_GD),
    HID_USAGE(HID_USAGE_GD_JOYSTICK),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),
    HID_REPORT_ID(ZMK_HID_REPORT_ID__IO_JOYSTICK),
    HID_COLLECTION(HID_COLLECTION_LOGICAL),
    HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),
    HID_USAGE(HID_USAGE_GD_X),
    HID_USAGE(HID_USAGE_GD_Y),
    HID_USAGE(HID_USAGE_GD_Z),
    HID_USAGE(HID_USAGE_GD_RX),
    HID_USAGE(HID_USAGE_GD_RY),
    HID_USAGE(HID_USAGE_GD_RZ),
    HID_LOGICAL_MIN8(-0x7F),
    HID_LOGICAL_MAX8(0x7F),
    HID_REPORT_SIZE(0x08),
    HID_REPORT_COUNT(0x06),
    HID_INPUT(ZMK_HID_MAIN_VAL_DATA | ZMK_HID_MAIN_VAL_VAR | ZMK_HID_MAIN_VAL_REL),
    HID_USAGE_PAGE(HID_USAGE_BUTTON),
    HID_USAGE_MIN8(0x1),
    HID_USAGE_MAX8(ZMK_HID_MOUSE_NUM_BUTTONS),
    HID_LOGICAL_MIN8(0x00),
    HID_LOGICAL_MAX8(0x01),
    HID_REPORT_SIZE(0x01),
    HID_REPORT_COUNT(0x8),
    HID_INPUT(ZMK_HID_MAIN_VAL_DATA | ZMK_HID_MAIN_VAL_VAR | ZMK_HID_MAIN_VAL_ABS),
    HID_END_COLLECTION,
    HID_END_COLLECTION,
#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_JOYSTICK)

#if IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)
    HID_USAGE_PAGE(HID_USAGE_GD),
    HID_USAGE(HID_USAGE_GD_MOUSE),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),
    HID_REPORT_ID(ZMK_HID_REPORT_ID__IO_MOUSE),
    HID_USAGE(HID_USAGE_GD_POINTER),
    HID_COLLECTION(HID_COLLECTION_PHYSICAL),
    HID_USAGE_PAGE(HID_USAGE_BUTTON),
    HID_USAGE_MIN8(0x1),
    HID_USAGE_MAX8(ZMK_HID_MOUSE_NUM_BUTTONS),
    HID_LOGICAL_MIN8(0x00),
    HID_LOGICAL_MAX8(0x01),
    HID_REPORT_SIZE(0x01),
    HID_REPORT_COUNT(0x5),
    HID_INPUT(ZMK_HID_MAIN_VAL_DATA | ZMK_HID_MAIN_VAL_VAR | ZMK_HID_MAIN_VAL_ABS),
    // Constant padding for the last 3 bits.
    HID_REPORT_SIZE(0x03),
    HID_REPORT_COUNT(0x01),
    HID_INPUT(ZMK_HID_MAIN_VAL_CONST | ZMK_HID_MAIN_VAL_VAR | ZMK_HID_MAIN_VAL_ABS),
    // Some OSes ignore pointer devices without X/Y data.
    HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),
    HID_USAGE(HID_USAGE_GD_X),
    HID_USAGE(HID_USAGE_GD_Y),
    HID_USAGE(HID_USAGE_GD_WHEEL),
    HID_LOGICAL_MIN8(-0x7F),
    HID_LOGICAL_MAX8(0x7F),
    HID_REPORT_SIZE(0x08),
    HID_REPORT_COUNT(0x03),
    HID_INPUT(ZMK_HID_MAIN_VAL_DATA | ZMK_HID_MAIN_VAL_VAR | ZMK_HID_MAIN_VAL_REL),
    HID_USAGE_PAGE(HID_USAGE_CONSUMER),
    HID_USAGE16(HID_USAGE_CONSUMER_AC_PAN),
    HID_LOGICAL_MIN8(-0x7F),
    HID_LOGICAL_MAX8(0x7F),
    HID_REPORT_SIZE(0x08),
    HID_REPORT_COUNT(0x01),
    HID_INPUT(ZMK_HID_MAIN_VAL_DATA | ZMK_HID_MAIN_VAL_VAR | ZMK_HID_MAIN_VAL_REL),
    HID_END_COLLECTION,
    HID_END_COLLECTION,
#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)

#if IS_ENABLED(CONFIG_ZMK_HID_IO_OUTPUT)
    HID_USAGE_PAGE(HID_USAGE_HAPTICS),
    HID_USAGE_MIN8(0x0),
    HID_USAGE_MAX8(0xFF),
    HID_REPORT_COUNT(0x2),
    HID_REPORT_SIZE(0x08),
    HID_OUTPUT(ZMK_HID_MAIN_VAL_DATA | ZMK_HID_MAIN_VAL_VAR | ZMK_HID_MAIN_VAL_ABS),
#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_OUTPUT)

    HID_END_COLLECTION,
};

#if IS_ENABLED(CONFIG_ZMK_HID_IO_JOYSTICK)
struct zmk_hid_joystick_report_body_alt {
    int8_t d_x;
    int8_t d_y;
    int8_t d_z;
    int8_t d_rx;
    int8_t d_ry;
    int8_t d_rz;
    zmk_joystick_button_flags_t buttons;
} __packed;
struct zmk_hid_joystick_report_alt {
    uint8_t report_id;
    struct zmk_hid_joystick_report_body_alt body;
} __packed;
int zmk_hid_joy2_button_press(zmk_joystick_button_t button);
int zmk_hid_joy2_button_release(zmk_joystick_button_t button);
int zmk_hid_joy2_buttons_press(zmk_joystick_button_flags_t buttons);
int zmk_hid_joy2_buttons_release(zmk_joystick_button_flags_t buttons);
void zmk_hid_joy2_movement_set(int16_t x, int16_t y);
// void zmk_hid_joy2_scroll_set(int8_t x, int8_t y);
void zmk_hid_joy2_movement_update(int16_t x, int16_t y);
// void zmk_hid_joy2_scroll_update(int8_t x, int8_t y);
void zmk_hid_joy2_clear(void);
struct zmk_hid_joystick_report_alt *zmk_hid_get_joystick_report_alt();
#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_JOYSTICK)

#if IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)
struct zmk_hid_mouse_report_body_alt {
    zmk_mouse_button_flags_t buttons;
    int8_t d_x;
    int8_t d_y;
    int8_t d_scroll_y;
    int8_t d_scroll_x;
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
void zmk_hid_mou2_scroll_set(int8_t x, int8_t y);
void zmk_hid_mou2_movement_update(int16_t x, int16_t y);
void zmk_hid_mou2_scroll_update(int8_t x, int8_t y);
void zmk_hid_mou2_clear(void);
struct zmk_hid_mouse_report_alt *zmk_hid_get_mouse_report_alt();
#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)

#if IS_ENABLED(CONFIG_ZMK_HID_IO_OUTPUT)
struct zmk_hid_io_output_report_body {
    uint8_t force;
    uint8_t duration;
} __packed;
struct zmk_hid_io_output_report {
    uint8_t report_id;
    struct zmk_hid_io_output_report_body body;
} __packed;
struct hid_io_output_event {
    enum zmk_transport tansport;
    uint8_t force;
    uint8_t duration;
};
void zmk_hid_io_output_process_report(struct zmk_hid_io_output_report_body *report,
                                      struct zmk_endpoint_instance endpoint);
#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_OUTPUT)
