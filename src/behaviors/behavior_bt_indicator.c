/*
 * Copyright (c) XXXX The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_bt_indicator

// Dependencies
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <drivers/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/behavior.h>
#include <zmk/keys.h>

#include <zephyr/kernel.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk-bt-indicator/bt_indicator.h>
#include <zmk/rgb_underglow.h>
#include <zmk/endpoints.h>
#include <zmk/ble.h>

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

/* ====== Struct Definitions ====== */
// Instance-specific Data struct
struct behavior_bt_indicator_data {};

// Instance-specific Config struct
struct behavior_bt_indicator_config {};
/* ====== Struct Definitions ====== */

/* ====== Properties ====== */
static uint8_t profile_leds[] = {7, 8, 9, 10, 11};
static bool is_indicator_active = false;

struct zmk_led_hsb inactive_colour = INACTIVE_LED_COLOUR;
struct zmk_led_hsb active_colour = ACTIVE_LED_COLOUR;
/* ====== Properties ====== */

/* ====== Initialization ====== */
static int bt_indicator_init(const struct device *dev) {
    return 0;
};
/* ====== Initialization ====== */

/* ====== Helper Functions ====== */
void refresh_bt_leds() {
    uint8_t active_profile = zmk_ble_active_profile_index();
    
    for (int i = 0; i < 5; i++) {
        if (is_indicator_active && i == active_profile) {
            // Light up active profile in Blue
            zmk_rgb_underglow_set_hsb_at_index(profile_leds[i], inactive_colour);
        } else {
            // Turn off if key released OR not the active profile
            zmk_rgb_underglow_set_hsb_at_index(profile_leds[i], active_colour);
        }
    }
}

void set_bt_indicator_state(bool active) {
    is_indicator_active = active;
    refresh_bt_leds();
}
/* ====== Helper Functions ====== */

/* ====== ZMK Events ====== */
static int on_bt_profile_changed(const zmk_event_t *eh);

ZMK_LISTENER(behavior_bt_indicator, on_bt_profile_changed);
ZMK_SUBSCRIPTION(behavior_bt_indicator, zmk_ble_active_profile_changed);

// Listener for Profile Changes (updates if key is already held)
static int on_bt_profile_changed(const zmk_event_t *eh) {
    refresh_bt_leds();
    return ZMK_BEHAVIOR_OPAQUE;
}
/* ====== ZMK Events ====== */

/* ====== Keypress Handlers ====== */
static int on_bt_indicator_binding_pressed(struct zmk_behavior_binding *binding,
                                           struct zmk_behavior_binding_event event) {
    set_bt_indicator_state(true);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_bt_indicator_binding_released(struct zmk_behavior_binding *binding,
                                            struct zmk_behavior_binding_event event) {
    set_bt_indicator_state(false);
    return ZMK_BEHAVIOR_OPAQUE;
}
/* ====== Keypress Handlers ====== */

/* ====== ZMK Behaviour Registration ====== */
static const struct behavior_driver_api bt_indicator_driver_api = {
    .binding_pressed = on_bt_indicator_binding_pressed,
    .binding_released = on_bt_indicator_binding_released,
    .locality = BEHAVIOR_LOCALITY_GLOBAL
};

BEHAVIOR_DT_INST_DEFINE(0,                                                  // Instance Number (0)
                        bt_indicator_init,                                  // Initialization Function
                        NULL,                                               // Power Management Device Pointer
                        &bt_indicator_data,                                 // Behavior Data Pointer
                        &bt_indicator_config,                               // Behavior Configuration Pointer
                        POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT    // Initialization Level, Device Priority
                        &bt_indicator_driver_api);                          // API struct
/* ====== ZMK Behaviour Registration ====== */Q

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
