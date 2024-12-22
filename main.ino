#include <Arduino.h>
#include "graphics.h"

void setup()
{
    delay(1500);
    Serial.begin(115200);
    Serial.printf("Serial initiated!\n");
}

void draw_box_rotation(int32_t width, float rad)
{
    clear_screen();
    draw_box_rotation(width, rad);
}

void loop()
{
    static int32_t width = 100;
    static float rad = 0;
    static int32_t ctr = 0;
    static float radRoll = 0;
    static int32_t scale = 12;

    draw_box_rotation(width, rad);
    graphics_update();

    ctr++;

    if (100 == ctr) {
        ctr = 0;
        radRoll++;

        if (scale * 2 == radRoll) {
        radRoll = 0;
        }

        rad = PI * radRoll / scale;
    }
}
