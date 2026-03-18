#include <zephyr/device.h>
#include <zmk/behaviors.h>
#include <zmk/keys.h>

extern void set_bt_indicator_state(bool active);

static int on_key_pressed(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    set_bt_indicator_state(true);
    return 0;
}

static int on_key_released(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    set_bt_indicator_state(false);
    return 0;
}

static const struct behavior_driver_api behavior_bt_glow_driver_api = {
    .binding_pressed = on_key_pressed,
    .binding_released = on_key_released,
};

DEVICE_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_bt_glow_driver_api);