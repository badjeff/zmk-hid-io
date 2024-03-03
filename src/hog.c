/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/settings/settings.h>
#include <zephyr/init.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>

#include <zmk/ble.h>
#include <zmk/endpoints_types.h>
#include <zmk/hog.h>
#include <zmk/hid.h>

#include <zmk/hid-io/hid.h>

enum {
    HIDS_REMOTE_WAKE = BIT(0),
    HIDS_NORMALLY_CONNECTABLE = BIT(1),
};

struct hids_info {
    uint16_t version; /* version number of base USB HID Specification */
    uint8_t code;     /* country HID Device hardware is localized for. */
    uint8_t flags;
} __packed;

struct hids_report {
    uint8_t id;   /* report id */
    uint8_t type; /* report type */
} __packed;

static struct hids_info info = {
    .version = 0x0000,
    .code = 0x00,
    .flags = HIDS_NORMALLY_CONNECTABLE | HIDS_REMOTE_WAKE,
};

enum {
    HIDS_INPUT = 0x01,
    HIDS_OUTPUT = 0x02,
    HIDS_FEATURE = 0x03,
};

#if IS_ENABLED(CONFIG_ZMK_HID_IO_OUTPUT)

static struct hids_report output_indicators = {
    .id = ZMK_HID_REPORT_ID__IO_OUTPUT,
    .type = HIDS_OUTPUT,
};

#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_OUTPUT)

#if IS_ENABLED(CONFIG_ZMK_HID_IO_JOYSTICK)

static struct hids_report joystick_input = {
    .id = ZMK_HID_REPORT_ID__IO_JOYSTICK,
    .type = HIDS_INPUT,
};

#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_JOYSTICK)

#if IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)

static struct hids_report mouse_input = {
    .id = ZMK_HID_REPORT_ID__IO_MOUSE,
    .type = HIDS_INPUT,
};

#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)

static bool host_requests_notification = false;
static uint8_t ctrl_point;
// static uint8_t proto_mode;

static ssize_t read_hids_info(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                              uint16_t len, uint16_t offset) {
    return bt_gatt_attr_read(conn, attr, buf, len, offset, attr->user_data,
                             sizeof(struct hids_info));
}

static ssize_t read_hids_report_ref(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                    void *buf, uint16_t len, uint16_t offset) {
    return bt_gatt_attr_read(conn, attr, buf, len, offset, attr->user_data,
                             sizeof(struct hids_report));
}

static ssize_t read_hids_report_map(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                    void *buf, uint16_t len, uint16_t offset) {
    return bt_gatt_attr_read(conn, attr, buf, len, offset, zmk_hid_report_desc_alt,
                             sizeof(zmk_hid_report_desc_alt));
}

#if IS_ENABLED(CONFIG_ZMK_HID_IO_OUTPUT)
static ssize_t write_hids_output_report(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                        const void *buf, uint16_t len, uint16_t offset,
                                        uint8_t flags) {

    if (offset != 0) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }
    if (len != sizeof(struct zmk_hid_io_output_report_body)) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    struct zmk_hid_io_output_report_body *report = (struct zmk_hid_io_output_report_body *)buf;
    int profile = zmk_ble_profile_index(bt_conn_get_dst(conn));
    if (profile < 0) {
        return BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
    }

    struct zmk_endpoint_instance endpoint = {.transport = ZMK_TRANSPORT_BLE,
                                             .ble = {
                                                 .profile_index = profile,
                                             }};
    zmk_hid_io_output_process_report(report, endpoint);

    return len;
}

#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_OUTPUT)

#if IS_ENABLED(CONFIG_ZMK_HID_IO_JOYSTICK)
static ssize_t read_hids_joystick_input_report(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                            void *buf, uint16_t len, uint16_t offset) {
    struct zmk_hid_joystick_report_body_alt *report_body = &zmk_hid_get_joystick_report_alt()->body;
    return bt_gatt_attr_read(conn, attr, buf, len, offset, report_body,
                             sizeof(struct zmk_hid_joystick_report_body_alt));
}
#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_JOYSTICK)

#if IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)
static ssize_t read_hids_mouse_input_report(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                            void *buf, uint16_t len, uint16_t offset) {
    struct zmk_hid_mouse_report_body_alt *report_body = &zmk_hid_get_mouse_report_alt()->body;
    return bt_gatt_attr_read(conn, attr, buf, len, offset, report_body,
                             sizeof(struct zmk_hid_mouse_report_body_alt));
}
#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)

