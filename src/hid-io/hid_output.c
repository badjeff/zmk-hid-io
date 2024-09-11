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
#include <zmk/hid-io/hid_output.h>

#if IS_ENABLED(CONFIG_ZMK_HID_IO_OUTPUT)

#if IS_ENABLED(CONFIG_ZMK_OUTPUT_BEHAVIOR_LISTENER)
#include <zmk/output/output_event.h>
#endif

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
        #else
        LOG_WRN("zmk,output-behavior-listener is not enabled.");
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