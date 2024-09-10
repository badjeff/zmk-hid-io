/*
 * Copyright (c) 2021 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_hid_io_key_press

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/behavior.h>
#include <zmk/hid.h>
#include <zephyr/input/input.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int behavior_hid_io_key_press_init(const struct device *dev) { return 0; };

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    LOG_DBG("position %d HID IO BUTTON 0x%02X", event.position, binding->param1);
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    input_report_key(dev, INPUT_BTN_0 + binding->param1, 1, true, K_FOREVER);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    LOG_DBG("position %d HID IO BUTTON 0x%02X", event.position, binding->param1);
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    input_report_key(dev, INPUT_BTN_0 + binding->param1, 0, true, K_FOREVER);
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_hid_io_key_press_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

#define JSKP_INST(n)                                                       \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_hid_io_key_press_init, NULL,       \
                            NULL, NULL, POST_KERNEL,                       \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,           \
                            &behavior_hid_io_key_press_driver_api);

DT_INST_FOREACH_STATUS_OKAY(JSKP_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
