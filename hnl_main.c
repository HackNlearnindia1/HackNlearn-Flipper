/**
 * HackNlearn India — Flipper Zero Firmware
 * hnl_main.c  —  Entry point & event loop
 *
 * Verified Furi / GUI APIs used:
 *
 *   furi_message_queue_alloc(capacity, item_size)  → FuriMessageQueue*
 *   furi_message_queue_free(queue)
 *   furi_message_queue_put(queue, item, timeout)   → FuriStatus
 *   furi_message_queue_get(queue, item, timeout)   → FuriStatus
 *
 *   view_port_alloc()                              → ViewPort*
 *   view_port_free(vp)
 *   view_port_draw_callback_set(vp, cb, ctx)
 *   view_port_input_callback_set(vp, cb, ctx)
 *   view_port_update(vp)
 *   view_port_enabled_set(vp, bool)
 *
 *   furi_record_open("gui")                        → Gui*
 *   furi_record_close("gui")
 *   gui_add_view_port(gui, vp, GuiLayerFullscreen)
 *   gui_remove_view_port(gui, vp)
 *
 *   furi_get_tick()                                → uint32_t  (ms ticks)
 *   FURI_LOG_I(tag, fmt, ...)
 *   UNUSED(x)
 */

#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include "hnl_main.h"
#include "hnl_draw.h"

#define TAG "HNL"

/* ── Number of menu items (keep in sync with hnl_draw.c) ──────── */
#define MENU_ROWS    6
#define MENU_VISIBLE 4

/* ── App context ───────────────────────────────────────────────── */
typedef struct {
    Gui*              gui;
    ViewPort*         view_port;
    FuriMessageQueue* input_queue;
    HNLState          state;
} HNLApp;

/* ── Draw callback (called from GUI thread) ────────────────────── */
static void hnl_draw_cb(Canvas* canvas, void* ctx) {
    HNLApp* app = ctx;

    switch(app->state.scene) {
    case SceneBoot:
        hnl_draw_boot(canvas, &app->state);
        break;
    case SceneMenu:
        hnl_draw_menu(canvas, &app->state);
        break;
    case SceneSubghz:
        hnl_draw_subghz(canvas);
        break;
    case SceneNfc:
        hnl_draw_nfc(canvas);
        break;
    case SceneBadusb:
        hnl_draw_badusb(canvas);
        break;
    case SceneIr:
        hnl_draw_ir(canvas);
        break;
    case SceneGpio:
        hnl_draw_gpio(canvas);
        break;
    case SceneBle:
        hnl_draw_ble(canvas);
        break;
    default:
        hnl_draw_menu(canvas, &app->state);
        break;
    }
}

/* ── Input callback (called from GUI thread → puts to queue) ───── */
static void hnl_input_cb(InputEvent* event, void* ctx) {
    HNLApp* app = ctx;
    furi_message_queue_put(app->input_queue, event, FuriWaitForever);
}

/* ── Menu helpers ──────────────────────────────────────────────── */
static void menu_nav(HNLState* s, int8_t delta) {
    int8_t next = (int8_t)s->selected + delta;
    if(next < 0)          next = 0;
    if(next >= MENU_ROWS) next = MENU_ROWS - 1;
    s->selected = (uint8_t)next;

    /* scroll window */
    if(s->selected < s->scroll)
        s->scroll = s->selected;
    if(s->selected >= s->scroll + MENU_VISIBLE)
        s->scroll = s->selected - MENU_VISIBLE + 1;
}

/* Map menu row → scene */
static const HNLScene SCENE_MAP[MENU_ROWS] = {
    SceneSubghz,
    SceneNfc,
    SceneBadusb,
    SceneIr,
    SceneGpio,
    SceneBle,
};

/* ── Entry point ───────────────────────────────────────────────── */
int32_t hnl_main_app(void* p) {
    UNUSED(p);
    FURI_LOG_I(TAG, HNL_NAME " " HNL_VERSION " starting");

    /* allocate app */
    HNLApp* app = malloc(sizeof(HNLApp));
    furi_assert(app);

    /* init state */
    app->state.selected  = 0;
    app->state.scroll    = 0;
    app->state.scene     = SceneBoot;
    app->state.boot_tick = furi_get_tick();

    /* input queue */
    app->input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    /* view port */
    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, hnl_draw_cb,   app);
    view_port_input_callback_set(app->view_port, hnl_input_cb, app);

    /* attach to GUI */
    app->gui = furi_record_open("gui");
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    /* show boot screen for ~2 s */
    InputEvent event;
    bool running = true;

    while(running) {
        /* switch from boot to menu after 2000 ms */
        if(app->state.scene == SceneBoot) {
            uint32_t elapsed = furi_get_tick() - app->state.boot_tick;
            if(elapsed >= 2000) {
                app->state.scene = SceneMenu;
                view_port_update(app->view_port);
            }
        }

        /* non-blocking input poll (100 ms timeout) */
        FuriStatus status =
            furi_message_queue_get(app->input_queue, &event, 100);

        if(status != FuriStatusOk) {
            /* timeout — just redraw and loop */
            view_port_update(app->view_port);
            continue;
        }

        /* handle only press & repeat events */
        if(event.type != InputTypePress && event.type != InputTypeRepeat) {
            continue;
        }

        if(app->state.scene == SceneBoot) {
            /* any key skips boot */
            app->state.scene = SceneMenu;

        } else if(app->state.scene == SceneMenu) {
            switch(event.key) {
            case InputKeyUp:
                menu_nav(&app->state, -1);
                break;
            case InputKeyDown:
                menu_nav(&app->state, 1);
                break;
            case InputKeyOk:
                app->state.scene = SCENE_MAP[app->state.selected];
                break;
            case InputKeyBack:
                running = false;
                break;
            default:
                break;
            }

        } else {
            /* any tool screen → Back returns to menu */
            if(event.key == InputKeyBack) {
                app->state.scene = SceneMenu;
            }
        }

        view_port_update(app->view_port);
    }

    /* cleanup — exact order matters */
    view_port_enabled_set(app->view_port, false);
    gui_remove_view_port(app->gui, app->view_port);
    furi_record_close("gui");
    view_port_free(app->view_port);
    furi_message_queue_free(app->input_queue);
    free(app);

    FURI_LOG_I(TAG, HNL_NAME " exited cleanly");
    return 0;
}
