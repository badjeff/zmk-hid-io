# HID IO Module for ZMK

This module add new HID Usage Page for ZMK.

## What it does

It allow develop to add custom hid usage page to ZMK.

## Installation

Include this project on your ZMK's west manifest in `config/west.yml`:

```yaml
manifest:
  ...
  projects:
    ...
    - name: zmk-hid-io
      remote: badjeff
      revision: main
    ...
```

Now enable the config in your `<shield>.config` file (read the Kconfig file to find out all possible options):

```conf

# Zephyr Config
CONFIG_USB_DEVICE_HID=y
CONFIG_USB_HID_DEVICE_COUNT=2

# Module Config
CONFIG_ZMK_HID_IO=y

# Enable hid joystick, to recognized as a jotstick/gamepad from host
CONFIG_ZMK_HID_IO_JOYSTICK=y

# Enable hid mouse, to recognized as a jotstick/gamepad from host
CONFIG_ZMK_HID_IO_MOUSE=y

# Enable hid haptic feedback, to accept HID Output from host. See https://github.com/badjeff/zmk-haptic-tester-macos
CONFIG_ZMK_HID_IO_OUTPUT=y

```

While module is enabling, a new HID interface shall available from usage page `0xFF0C`. The actual value of usage page and report id could be modified in `include/zmk/hid-io/hid.h`.


## How it actually works

This module is only convert input events form input subsystem into hid input/output reports.
See below config to explain how it cooperates with other input driver modules to setup a joystick/gamepad hid device.

ZMK modules that used and should add to your `config/west.yml` for below `shield.keymap` sample.
- [zmk-input-behavior-listener](https://github.com/badjeff/zmk-input-behavior-listener)
- [zmk-analog-input-driver](https://github.com/badjeff/zmk-analog-input-driver)

```keymap

// index of keymap layers
#define DEF 0 // default layer

/ {
        /* input config for mouse move mode on default layer (DEF & MSK) */
        /* see README.md in module [zmk-input-behavior-listener] to setup */
        analog_input_ibl {
                /* new forked compatible name */
                compatible = "zmk,input-behavior-listener";
                
                /* alias to the actial analog input device */
                /* see README.md in module [zmk-analog-input-driver] to setup */
                device = <&anin0>;

                /* only enable in default layer (DEF) & mouse key layer (MSK) */
                layers = <DEF>;

                /* event code value to override raw input event */
                /* designed for switching to mouse scroll, xy-swap, precise-mode+, etc */
                evt-type = <INPUT_EV_REL>;
                x-input-code = <INPUT_REL_X>;
                y-input-code = <INPUT_REL_Y>;
                scale-multiplier = <1>;
                scale-divisor = <1>;

                /* bind a behavior */
                bindings = <&ib_fwd_to_hid_io>;
        };

        /* Setup behavior to intecept input and forward to new usage page  */
        /* Use CONFIG_ZMK_HID_IO_JOYSTICK or CONFIG_ZMK_HID_IO_MOUSE to switch usage in HID Descriptor */
        /* Or, build your own HID Descriptor */
        ib_fwd_to_hid_io: ib_forward_fwd_to_hid_io {
                compatible = "zmk,input-behavior-fwd-to-hid-io";
                #binding-cells = <0>;
        };

        keymap {
                compatible = "zmk,keymap";
                DEF_layer {
                        bindings = < .... ... >;
                };
       };

};
```