static void input_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value) {
    host_requests_notification = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

static ssize_t write_ctrl_point(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                const void *buf, uint16_t len, uint16_t offset, uint8_t flags) {
    uint8_t *value = attr->user_data;

    if (offset + len > sizeof(ctrl_point)) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    memcpy(value + offset, buf, len);

    return len;
}

/* HID Service Declaration */
BT_GATT_SERVICE_DEFINE(
    hog_svc_alt, BT_GATT_PRIMARY_SERVICE(BT_UUID_HIDS),
    BT_GATT_CHARACTERISTIC(BT_UUID_HIDS_INFO, BT_GATT_CHRC_READ, BT_GATT_PERM_READ, read_hids_info,
                           NULL, &info),
    BT_GATT_CHARACTERISTIC(BT_UUID_HIDS_REPORT_MAP, BT_GATT_CHRC_READ, BT_GATT_PERM_READ_ENCRYPT,
                           read_hids_report_map, NULL, NULL),

#if IS_ENABLED(CONFIG_ZMK_HID_IO_JOYSTICK)
    BT_GATT_CHARACTERISTIC(BT_UUID_HIDS_REPORT, BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ_ENCRYPT, read_hids_joystick_input_report, NULL, NULL),
    BT_GATT_CCC(input_ccc_changed, BT_GATT_PERM_READ_ENCRYPT | BT_GATT_PERM_WRITE_ENCRYPT),
    BT_GATT_DESCRIPTOR(BT_UUID_HIDS_REPORT_REF, BT_GATT_PERM_READ_ENCRYPT, read_hids_report_ref,
                       NULL, &joystick_input),
#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_JOYSTICK)

#if IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)
    BT_GATT_CHARACTERISTIC(BT_UUID_HIDS_REPORT, BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ_ENCRYPT, read_hids_mouse_input_report, NULL, NULL),
    BT_GATT_CCC(input_ccc_changed, BT_GATT_PERM_READ_ENCRYPT | BT_GATT_PERM_WRITE_ENCRYPT),
    BT_GATT_DESCRIPTOR(BT_UUID_HIDS_REPORT_REF, BT_GATT_PERM_READ_ENCRYPT, read_hids_report_ref,
                       NULL, &mouse_input),
#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)

#if IS_ENABLED(CONFIG_ZMK_HID_IO_OUTPUT)
    BT_GATT_CHARACTERISTIC(BT_UUID_HIDS_REPORT,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                           BT_GATT_PERM_READ_ENCRYPT | BT_GATT_PERM_WRITE_ENCRYPT, NULL,
                           write_hids_output_report, NULL),
    BT_GATT_DESCRIPTOR(BT_UUID_HIDS_REPORT_REF, BT_GATT_PERM_READ_ENCRYPT, read_hids_report_ref,
                       NULL, &output_indicators),
#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_OUTPUT)

    BT_GATT_CHARACTERISTIC(BT_UUID_HIDS_CTRL_POINT, BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                           BT_GATT_PERM_WRITE, NULL, write_ctrl_point, &ctrl_point));

struct bt_conn *destination_connection_alt(void) {
    struct bt_conn *conn;
    bt_addr_le_t *addr = zmk_ble_active_profile_addr();
    LOG_DBG("Address pointer %p", addr);
    if (!bt_addr_le_cmp(addr, BT_ADDR_LE_ANY)) {
        LOG_WRN("Not sending, no active address for current profile");
        return NULL;
    } else if ((conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, addr)) == NULL) {
        LOG_WRN("Not sending, not connected to active profile");
        return NULL;
    }

    return conn;
}

K_THREAD_STACK_DEFINE(hog_alt_q_stack, CONFIG_ZMK_BLE_THREAD_STACK_SIZE);

struct k_work_q hog_alt_work_q;

#if IS_ENABLED(CONFIG_ZMK_HID_IO_JOYSTICK)

K_MSGQ_DEFINE(zmk_hog_joystick_alt_msgq, sizeof(struct zmk_hid_joystick_report_body_alt),
              CONFIG_ZMK_HID_IO_BLE_JOYSTICK_REPORT_QUEUE_SIZE, 4);

