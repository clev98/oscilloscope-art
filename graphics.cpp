#include <arduino.h>
#include "graphics.h"
#include "soc/rtc_io_reg.h"
#include "soc/sens_reg.h"

// Macros
#define MAX_LINES       1024
#define X_PIN           25
#define Y_PIN           26
#define X_RESOLUTION    255
#define Y_RESOLUTION    255

typedef struct
{
    double x1;
    double x2;
} vec2d_t;

typedef struct
{
    vec2d_t line_start;
    vec2d_t line_end;
    int32_t length;
} line_t;

// Function foward decalarations
line_t new_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
double vec2d_length(vec2d_t vec);
double vec2d_distance_between(vec2d_t vec1, vec2d_t vec2);
vec2d_t vec2d_normalise(vec2d_t vec);
vec2d_t vec2d_subtract(vec2d_t vec1, vec2d_t vec2);
vec2d_t vec2d_add(vec2d_t vec1, vec2d_t vec2);
void write_x_pixel(int32_t pos);
void write_y_pixel(int32_t pos);

void write_dac1(uint8_t value);
void write_dac2(uint8_t value);

line_t lines[MAX_LINES];
int32_t num_lines = 0;

/**
 * @brief Draws a line between two given points.
 * 
 * This function draws a line between the starting point (x1, y1) and the ending point (x2, y2).
 * 
 * @param x1 Starting x-position.
 * @param y1 Starting y-position.
 * @param x2 Ending x-position.
 * @param y2 Ending y-position.
 * 
 * @return int32_t 0 if drawing the line fails (due to reaching the maximum number of lines), 1 on success.
 */
int32_t draw_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    if (num_lines >= MAX_LINES)
    {
        return 0;
    }

    lines[num_lines] = new_line(x1, y1, x2, y2);
    num_lines++;

    return 1;
}


line_t new_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    // Define the start of the line
    vec2d_t line_start = 
    {
        .x1 = x1,
        .x2 = y1
    };
    
    // Defie the end of the line
    vec2d_t line_end = 
    {
        .x1 = x2,
        .x2 = y2        
    };
    
    // Make a line object
    line_t line = 
    {
        .line_start = line_start,
        .line_end   = line_end
    };

    // Set the length of the line
    double line_length = vec2d_distance_between(line.line_start, line.line_end);
    line.length = line_length;

    return line;
}

/**
 * @brief Clears the screen by resetting the number of lines.
 * 
 * This function clears the screen by resetting the count of lines to zero.
 */
void clear_screen(void)
{
    num_lines = 0;
}

void graphics_update(void)
{

    for(int32_t line_index=0; line_index<num_lines; line_index++)
    {
        // Select the next line to draw
        line_t line_to_draw = lines[line_index];

        // Incrementally move the laser from the start to the end of the line                                       // ~7 us
        vec2d_t draw_direction = vec2d_normalise(vec2d_subtract(line_to_draw.line_end, line_to_draw.line_start));

        vec2d_t laser_position = line_to_draw.line_start;

        // Keeps advancing the position of the laser until it has drawn the fulll length of the line                // ~6 us (condition time)
        while(vec2d_distance_between(laser_position, line_to_draw.line_start) < line_to_draw.length)
        {
            double increment_length = 1;

            // Write laser position to the screen                                                                   // ~40 us
            write_x_pixel(laser_position.x1);
            write_y_pixel(laser_position.x2);

            // Advance the laser to the next position                                                               // ~1 us
            laser_position.x1 += increment_length*draw_direction.x1;
            laser_position.x2 += increment_length*draw_direction.x2;

        }

    }
}

/* ---------------- Geometry Functions ---------------- */
void draw_center_rectangle(int32_t width, int32_t height, int32_t xpos, int32_t ypos)
{
    int32_t start_xpos  = xpos - width / 2;
    int32_t start_ypos = ypos - height / 2;

    draw_line(start_xpos, start_ypos, start_xpos + width, start_ypos);
    draw_line(start_xpos + width, start_ypos, start_xpos + width, start_ypos + height);
    draw_line(start_xpos + width, start_ypos + height, start_xpos, start_ypos + height);
    draw_line(start_xpos, start_ypos + height, start_xpos, start_ypos);
}

