/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_input_behavior_fwd_to_hid_io

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/input/input.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/keymap.h>
#include <zmk/behavior.h>

#if IS_ENABLED(CONFIG_ZMK_HID_IO)
#include <zmk/hid-io/endpoints.h>
#include <zmk/hid-io/hid.h>
#endif

// #if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct behavior_fwd_to_hid_io_config {
};

enum fwd_to_hid_io_xy_data_mode {
    HID_IO_JOYSTICK_XY_DATA_MODE_NONE,
    HID_IO_JOYSTICK_XY_DATA_MODE_REL,
    HID_IO_JOYSTICK_XY_DATA_MODE_ABS,
};

struct fwd_to_hid_io_xy_data {
    enum fwd_to_hid_io_xy_data_mode mode;
    int16_t x;
    int16_t y;
};

struct behavior_fwd_to_hid_io_data {
    const struct device *dev;
    struct fwd_to_hid_io_xy_data data;
    struct fwd_to_hid_io_xy_data wheel_data;
    uint8_t button_set;
    uint8_t button_clear;
};

static void handle_rel_code(struct behavior_fwd_to_hid_io_data *data, struct input_event *evt) {
    switch (evt->code) {
    case INPUT_REL_X:
        data->data.mode = HID_IO_JOYSTICK_XY_DATA_MODE_REL;
        data->data.x += evt->value;
        break;
    case INPUT_REL_Y:
        data->data.mode = HID_IO_JOYSTICK_XY_DATA_MODE_REL;
        data->data.y += evt->value;
        break;
    case INPUT_REL_WHEEL:
        data->wheel_data.mode = HID_IO_JOYSTICK_XY_DATA_MODE_REL;
        data->wheel_data.y += evt->value;
        break;
    case INPUT_REL_HWHEEL:
        data->wheel_data.mode = HID_IO_JOYSTICK_XY_DATA_MODE_REL;
        data->wheel_data.x += evt->value;
        break;
    default:
        break;
    }
}

static void handle_key_code(struct behavior_fwd_to_hid_io_data *data, struct input_event *evt) {
    int8_t btn;

    switch (evt->code) {
    case INPUT_BTN_0:
    case INPUT_BTN_1:
    case INPUT_BTN_2:
    case INPUT_BTN_3:
    case INPUT_BTN_4:
        btn = evt->code - INPUT_BTN_0;
        if (evt->value > 0) {
            WRITE_BIT(data->button_set, btn, 1);
        } else {
            WRITE_BIT(data->button_clear, btn, 1);
        }
        break;
    default:
        break;
    }
}

static void clear_xy_data(struct fwd_to_hid_io_xy_data *data) {
    data->x = data->y = 0;
    data->mode = HID_IO_JOYSTICK_XY_DATA_MODE_NONE;
}

static int to_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {

    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    struct behavior_fwd_to_hid_io_data *data = 
        (struct behavior_fwd_to_hid_io_data *)dev->data;
    // const struct behavior_fwd_to_hid_io_config *cfg = dev->config;
    
    struct input_event *evt = (struct input_event *)event.position;

    switch (evt->type) {
    case INPUT_EV_REL:
        handle_rel_code(data, evt);
        break;
    case INPUT_EV_KEY:
        handle_key_code(data, evt);
        break;
    }

    if (evt->sync) {

#if IS_ENABLED(CONFIG_ZMK_HID_IO)

    #if IS_ENABLED(CONFIG_ZMK_HID_IO_JOYSTICK)
        if (data->data.mode == HID_IO_JOYSTICK_XY_DATA_MODE_REL) {
            zmk_hid_joy2_movement_set(data->data.x, data->data.y);
        }
        if (data->button_set != 0) {
            for (int i = 0; i < ZMK_HID_MOUSE_NUM_BUTTONS; i++) {
                if ((data->button_set & BIT(i)) != 0) {
                    zmk_hid_joy2_button_press(i);
                }
            }
        }
        if (data->button_clear != 0) {
            for (int i = 0; i < ZMK_HID_MOUSE_NUM_BUTTONS; i++) {
                if ((data->button_clear & BIT(i)) != 0) {
                    zmk_hid_joy2_button_release(i);
                }
            }
        }
        zmk_endpoints_send_joystick_report_alt();
        zmk_hid_joy2_movement_set(0, 0);
    #endif

    #if IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)
        if (data->wheel_data.mode == HID_IO_JOYSTICK_XY_DATA_MODE_REL) {
            zmk_hid_mou2_scroll_set(data->wheel_data.x, data->wheel_data.y);
        }
        if (data->data.mode == HID_IO_JOYSTICK_XY_DATA_MODE_REL) {
            zmk_hid_mou2_movement_set(data->data.x, data->data.y);
        }
        if (data->button_set != 0) {
            for (int i = 0; i < ZMK_HID_MOUSE_NUM_BUTTONS; i++) {
                if ((data->button_set & BIT(i)) != 0) {
                    zmk_hid_mou2_button_press(i);
                }
            }
        }
        if (data->button_clear != 0) {
            for (int i = 0; i < ZMK_HID_MOUSE_NUM_BUTTONS; i++) {
                if ((data->button_clear & BIT(i)) != 0) {
                    zmk_hid_mou2_button_release(i);
                }
            }
        }
        zmk_endpoints_send_mouse_report_alt();
        zmk_hid_mou2_scroll_set(0, 0);
        zmk_hid_mou2_movement_set(0, 0);
    #endif

#endif

        clear_xy_data(&data->data);
        clear_xy_data(&data->wheel_data);

        data->button_set = data->button_clear = 0;
    }

    return ZMK_BEHAVIOR_OPAQUE;
}

static int input_behavior_to_init(const struct device *dev) {
    struct behavior_fwd_to_hid_io_data *data = dev->data;
    data->dev = dev;
    return 0;
};

static const struct behavior_driver_api behavior_fwd_to_hid_io_driver_api = {
    .binding_pressed = to_keymap_binding_pressed,
};

#define KP_INST(n)                                                                         \
    static struct behavior_fwd_to_hid_io_data behavior_fwd_to_hid_io_data_##n = {};        \
    static struct behavior_fwd_to_hid_io_config behavior_fwd_to_hid_io_config_##n = {      \
    };                                                                                     \
    BEHAVIOR_DT_INST_DEFINE(n, input_behavior_to_init, NULL,                               \
                            &behavior_fwd_to_hid_io_data_##n,                              \
                            &behavior_fwd_to_hid_io_config_##n,                            \
                            POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,              \
                            &behavior_fwd_to_hid_io_driver_api);

DT_INST_FOREACH_STATUS_OKAY(KP_INST)

// #endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
