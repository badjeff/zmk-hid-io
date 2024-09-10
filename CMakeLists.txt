zephyr_library()

if ((NOT CONFIG_ZMK_SPLIT) OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL)

  zephyr_library_sources_ifdef(CONFIG_ZMK_INPUT_BEHAVIOR_FWD_TO_HID_IO src/behaviors/input_behavior_fwd_to_hid_io.c)
  zephyr_library_sources_ifdef(CONFIG_ZMK_BEHAVIOR_HID_IO_KEY_PRESS src/behaviors/behavior_hid_io_key_press.c)

  zephyr_library_sources_ifdef(CONFIG_ZMK_HID_IO src/hid-io/hid_io.c)
  zephyr_library_sources_ifdef(CONFIG_ZMK_HID_IO src/hid-io/endpoints.c)
  zephyr_library_sources_ifdef(CONFIG_ZMK_HID_IO src/hid-io/hid.c)
  if (CONFIG_ZMK_BLE)
    zephyr_library_sources_ifdef(CONFIG_ZMK_HID_IO src/hid-io/hog.c)
  endif()
  zephyr_library_sources_ifdef(CONFIG_ZMK_HID_IO src/hid-io/usb_hid.c)
  zephyr_include_directories(include)

  zephyr_include_directories(${APPLICATION_SOURCE_DIR}/include)

endif()
