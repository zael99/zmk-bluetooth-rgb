#include <zephyr/kernel.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/rgb_underglow.h>

/* * Based on our mapping:
 * Key '1' = Index 7 (or 8 depending on snake)
 * Key '2' = Index 8 (or 9)
 * Key '3' = Index 9 (or 10)
 */
static uint8_t profile_leds[] = {7, 8, 9, 10, 11}; 

static int led_profile_handler(const zmk_event_t *eh) {
    const struct zmk_ble_active_profile_changed *ev = as_zmk_ble_active_profile_changed(eh);
    if (ev == NULL) return 0;

    // 1. Clear the specific profile indicator keys first
    for (int i = 0; i < 5; i++) {
        zmk_rgb_underglow_set_hsb_at_index(profile_leds[i], (struct zmk_led_hsb){.h = 0, .s = 0, .b = 0});
    }

    // 2. Light up the LED corresponding to the active profile (0, 1, or 2)
    // We'll use Blue (Hue 240) for the active profile
    if (ev->index < 5) {
        zmk_rgb_underglow_set_hsb_at_index(profile_leds[ev->index], (struct zmk_led_hsb){.h = 240, .s = 100, .b = 50});
    }

    return 0;
}

ZMK_LISTENER(bt_indicator, led_profile_handler);
ZMK_SUBSCRIPTION(bt_indicator, zmk_ble_active_profile_changed);