void draw_sqaure_rotation(int32_t width, float rad)
{
  /*
  vec2d_t vector = 
    {
        .x1 = sin(rad) * width,
        .x2 = sin(2*rad) * width
    };
  */
  vec2d_t vector1 = 
    {
        .x1 = width * cos(rad),
        .x2 = width * sin(rad)
    };
  vec2d_t vector2 = 
    {
        .x1 = width * cos(rad + PI / 2),
        .x2 = width * sin(rad + PI / 2)
    };
  vec2d_t vector3 = 
    {
        .x1 = width * cos(rad + PI),
        .x2 = width * sin(rad + PI)
    };
  vec2d_t vector4 = 
    {
        .x1 = width * cos(rad + 3 * PI / 2),
        .x2 = width * sin(rad + 3 * PI / 2)
    };

  //if (0 == vector.x1) {
  //  vector.x1 = width;
  //}

  // Serial.printf("X %lg Y %lf RAD %f \n", vector.x1, vector.x2, rad);

  draw_line(vector1.x1 + width, vector1.x2 + width, vector2.x1 + width, vector2.x2 + width);
  draw_line(vector2.x1 + width, vector2.x2 + width, vector3.x1 + width, vector3.x2 + width);
  draw_line(vector3.x1 + width, vector3.x2 + width, vector4.x1 + width, vector4.x2 + width);
  draw_line(vector4.x1 + width, vector4.x2 + width, vector1.x1 + width, vector1.x2 + width);

  // draw_line(vector.x1 + width, vector.x2 + width, -vector.x2 + width, vector.x1 + width);
  // draw_line(-vector.x2 + width, vector.x1 + width, -vector.x1 + width, -vector.x2 + width);
  // draw_line(-vector.x1 + width, -vector.x2 + width, vector.x2 + width, -vector.x1 + width);
  // draw_line(vector.x2 + width, -vector.x1 + width, vector.x1 + width, vector.x2 + width);
}

/* ---------------- Vector Functions ---------------- */
double vec2d_length(vec2d_t vec)
{
    return sqrt(vec.x1*vec.x1 + vec.x2*vec.x2);
}

double vec2d_distance_between(vec2d_t vec1, vec2d_t vec2)
{
    vec2d_t vector = 
    {
        .x1 = vec1.x1 - vec2.x1,
        .x2 = vec1.x2 - vec2.x2
    };
    return vec2d_length(vector);
}

vec2d_t vec2d_normalise(vec2d_t vec)
{
    // Get the length of the vector
    double vector_length = vec2d_length(vec);

    // Normalise the passed vector by its length
    vec2d_t vector = 
    {
        .x1 = vec.x1 / vector_length,
        .x2 = vec.x2 / vector_length
    };

    // Return the unit vector
    return vector;
}

/**
 * @brief 
 * 
 * @param vec1 
 * @param vec2 
 * @return vec2d_t vec1 - vec2
 */
vec2d_t vec2d_subtract(vec2d_t vec1, vec2d_t vec2)
{
    vec2d_t vector = 
    {
        vec1.x1 - vec2.x1,
        vec1.x2 - vec2.x2
    };

    return vector;
}

vec2d_t vec2d_add(vec2d_t vec1, vec2d_t vec2)
{
    vec2d_t vector = 
    {
        vec1.x1 + vec2.x1,
        vec1.x2 + vec2.x2
    };

    return vector;
}

/* ---------------- Hardware Interface ---------------- */
void write_x_pixel(int32_t pos)
{
    if(pos < 0) return;
    if(pos > X_RESOLUTION) return;
    write_dac1((uint8_t)pos);
}

void write_y_pixel(int32_t pos)
{
    if(pos < 0) return;
    if(pos > Y_RESOLUTION) return;
    write_dac2((uint8_t)pos);
}

void write_dac1(uint8_t value)
{
    // disable CW generator on channel 1
    CLEAR_PERI_REG_MASK(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_CW_EN1_M);
    // enable DAC channel output
    SET_PERI_REG_MASK(RTC_IO_PAD_DAC1_REG, RTCIO_PAD_PDAC1_MUX_SEL | RTC_IO_PDAC1_XPD_DAC | RTC_IO_PDAC1_DAC_XPD_FORCE);
    // setting DAC output value
    SET_PERI_REG_BITS(RTC_IO_PAD_DAC1_REG, RTC_IO_PDAC1_DAC, value, RTC_IO_PDAC1_DAC_S);
}

void write_dac2(uint8_t value)
{
    // disable CW generator on channel 2
    CLEAR_PERI_REG_MASK(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_CW_EN2_M);
    // enable DAC channel output
    SET_PERI_REG_MASK(RTC_IO_PAD_DAC2_REG, RTCIO_PAD_PDAC2_MUX_SEL | RTC_IO_PDAC2_XPD_DAC | RTC_IO_PDAC2_DAC_XPD_FORCE);
    // setting DAC output value
    SET_PERI_REG_BITS(RTC_IO_PAD_DAC2_REG, RTC_IO_PDAC2_DAC, value, RTC_IO_PDAC2_DAC_S);
}
