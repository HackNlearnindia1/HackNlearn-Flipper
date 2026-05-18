/**
 * HackNlearn India — Flipper Zero Firmware
 * hnl_draw.c  —  All screen rendering
 *
 * Canvas API used (all verified from official SDK):
 *   canvas_clear(canvas)
 *   canvas_set_color(canvas, ColorBlack | ColorWhite)
 *   canvas_set_font(canvas, FontPrimary | FontSecondary | FontBigNumbers)
 *   canvas_draw_str(canvas, x, y, "text")
 *   canvas_draw_str_aligned(canvas, x, y, AlignLeft|AlignCenter|AlignRight,
 *                                         AlignTop|AlignCenter|AlignBottom, "text")
 *   canvas_draw_line(canvas, x1, y1, x2, y2)
 *   canvas_draw_box(canvas, x, y, w, h)
 *   canvas_draw_frame(canvas, x, y, w, h)
 *   canvas_draw_dot(canvas, x, y)
 *
 * Display: 128 × 64 pixels, 1-bit monochrome
 */

#include <gui/canvas.h>
#include <gui/elements.h>
#include <furi.h>
#include "hnl_main.h"
#include "hnl_draw.h"

/* ── Internal helpers ──────────────────────────────────────────── */

/** Filled top header bar with white title text */
static void header_bar(Canvas* canvas, const char* title) {
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(canvas, 0, 0, 128, 13);
    canvas_set_color(canvas, ColorWhite);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, title);
    canvas_set_color(canvas, ColorBlack);
}

/** Small hint text on the very bottom row */
static void bottom_hint(Canvas* canvas, const char* hint) {
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 56, AlignCenter, AlignTop, hint);
}

/* ── Boot screen ───────────────────────────────────────────────── */
void hnl_draw_boot(Canvas* canvas, const HNLState* s) {
    UNUSED(s);
    canvas_clear(canvas);

    /* outer border */
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_frame(canvas, 0, 0, 128, 64);

    /* logo */
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 8,  AlignCenter, AlignTop, "HackNlearn");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 22, AlignCenter, AlignTop, "I N D I A");

    /* divider */
    canvas_draw_line(canvas, 8, 32, 120, 32);

    /* subtitle */
    canvas_draw_str_aligned(canvas, 64, 36, AlignCenter, AlignTop, "Flipper Zero Firmware");
    canvas_draw_str_aligned(canvas, 64, 47, AlignCenter, AlignTop, HNL_VERSION);
    canvas_draw_str_aligned(canvas, 64, 56, AlignCenter, AlignTop, "[ Loading... ]");
}

/* ── Menu ──────────────────────────────────────────────────────── */

#define MENU_ROWS      6
#define MENU_VISIBLE   4   /* rows visible at once on 64-px screen */
#define ROW_H         12   /* pixels per row                        */
#define ROW_Y_START   14   /* y of first row                        */

static const char* const MENU_LABELS[MENU_ROWS] = {
    "Sub-GHz  Scan/Replay",
    "NFC/RFID Read/Write",
    "BadUSB   HID Scripts",
    "IR Blast Universal",
    "GPIO     UART/SPI",
    "BLE      BT Scanner",
};

void hnl_draw_menu(Canvas* canvas, const HNLState* s) {
    canvas_clear(canvas);
    header_bar(canvas, HNL_NAME);

    /* rows */
    for(uint8_t i = 0; i < MENU_VISIBLE; i++) {
        uint8_t idx = s->scroll + i;
        if(idx >= MENU_ROWS) break;

        uint8_t y = ROW_Y_START + i * ROW_H;

        if(idx == s->selected) {
            /* highlight bar */
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(canvas, 0, y, 122, ROW_H);
            canvas_set_color(canvas, ColorWhite);
        } else {
            canvas_set_color(canvas, ColorBlack);
        }

        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 3, y + ROW_H - 2, MENU_LABELS[idx]);

        canvas_set_color(canvas, ColorBlack);
    }

    /* scrollbar track */
    canvas_draw_line(canvas, 126, ROW_Y_START, 126, ROW_Y_START + MENU_VISIBLE * ROW_H);

    /* scrollbar thumb */
    uint8_t thumb_h = (MENU_VISIBLE * ROW_H) / MENU_ROWS;
    uint8_t thumb_y = ROW_Y_START + (s->scroll * MENU_VISIBLE * ROW_H / MENU_ROWS);
    canvas_draw_box(canvas, 124, thumb_y, 3, thumb_h);

    bottom_hint(canvas, "OK=Open  Back=Exit");
}

