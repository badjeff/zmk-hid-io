/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zmk/keys.h>
#include <zmk/hid.h>
#include <zmk/endpoints_types.h>

#if IS_ENABLED(CONFIG_ZMK_HID_IO_OUTPUT)

struct zmk_hid_io_output_report_body {
    uint8_t force;
    uint8_t value;
} __packed;
struct zmk_hid_io_output_report {
    uint8_t report_id;
    struct zmk_hid_io_output_report_body body;
} __packed;
struct hid_io_output_event {
    enum zmk_transport tansport;
    uint8_t force;
    uint8_t value;
};
void zmk_hid_io_output_process_report(struct zmk_hid_io_output_report_body *report,
                                      struct zmk_endpoint_instance endpoint);

#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_OUTPUT)
