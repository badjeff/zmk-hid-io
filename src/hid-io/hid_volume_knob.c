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
#include <zmk/hid-io/hid_volume_knob.h>

#if IS_ENABLED(CONFIG_ZMK_HID_IO_VOLUME_KNOB)

static struct zmk_hid_volume_knob_report_alt volume_knob_report_alt = {
    .report_id = ZMK_HID_REPORT_ID__IO_VOLUME_KNOB,
    .body = { .d_vol = 0 }};

void zmk_hid_volume_knob_vol_set(int8_t vol) {
    volume_knob_report_alt.body.d_vol = vol;
    LOG_DBG("vol knob vol set to %d", volume_knob_report_alt.body.d_vol);
}

void zmk_hid_volume_knob_vol_update(int8_t vol) {
    volume_knob_report_alt.body.d_vol += vol;
    LOG_DBG("vol knob vol updated to %d", volume_knob_report_alt.body.d_vol);
}

void zmk_hid_volume_knob_clear(void) {
    LOG_DBG("vol knob report cleared");
    memset(&volume_knob_report_alt.body, 0, sizeof(volume_knob_report_alt.body));
}

struct zmk_hid_volume_knob_report_alt *zmk_hid_get_volume_knob_report_alt(void) {
    return &volume_knob_report_alt;
}

#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_VOLUME_KNOB)
