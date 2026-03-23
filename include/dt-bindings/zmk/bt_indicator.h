
#pragma once

/* ====== Constant Definitions ====== */
#define STRIP_CHOSEN DT_CHOSEN(zmk_underglow)
#define STRIP_NUM_PIXELS DT_PROP(STRIP_CHOSEN, chain_length)

#define ACTIVE_LED_COLOUR {.h = 0, .s = 0, .b = 0}
#define INACTIVE_LED_COLOUR {.h = 240, .s = 100, .b = 50}
/* ====== Constant Definitions ====== */

/* ====== Struct Definitions ====== */
// Instance-specific Data struct
struct behavior_bt_indicator_data {};

// Instance-specific Config struct
struct behavior_bt_indicator_config {};
/* ====== Struct Definitions ====== */