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

# Add one more USB_HID_DEVICE. 
# Hence, zmk default use 2 HID, HID_0 for keyboard, HID_1 for mouse.
CONFIG_USB_HID_DEVICE_COUNT=3

# Module Config
CONFIG_ZMK_HID_IO=y

# Enable hid joystick
CONFIG_ZMK_HID_IO_JOYSTICK=y

# Enable hid mouse
CONFIG_ZMK_HID_IO_MOUSE=y

# Enable hid haptic feedback
CONFIG_ZMK_HID_IO_OUTPUT=y

# Enable logging
CONFIG_ZMK_HID_IO_LOG_LEVEL_DBG=y
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

#define HID_IO_USAGE_FWD_TO_MOUSE 1
#define HID_IO_USAGE_FWD_TO_JOYSTICK 2

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
        /* via hid-io-module, or build your own HID Descriptor */
        ib_fwd_to_hid_io: ib_forward_fwd_to_hid_io {
                compatible = "zmk,input-behavior-fwd-to-hid-io";
                #binding-cells = <0>;
                /* route the input as joystick report */
                usage = <HID_IO_USAGE_FWD_TO_JOYSTICK>;
        };

        /* Setup joystick key press behavior and listener */
        /* listerer shall forward to JOYSTICK usage page if CONFIG_ZMK_HID_IO_JOYSTICK enabling */
        hidiokp: hid_io_key_press {
            compatible = "zmk,behavior-hid-io-key-press";
            #binding-cells = <1>;
        };
	hidiokp_ibl {
		compatible = "zmk,input-behavior-listener";
		device = <&hidiokp>;
		layers = <DEFAULT>;
                bindings = <&ib_fwd_to_hid_io>;
	};

        keymap {
                compatible = "zmk,keymap";
                DEF_layer {
                        bindings = < .... 
                        
                        &hidiokp 0 /* joystick 1st button */
                        &hidiokp 0 /* joystick 2nd button */

                        ... >;
                };
       };

};
```
