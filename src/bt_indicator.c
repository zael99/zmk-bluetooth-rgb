#include <zephyr/kernel.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/rgb_underglow.h>
#include <zmk/endpoints.h>
#include <zmk/ble.h>

static uint8_t profile_leds[] = {7, 8, 9}; 
static bool is_indicator_active = false;

// Function to refresh the LEDs based on current state
void refresh_bt_leds() {
    uint8_t active_profile = zmk_ble_active_profile_index();
    
    for (int i = 0; i < 3; i++) {
        if (is_indicator_active && i == active_profile) {
            // Light up active profile in Blue
            zmk_rgb_underglow_set_hsb_at_index(profile_leds[i], (struct zmk_led_hsb){.h = 240, .s = 100, .b = 50});
        } else {
            // Turn off if key released OR not the active profile
            zmk_rgb_underglow_set_hsb_at_index(profile_leds[i], (struct zmk_led_hsb){.h = 0, .s = 0, .b = 0});
        }
    }
}

// Listener for Profile Changes (updates if key is already held)
static int led_profile_handler(const zmk_event_t *eh) {
    refresh_bt_leds();
    return 0;
}

ZMK_LISTENER(bt_indicator, led_profile_handler);
ZMK_SUBSCRIPTION(bt_indicator, zmk_ble_active_profile_changed);

// Exported function for your keymap behavior
void set_bt_indicator_state(bool active) {
    is_indicator_active = active;
    refresh_bt_leds();
}