void send_joystick_report_alt_callback(struct k_work *work) {
    struct zmk_hid_joystick_report_body_alt report;
    while (k_msgq_get(&zmk_hog_joystick_alt_msgq, &report, K_NO_WAIT) == 0) {
        struct bt_conn *conn = destination_connection_alt();
        if (conn == NULL) {
            return;
        }

        struct bt_gatt_notify_params notify_params = {
            
            .attr = &hog_svc_alt.attrs[5],
            // .attr = &hog_svc_alt.attrs[9],

            .data = &report,
            .len = sizeof(report),
        };

        int err = bt_gatt_notify_cb(conn, &notify_params);
        if (err == -EPERM) {
            bt_conn_set_security(conn, BT_SECURITY_L2);
        } else if (err) {
            LOG_DBG("Error notifying %d", err);
        }

        bt_conn_unref(conn);
    }
};

K_WORK_DEFINE(hog_alt_joystick_work, send_joystick_report_alt_callback);

int zmk_hog_send_joystick_report_alt(struct zmk_hid_joystick_report_body_alt *report) {
    int err = k_msgq_put(&zmk_hog_joystick_alt_msgq, report, K_MSEC(100));
    if (err) {
        switch (err) {
        case -EAGAIN: {
            LOG_WRN("joystick message queue full, popping first message and queueing again");
            struct zmk_hid_joystick_report_body_alt discarded_report;
            k_msgq_get(&zmk_hog_joystick_alt_msgq, &discarded_report, K_NO_WAIT);
            return zmk_hog_send_joystick_report_alt(report);
        }
        default:
            LOG_WRN("Failed to queue joystick report to send (%d)", err);
            return err;
        }
    }

    k_work_submit_to_queue(&hog_alt_work_q, &hog_alt_joystick_work);

    return 0;
};
#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_JOYSTICK)

#if IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)

K_MSGQ_DEFINE(zmk_hog_mouse_alt_msgq, sizeof(struct zmk_hid_mouse_report_body_alt),
              CONFIG_ZMK_HID_IO_BLE_MOUSE_REPORT_QUEUE_SIZE, 4);

void send_mouse_report_alt_callback(struct k_work *work) {
    struct zmk_hid_mouse_report_body_alt report;
    while (k_msgq_get(&zmk_hog_mouse_alt_msgq, &report, K_NO_WAIT) == 0) {
        struct bt_conn *conn = destination_connection_alt();
        if (conn == NULL) {
            return;
        }

        struct bt_gatt_notify_params notify_params = {
            
            .attr = &hog_svc_alt.attrs[5],
            // .attr = &hog_svc_alt.attrs[9],

            .data = &report,
            .len = sizeof(report),
        };

        int err = bt_gatt_notify_cb(conn, &notify_params);
        if (err == -EPERM) {
            bt_conn_set_security(conn, BT_SECURITY_L2);
        } else if (err) {
            LOG_DBG("Error notifying %d", err);
        }

        bt_conn_unref(conn);
    }
};

K_WORK_DEFINE(hog_alt_mouse_work, send_mouse_report_alt_callback);

int zmk_hog_send_mouse_report_alt(struct zmk_hid_mouse_report_body_alt *report) {
    int err = k_msgq_put(&zmk_hog_mouse_alt_msgq, report, K_MSEC(100));
    if (err) {
        switch (err) {
        case -EAGAIN: {
            LOG_WRN("mouse message queue full, popping first message and queueing again");
            struct zmk_hid_mouse_report_body_alt discarded_report;
            k_msgq_get(&zmk_hog_mouse_alt_msgq, &discarded_report, K_NO_WAIT);
            return zmk_hog_send_mouse_report_alt(report);
        }
        default:
            LOG_WRN("Failed to queue mouse report to send (%d)", err);
            return err;
        }
    }

    k_work_submit_to_queue(&hog_alt_work_q, &hog_alt_mouse_work);

    return 0;
};
#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)

static int zmk_hog_init(void) {
    static const struct k_work_queue_config queue_config = {.name = "HID Over GATT Send Work"};
    k_work_queue_start(&hog_alt_work_q, hog_alt_q_stack, K_THREAD_STACK_SIZEOF(hog_alt_q_stack),
                       CONFIG_ZMK_BLE_THREAD_PRIORITY, &queue_config);

    return 0;
}

SYS_INIT(zmk_hog_init, APPLICATION, CONFIG_ZMK_BLE_INIT_PRIORITY);
