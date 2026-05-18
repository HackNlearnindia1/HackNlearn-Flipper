#pragma once

/* ── Firmware identity ─────────────────────── */
#define HNL_NAME    "HackNlearn India"
#define HNL_VERSION "v1.0.0"
#define HNL_DEV     "HNL-Flipper"

/* ── App screens ───────────────────────────── */
typedef enum {
    SceneBoot   = 0,
    SceneMenu   = 1,
    SceneSubghz = 2,
    SceneNfc    = 3,
    SceneBadusb = 4,
    SceneIr     = 5,
    SceneGpio   = 6,
    SceneBle    = 7,
} HNLScene;

/* ── Shared app state ──────────────────────── */
typedef struct {
    uint8_t  selected;   /* highlighted menu row   */
    uint8_t  scroll;     /* first visible row index */
    HNLScene scene;      /* active screen           */
    uint32_t boot_tick;  /* furi_get_tick() at boot */
} HNLState;
