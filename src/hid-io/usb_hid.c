/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/device.h>
#include <zephyr/init.h>

#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/class/usb_hid.h>

#include <zmk/usb.h>
#include <zmk/hid.h>
#include <zmk/keymap.h>
#include <zmk/event_manager.h>

#include <zmk/hid-io/hid.h>
#include <zmk/hid-io/usb_hid.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(hid_io, CONFIG_ZMK_HID_IO_LOG_LEVEL);

static const struct device *hid_dev;

static K_SEM_DEFINE(hid_sem, 1, 1);

static void in_ready_cb(const struct device *dev) { k_sem_give(&hid_sem); }

#define HID_GET_REPORT_TYPE_MASK 0xff00
#define HID_GET_REPORT_ID_MASK 0x00ff

#define HID_REPORT_TYPE_INPUT 0x100
#define HID_REPORT_TYPE_OUTPUT 0x200
#define HID_REPORT_TYPE_FEATURE 0x300

static int get_report_cb(const struct device *dev, struct usb_setup_packet *setup, int32_t *len,
                         uint8_t **data) {

    /*
     * 7.2.1 of the HID v1.11 spec is unclear about handling requests for reports that do not exist
     * For requested reports that aren't input reports, return -ENOTSUP like the Zephyr subsys does
     */
    if ((setup->wValue & HID_GET_REPORT_TYPE_MASK) != HID_REPORT_TYPE_INPUT &&
        (setup->wValue & HID_GET_REPORT_TYPE_MASK) != HID_REPORT_TYPE_FEATURE) {
        LOG_ERR("[# hid-io #] Get: Unsupported report type %d requested",
                (setup->wValue & HID_GET_REPORT_TYPE_MASK) << 8);
        return -ENOTSUP;
    }

    switch (setup->wValue & HID_GET_REPORT_ID_MASK) {
#if IS_ENABLED(CONFIG_ZMK_HID_IO_JOYSTICK)
    case ZMK_HID_REPORT_ID__IO_JOYSTICK: {
        struct zmk_hid_joystick_report_alt *report = zmk_hid_get_joystick_report_alt();
        *data = (uint8_t *)report;
        *len = sizeof(*report);
        break;
    }
#endif
#if IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)
    case ZMK_HID_REPORT_ID__IO_MOUSE: {
        struct zmk_hid_mouse_report_alt *report = zmk_hid_get_mouse_report_alt();
        *data = (uint8_t *)report;
        *len = sizeof(*report);
        break;
    }
#endif
#if IS_ENABLED(CONFIG_ZMK_HID_IO_VOLUME_KNOB)
    case ZMK_HID_REPORT_ID__IO_VOLUME_KNOB: {
        struct zmk_hid_volume_knob_report_alt *report = zmk_hid_get_volume_knob_report_alt();
        *data = (uint8_t *)report;
        *len = sizeof(*report);
        break;
    }
#endif
    default:
        LOG_ERR("[# hid-io #] Invalid report ID %d requested", setup->wValue & HID_GET_REPORT_ID_MASK);
        return -EINVAL;
    }

    return 0;
}

static int set_report_cb(const struct device *dev, struct usb_setup_packet *setup, int32_t *len,
                         uint8_t **data) {
    if ((setup->wValue & HID_GET_REPORT_TYPE_MASK) != HID_REPORT_TYPE_OUTPUT &&
        (setup->wValue & HID_GET_REPORT_TYPE_MASK) != HID_REPORT_TYPE_FEATURE) {
        LOG_ERR("[# hid-io #] Set: Unsupported report type %d requested",
                (setup->wValue & HID_GET_REPORT_TYPE_MASK) >> 8);
        return -ENOTSUP;
    }

    switch (setup->wValue & HID_GET_REPORT_ID_MASK) {
#if IS_ENABLED(CONFIG_ZMK_HID_IO_OUTPUT)
    case ZMK_HID_REPORT_ID__IO_OUTPUT:
        if (*len != sizeof(struct zmk_hid_io_output_report)) {
            LOG_ERR("[# hid-io #] HAPTIC set report is malformed: length=%d", *len);
            return -EINVAL;
        } else {
            struct zmk_hid_io_output_report *report = (struct zmk_hid_io_output_report *)*data;
            struct zmk_endpoint_instance endpoint = {
                .transport = ZMK_TRANSPORT_USB,
            };
            zmk_hid_io_output_process_report(&report->body, endpoint);
        }
        break;
#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_OUTPUT)
    default:
        LOG_ERR("[# hid-io #] ## Invalid report ID %d requested", setup->wValue & HID_GET_REPORT_ID_MASK);
        return -EINVAL;
    }

    return 0;
}

static const struct hid_ops ops = {
    .int_in_ready = in_ready_cb,
    .get_report = get_report_cb,
    .set_report = set_report_cb,
};

static int zmk_usb_hid_send_report_alt(const uint8_t *report, size_t len) {
    switch (zmk_usb_get_status()) {
    case USB_DC_SUSPEND:
        return usb_wakeup_request();
    case USB_DC_ERROR:
    case USB_DC_RESET:
    case USB_DC_DISCONNECTED:
    case USB_DC_UNKNOWN:
        return -ENODEV;
    default:
        k_sem_take(&hid_sem, K_MSEC(30));
        LOG_HEXDUMP_DBG(report, len, "HID-IO HID report");
        int err = hid_int_ep_write(hid_dev, report, len, NULL);

        if (err) {
            k_sem_give(&hid_sem);
        }

        return err;
    }
}

#if IS_ENABLED(CONFIG_ZMK_HID_IO_JOYSTICK)
int zmk_usb_hid_send_joystick_report_alt() {
    struct zmk_hid_joystick_report_alt *report = zmk_hid_get_joystick_report_alt();
    return zmk_usb_hid_send_report_alt((uint8_t *)report, sizeof(*report));
}
#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_JOYSTICK)

#if IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)
int zmk_usb_hid_send_mouse_report_alt() {
    struct zmk_hid_mouse_report_alt *report = zmk_hid_get_mouse_report_alt();
    return zmk_usb_hid_send_report_alt((uint8_t *)report, sizeof(*report));
}
#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)

#if IS_ENABLED(CONFIG_ZMK_HID_IO_VOLUME_KNOB)
int zmk_usb_hid_send_volume_knob_report_alt() {
    struct zmk_hid_volume_knob_report_alt *report = zmk_hid_get_volume_knob_report_alt();
    return zmk_usb_hid_send_report_alt((uint8_t *)report, sizeof(*report));
}
#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_VOLUME_KNOB)

static int zmk_usb_hid_init_alt(void) {
    hid_dev = device_get_binding("HID_1");
    if (hid_dev == NULL) {
        LOG_ERR("Unable to locate HID device");
        return -EINVAL;
    }

    usb_hid_register_device(hid_dev, zmk_hid_report_desc_alt, sizeof(zmk_hid_report_desc_alt), &ops);

    usb_hid_init(hid_dev);

    return 0;
}

SYS_INIT(zmk_usb_hid_init_alt, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