/* ── Sub-GHz ───────────────────────────────────────────────────── */
void hnl_draw_subghz(Canvas* canvas) {
    canvas_clear(canvas);
    header_bar(canvas, "SUB-GHZ");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 23, "Freq : 433.920 MHz");
    canvas_draw_str(canvas, 2, 33, "Mode : SCANNING");
    canvas_draw_str(canvas, 2, 43, "RSSI : -72 dBm");

    /* signal bars */
    canvas_draw_str(canvas, 2, 53, "Sig:");
    for(uint8_t b = 0; b < 8; b++) {
        uint8_t bh = 2 + b;
        uint8_t bx = 28 + b * 5;
        uint8_t by = 54 - bh;
        canvas_draw_box(canvas, bx, by, 4, bh);
    }

    bottom_hint(canvas, "Back=Menu");
}

/* ── NFC ───────────────────────────────────────────────────────── */
void hnl_draw_nfc(Canvas* canvas) {
    canvas_clear(canvas);
    header_bar(canvas, "NFC / RFID");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 23, "Mode : ISO14443A");
    canvas_draw_str(canvas, 2, 33, "UID  : --:--:--:--");
    canvas_draw_str(canvas, 2, 43, "Type : Waiting...");

    /* waiting box */
    canvas_draw_frame(canvas, 44, 49, 40, 10);
    canvas_draw_str_aligned(canvas, 64, 50, AlignCenter, AlignTop, "SCAN");

    bottom_hint(canvas, "Back=Menu");
}

/* ── BadUSB ────────────────────────────────────────────────────── */
void hnl_draw_badusb(Canvas* canvas) {
    canvas_clear(canvas);
    header_bar(canvas, "BADUSB");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 23, "1. whoami.txt");
    canvas_draw_str(canvas, 2, 33, "2. net_scan.txt");
    canvas_draw_str(canvas, 2, 43, "3. rickroll.txt");
    canvas_draw_str(canvas, 2, 53, "4. custom...");

    /* selection arrow next to first item */
    canvas_draw_str(canvas, 118, 23, "<");

    bottom_hint(canvas, "OK=Run  Back=Menu");
}

/* ── IR Blaster ────────────────────────────────────────────────── */
void hnl_draw_ir(Canvas* canvas) {
    canvas_clear(canvas);
    header_bar(canvas, "IR BLASTER");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 23, "Brand : Samsung");
    canvas_draw_str(canvas, 2, 33, "Device: TV");
    canvas_draw_str(canvas, 2, 43, "Freq  : 38 kHz");

    /* button row */
    uint8_t bw = 28, bh = 10, by = 50;
    canvas_draw_frame(canvas,  2, by, bw, bh);
    canvas_draw_str  (canvas,  5, by + bh - 2, "VOL+");
    canvas_draw_frame(canvas, 34, by, bw, bh);
    canvas_draw_str  (canvas, 37, by + bh - 2, "VOL-");
    canvas_draw_frame(canvas, 66, by, bw, bh);
    canvas_draw_str  (canvas, 69, by + bh - 2, "MUTE");
    canvas_draw_frame(canvas, 98, by, bw, bh);
    canvas_draw_str  (canvas,101, by + bh - 2, "PWR");
}

/* ── GPIO ──────────────────────────────────────────────────────── */
void hnl_draw_gpio(Canvas* canvas) {
    canvas_clear(canvas);
    header_bar(canvas, "GPIO");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 23, "PA0: OUTPUT [HIGH]");
    canvas_draw_str(canvas, 2, 33, "PA1: INPUT  [LOW]");
    canvas_draw_str(canvas, 2, 43, "PA4: UART-TX [ON]");
    canvas_draw_str(canvas, 2, 53, "PA5: UART-RX [ON]");

    bottom_hint(canvas, "Back=Menu");
}

/* ── BLE ───────────────────────────────────────────────────────── */
void hnl_draw_ble(Canvas* canvas) {
    canvas_clear(canvas);
    header_bar(canvas, "BLE SCANNER");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 23, "Realme Watch  -67dB");
    canvas_draw_str(canvas, 2, 33, "JBL Speaker   -72dB");
    canvas_draw_str(canvas, 2, 43, "Unknown BE:AC -81dB");
    canvas_draw_str(canvas, 2, 53, "Scan: ACTIVE");

    bottom_hint(canvas, "Back=Menu");
}
