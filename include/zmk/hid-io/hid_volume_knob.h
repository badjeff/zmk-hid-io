/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zmk/keys.h>
#include <zmk/hid.h>
#include <zmk/endpoints_types.h>

#if IS_ENABLED(CONFIG_ZMK_HID_IO_VOLUME_KNOB)

// #include <zmk/hid-io/volume_knob.h>

struct zmk_hid_volume_knob_report_body_alt {
    int8_t d_vol;
} __packed;
struct zmk_hid_volume_knob_report_alt {
    uint8_t report_id;
    struct zmk_hid_volume_knob_report_body_alt body;
} __packed;
void zmk_hid_volume_knob_vol_set(int8_t vol);
void zmk_hid_volume_knob_vol_update(int8_t vol);
void zmk_hid_volume_knob_clear(void);
struct zmk_hid_volume_knob_report_alt *zmk_hid_get_volume_knob_report_alt();

#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_VOLUME_KNOB)
