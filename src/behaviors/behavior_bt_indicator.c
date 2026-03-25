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

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

/* ====== Properties ====== */
#define STRIP_CHOSEN DT_CHOSEN(zmk_underglow)
#define STRIP_NUM_PIXELS DT_PROP(STRIP_CHOSEN, chain_length)

static const struct device *led_strip;

static struct led_rgb pixels[STRIP_NUM_PIXELS];

static uint8_t profile_leds[] = {7, 8, 9, 10, 11};
static bool is_indicator_active = false;

static const struct device *led_strip;
static struct led_rgb pixels[STRIP_NUM_PIXELS];

struct zmk_led_hsb inactive_colour = INACTIVE_LED_COLOUR;
struct zmk_led_hsb active_colour = ACTIVE_LED_COLOUR;
/* ====== Properties ====== */

/* ====== Initialization ====== */
static int bt_indicator_init(const struct device *dev) {
    led_strip = DEVICE_DT_GET(STRIP_CHOSEN);
    return 0;
};
/* ====== Initialization ====== */

/* ====== Helper Functions ====== */
static struct zmk_led_hsb hsb_scale_min_max(struct zmk_led_hsb hsb) {
    hsb.b = CONFIG_ZMK_RGB_UNDERGLOW_BRT_MIN +
            (CONFIG_ZMK_RGB_UNDERGLOW_BRT_MAX - CONFIG_ZMK_RGB_UNDERGLOW_BRT_MIN) * hsb.b / BRT_MAX;
    return hsb;
}

static struct led_rgb hsb_to_rgb(struct zmk_led_hsb hsb) {
    float r = 0, g = 0, b = 0;

    uint8_t i = hsb.h / 60;
    float v = hsb.b / ((float)BRT_MAX);
    float s = hsb.s / ((float)SAT_MAX);
    float f = hsb.h / ((float)HUE_MAX) * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    switch (i % 6) {
    case 0:
        r = v;
        g = t;
        b = p;
        break;
    case 1:
        r = q;
        g = v;
        b = p;
        break;
    case 2:
        r = p;
        g = v;
        b = t;
        break;
    case 3:
        r = p;
        g = q;
        b = v;
        break;
    case 4:
        r = t;
        g = p;
        b = v;
        break;
    case 5:
        r = v;
        g = p;
        b = q;
        break;
    }

    struct led_rgb rgb = {r : r * 255, g : g * 255, b : b * 255};

    return rgb;
}

void set_pixel_color(int index, struct zmk_led_hsb color) {
    if (index > STRIP_NUM_PIXELS || index < 0) {
        // Return if outside appropriate range
        return;
    }

    pixels[index] = hsb_to_rgb(hsb_scale_min_max(color));
}

void refresh_bt_leds() {
    // First set all leds to off
    for (int i = 0; i < STRIP_NUM_PIXELS; i++) {
        set_pixel_color(i, inactive_colour);
    }

    // Anything passed here is for setting the BT indicator color
    if (!is_indicator_active) {
        return;
    }

    // Light up active profile in the active colour
    uint8_t active_profile = zmk_ble_active_profile_index();
    set_pixel_color(profile_leds[active_profile], active_colour);
    
    led_strip_update_rgb(led_strip, pixels, STRIP_NUM_PIXELS);
}
/* ====== Helper Functions ====== */

/* ====== Keypress Handlers ====== */
void set_bt_indicator_state(bool active);

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

void set_bt_indicator_state(bool active) {
    is_indicator_active = active;
    if (active) {
        zmk_rgb_underglow_off();
    } else {
        zmk_rgb_underglow_on();
    }
    refresh_bt_leds();
}
/* ====== Keypress Handlers ====== */

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

/* ====== ZMK Behaviour Registration ====== */
static const struct behavior_driver_api bt_indicator_driver_api = {
    .binding_pressed = on_bt_indicator_binding_pressed,
    .binding_released = on_bt_indicator_binding_released,
    .locality = BEHAVIOR_LOCALITY_GLOBAL
};

BEHAVIOR_DT_INST_DEFINE(0, 
                        bt_indicator_init, 
                        NULL, 
                        NULL,                               
                        NULL, 
                        POST_KERNEL,                          
                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, 
                        &bt_indicator_driver_api);
/* ====== ZMK Behaviour Registration ====== */

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
