/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <zmk/endpoints.h>

#if IS_ENABLED(CONFIG_ZMK_HID_IO_JOYSTICK)
int zmk_endpoints_send_joystick_report_alt();
#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_JOYSTICK)

#if IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)
int zmk_endpoints_send_mouse_report_alt();
#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_MOUSE)

#if IS_ENABLED(CONFIG_ZMK_HID_IO_VOLUME_KNOB)
int zmk_endpoints_send_volume_knob_report_alt();
#endif // IS_ENABLED(CONFIG_ZMK_HID_IO_VOLUME_KNOB)
