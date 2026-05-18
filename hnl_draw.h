#pragma once
#include <gui/canvas.h>
#include "hnl_main.h"

/* Boot & menu */
void hnl_draw_boot  (Canvas* canvas, const HNLState* s);
void hnl_draw_menu  (Canvas* canvas, const HNLState* s);

/* Tool screens */
void hnl_draw_subghz(Canvas* canvas);
void hnl_draw_nfc   (Canvas* canvas);
void hnl_draw_badusb(Canvas* canvas);
void hnl_draw_ir    (Canvas* canvas);
void hnl_draw_gpio  (Canvas* canvas);
void hnl_draw_ble   (Canvas* canvas);
