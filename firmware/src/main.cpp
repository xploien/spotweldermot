#include <stdint.h>
extern "C" {

#include "../ch32fun/ch32fun/ch32fun.h"
#include "st7789.h"
}

#include "input.h"

uint16_t colors[] = {
    BLACK,     NAVY,     DARKGREEN, DARKCYAN,    MAROON, PURPLE, OLIVE,
    LIGHTGREY, DARKGREY, BLUE,      GREEN,       CYAN,   RED,    MAGENTA,
    YELLOW,    WHITE,    ORANGE,    GREENYELLOW, PINK,
};

uint8_t rand8(void);

inputdevices::selectmenus inputdevices::current_menu = inputdevices::MENU_TIME;

int main(void) {

  SystemInit();
  inputdevices::gpio_init();
  Delay_Ms(100);

  tft_init();
  tft_fill_rect(0, 0, 240, 240, BLACK);
  tft_draw_pixel(0, 0, BLUE);
  tft_draw_pixel(0, 239, BLUE);
  tft_draw_pixel(239, 0, BLUE);
  tft_draw_pixel(239, 239, BLUE);

  while (1) {

    inputdevices::isencoderpushed();

    int contador = inputdevices::encoder();
    // tft_draw_thick_circle(119, 119, 119, 109, colors[rand8() % 19]);
    // tft_draw_thick_circle(119, 119, 100, 90, colors[rand8() % 19]);
    // tft_draw_thick_circle(119, 119, 100 - 19, 90 - 19, colors[rand8() % 19]);
    tft_set_color(BLUE);
    tft_set_background_color(BLACK);
    tft_set_cursor(119, 119);
    tft_print_number(contador, 0);

    tft_set_cursor(119, 180);
    tft_print_number(inputdevices::current_menu, 0);
  }
}
