/*
 * Copyright (c) XXXX The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_bt_indicator

/* ====== Dependencies ====== */
// Logs
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// Zephyr Imports
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led_strip.h>
#include <drivers/behavior.h>

// ZMK Imports
#include <zmk/behavior.h>
#include <zmk/keys.h>
#include <zmk/rgb_underglow.h>
#include <zmk/endpoints.h>
#include <zmk/ble.h>

// Events
#include <zmk/events/ble_active_profile_changed.h>

// Module Imports
#include <dt-bindings/zmk/bt_indicator.h>
/* ====== Dependencies ====== */

//#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

/* ====== Defines ====== */
#define ACTIVE_LED_COLOR (struct zmk_led_hsb) {.h = 240, .s = 100, .b = 100}
/* ====== Defines ====== */

/* ====== Properties ====== */
static uint8_t profile_leds[] = {5, 4, 3, 2, 1};
static bool is_indicator_active = false;

struct zmk_led_hsb prev_color;
int prev_effect = -99;
bool prev_on_off_state = false;
/* ====== Properties ====== */

/* ====== LED State Management ====== */
static void save_current_led_state() {
    prev_color = zmk_rgb_underglow_calc_hue(0);
    prev_effect = zmk_rgb_underglow_calc_effect(0);
    zmk_rgb_underglow_get_state(&prev_on_off_state);
}

static void restore_prev_led_state() {
    // Restore previous effect if it was set
    zmk_rgb_underglow_select_effect(prev_effect);

    // Only restore color if it was previously set (prevents restoring to default if no color was set before)
    if (prev_color.h > 0 || prev_color.s > 0 || prev_color.b > 0) {
        zmk_rgb_underglow_set_hsb(prev_color);
    }
    
    // Restore on/off state
    if (prev_on_off_state) {
        zmk_rgb_underglow_on();
    } else {
        zmk_rgb_underglow_off();
    }
}
/* ====== LED State Management ====== */

/* ====== Helper Functions ====== */
static void refresh_bt_leds() {
    if (!is_indicator_active) {
        LOG_DBG("BT indicator not active, skipping refresh");
        return;
    }

    // Turn on rgb underglow if it's not already on
    zmk_rgb_underglow_on();
    
    // Set the color to the active color
    uint8_t active_profile = zmk_ble_active_profile_index();

    for (int i = 0; i < sizeof(profile_leds); i++) {
        if (i == active_profile) {
            zmk_rgb_underglow_set_layered_hsb_index(profile_leds[i], ACTIVE_LED_COLOR);
        } else {
            zmk_rgb_underglow_set_layered_hsb_index(profile_leds[i], (struct zmk_led_hsb){0, 0, 0});
        }
    }
}
/* ====== Helper Functions ====== */

/* ====== Keypress Handlers ====== */
void set_bt_indicator_state(bool active) {
    is_indicator_active = active;

    refresh_bt_leds();
}

static int on_bt_indicator_binding_pressed(struct zmk_behavior_binding *binding,
                                           struct zmk_behavior_binding_event event) {
    save_current_led_state();
    set_bt_indicator_state(true);

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_bt_indicator_binding_released(struct zmk_behavior_binding *binding,
                                            struct zmk_behavior_binding_event event) {
    restore_prev_led_state();
    set_bt_indicator_state(false);

    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_bt_indicator_driver_api = {
    .binding_pressed = on_bt_indicator_binding_pressed,
    .binding_released = on_bt_indicator_binding_released,
    .locality = BEHAVIOR_LOCALITY_GLOBAL
};
/* ====== Keypress Handlers ====== */

/* ====== ZMK Bluetooth Profile Updated Events ====== */
static int on_bt_profile_changed(const zmk_event_t *eh) {
    refresh_bt_leds();
    return ZMK_BEHAVIOR_OPAQUE;
}

ZMK_LISTENER(behavior_bt_indicator, on_bt_profile_changed);
ZMK_SUBSCRIPTION(behavior_bt_indicator, zmk_ble_active_profile_changed);
/* ====== ZMK Bluetooth Profile Updated Events ====== */

/* ====== ZMK Behaviour Registration ====== */
static int bt_indicator_init(const struct device *dev) {
    return 0;
};

#define BEHAVIOR_BT_INDICATOR_INST(n)                                                       \
    /*static struct behavior_bt_indicator_config behavior_bt_indicator_config_##n = {         \
        .default_layer = DT_INST_PROP(n, default_layer),                                    \
    };*/                                                                                      \
                                                                                            \
    BEHAVIOR_DT_INST_DEFINE(n,                                                              \
                            bt_indicator_init,                                              \
                            NULL,                                                           \
                            NULL,                                                           \
                            NULL /*&behavior_bt_indicator_config_##n*/,                     \
                            POST_KERNEL,                                                    \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                            \
                            &behavior_bt_indicator_driver_api);

DT_INST_FOREACH_STATUS_OKAY(BEHAVIOR_BT_INDICATOR_INST)
/* ====== ZMK Behaviour Registration ====== */

//#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
