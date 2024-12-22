#include <Arduino.h>
#include "graphics.h"

#define NUM_KEYS    4

char keys_pressed[NUM_KEYS];

typedef enum
{
    w = 0,
    a,
    s,
    d
} keys_t;

void update_pressed_keys(void)
{
    if (Serial.available() > 0)
    {
        char received_char = Serial.read();
        if (received_char == 'W')   keys_pressed[w] = true;
        if (received_char == 'A')   keys_pressed[a] = true;
        if (received_char == 'S')   keys_pressed[s] = true;
        if (received_char == 'D')   keys_pressed[d] = true;

        if (received_char == 'w')   keys_pressed[w] = false;
        if (received_char == 'a')   keys_pressed[a] = false;
        if (received_char == 's')   keys_pressed[s] = false;
        if (received_char == 'd')   keys_pressed[d] = false;
    }
}

void draw_number(int32_t number, int32_t x, int32_t y)
{
    // Define the line segments for each digit (0-9)
    // Each line is represented by a tuple {x1, y1, x2, y2}
    int digit_segments[10][5][4] =
    {
        /* 0 */ {{0, 20, 10, 20}, {10, 20, 10, 0}, {10, 0, 0, 0}, {0, 0, 0, 20}},
        /* 1 */ {{10, 20, 10, 0}},
        /* 2 */ {{0, 20, 10, 20}, {10, 20, 10, 10}, {10, 10, 0, 0}, {0, 0, 10, 0}},
        /* 3 */ {{0, 20, 10, 20}, {10, 20, 10, 10}, {10, 10, 0, 10}, {10, 10, 10, 0}, {10, 0, 0, 0}},
        /* 4 */ {{0, 20, 0, 10}, {10, 10, 0, 10}, {10, 20, 10, 0}},
        /* 5 */ {{10, 20, 0, 20}, {0, 20, 0, 10}, {0, 10, 10, 10}, {10, 10, 10, 0}, {10, 0, 0, 0}},
        /* 6 */ {{10, 20, 0, 20}, {0, 20, 0, 0}, {0, 0, 10, 0}, {10, 0, 10, 10}, {10, 10, 0, 10}},
        /* 7 */ {{0, 20, 10, 20}, {10, 20, 10, 0}},
        /* 8 */ {{0, 20, 10, 20}, {10, 20, 10, 0}, {10, 0, 0, 0}, {0, 0, 0, 20}, {0, 10, 10, 10}},
        /* 9 */ {{10, 10, 0, 10}, {0, 10, 0, 20}, {0, 20, 10, 20}, {10, 20, 10, 0}}
    };

    // If the number is out of range, draw a zero 
    if (number < 0 || number > 9) number = 0;

    for (int32_t i = 0; i < sizeof(digit_segments[number]) / sizeof(digit_segments[number][0]); i++)
    {
        int32_t x1 = digit_segments[number][i][0] + x;
        int32_t y1 = digit_segments[number][i][1] + y;
        int32_t x2 = digit_segments[number][i][2] + x;
        int32_t y2 = digit_segments[number][i][3] + y;
        draw_line(x1, y1, x2, y2);
    }
}

void setup()
{
    delay(1500);
    Serial.begin(115200);
    Serial.printf("\n\nSerial initiated!!\n\n");
}

void draw_box_rotation(int32_t width, float rad)
{
  clear_screen();
  draw_sqaure_rotation(width, rad);
}

void draw_box(void)
{
    static int32_t count = 0;
    static double x_pos = 127;
    static double y_pos = 127;
    
    static double x_vel = 2000 + random(10000);
    static double y_vel = 2000 + random(10000);

    uint32_t draw_update_period = 10;

    static uint64_t last_draw_time = 0;
    uint64_t dt = millis()-last_draw_time;

    if(dt > draw_update_period)
    {
        // Reset timer
        last_draw_time = millis();

        double action_force     = 1200.0;     // [force units]
        double gravity_force    = 900;
        double damping_factor   = 0.8;
        
        x_pos += x_vel*dt / 1000.0;
        y_pos += y_vel*dt / 1000.0;
        
        y_vel -= gravity_force*dt / 1000.0;

        if (x_pos < 0 || x_pos > 255) x_vel *= -1*damping_factor;

        if (y_pos < 0 || y_pos > 255)
        {
            x_vel *= damping_factor;
            y_vel *= -1*damping_factor;
        }

        if(keys_pressed[d]) x_vel += action_force*dt / 1000.0;
        if(keys_pressed[a]) x_vel -= action_force*dt / 1000.0;
        if(keys_pressed[w]) y_vel += action_force*dt / 1000.0;
        if(keys_pressed[s]) y_vel -= action_force*dt / 1000.0;

        // Prevents particle from being outside the domain (1000x1000 mm grid)
        x_pos = constrain(x_pos, 0, 255);
        y_pos = constrain(y_pos, 0, 255);
        
        clear_screen();
        draw_center_rectangle(20,20,x_pos,y_pos);
        
        int32_t number = y_pos;

        int32_t ones        = number % 10;
        int32_t tens        = (number/10) % 10;
        int32_t hundreds    = (number / 100) % 10;
        int32_t thousands   = (number / 1000) % 10;

        // draw_number(thousands,  5+25*0, 230);
        draw_number(hundreds,   5+22*0, 230);
        draw_number(tens,       5+22*1, 230);
        draw_number(ones,       5+22*2, 230);
    }
}

void loop()
{
    static int32_t width = 100;
    static float rad = 0;
    static int32_t ctr = 0;
    static float radRoll = 0;
    static int32_t scale = 12;

    // update_pressed_keys();
    // draw_box();
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
