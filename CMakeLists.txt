zephyr_library()

if ((NOT CONFIG_ZMK_SPLIT) OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL)

  zephyr_library_sources_ifdef(CONFIG_ZMK_INPUT_BEHAVIOR_FWD_TO_HID_IO src/input_behavior_fwd_to_hid_io.c)

  zephyr_library_sources_ifdef(CONFIG_ZMK_HID_IO src/hid_io.c)
  zephyr_library_sources_ifdef(CONFIG_ZMK_HID_IO src/endpoints.c)
  zephyr_library_sources_ifdef(CONFIG_ZMK_HID_IO src/hid.c)
  if (CONFIG_ZMK_BLE)
    zephyr_library_sources_ifdef(CONFIG_ZMK_HID_IO src/hog.c)
  endif()
  zephyr_library_sources_ifdef(CONFIG_ZMK_HID_IO src/usb_hid.c)
  zephyr_include_directories(include)

  zephyr_include_directories(${APPLICATION_SOURCE_DIR}/include)

endif()
