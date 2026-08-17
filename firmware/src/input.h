#pragma once

extern "C" {
#include "../ch32fun/ch32fun/ch32fun.h"
#include "st7789.h"
}

class inputdevices {
public:
// PC0 — Encoder A
#define ENC_A_PORT GPIOC
#define ENC_A_PIN 0

// PC1 — Encoder B  (antes PD1 = SWIO/debug — NO tocar)
#define ENC_B_PORT GPIOC
#define ENC_B_PIN 1

// PD2 — Encoder push
#define ENC_PUSH_PORT GPIOD
#define ENC_PUSH_PIN 2

// PD3 — Botón OK
#define BTN_OK_PORT GPIOD
#define BTN_OK_PIN 3

#define READ_PIN(port, pin) (((port)->INDR >> (pin)) & 1u)

  typedef enum {
    MENU_TIME = 0,
    MENU_MAGNITUDE = 1,
    MENU_PERCENTAGE = 2,
    MAX_STATES
  } selectmenus;

  static selectmenus current_menu;

  static inline void gpio_init(void) {
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD;

    // PC0 — Enc A — input pull-up
    GPIOC->CFGLR =
        (GPIOC->CFGLR & ~(0xFu << (ENC_A_PIN * 4))) | (0x8u << (ENC_A_PIN * 4));
    GPIOC->BSHR = (1u << ENC_A_PIN);

    // PC1 — Enc B — input pull-up
    GPIOC->CFGLR =
        (GPIOC->CFGLR & ~(0xFu << (ENC_B_PIN * 4))) | (0x8u << (ENC_B_PIN * 4));
    GPIOC->BSHR = (1u << ENC_B_PIN);

    // PD2 — Enc Push — input pull-up
    GPIOD->CFGLR = (GPIOD->CFGLR & ~(0xFu << (ENC_PUSH_PIN * 4))) |
                   (0x8u << (ENC_PUSH_PIN * 4));
    GPIOD->BSHR = (1u << ENC_PUSH_PIN);

    // PD3 — Btn OK — input flotante (debounce HW externo)
    GPIOD->CFGLR = (GPIOD->CFGLR & ~(0xFu << (BTN_OK_PIN * 4))) |
                   (0x4u << (BTN_OK_PIN * 4));
  }

  static inline void isencoderpushed(void) {
    static uint32_t last_ms = 0;
    if (!(READ_PIN(ENC_PUSH_PORT, ENC_PUSH_PIN))) {
      uint32_t now = SysTick->CNT / (FUNCONF_SYSTEM_CORE_CLOCK / 1000);
      if ((now - last_ms) > 200) {
        last_ms = now;
        current_menu = (selectmenus)((current_menu + 1) % MAX_STATES);
      }
    }
  }

  static inline int encoder(void) {
    static uint8_t prev_A = 1;
    static int contador = 0;

    uint8_t A = READ_PIN(ENC_A_PORT, ENC_A_PIN);
    uint8_t B = READ_PIN(ENC_B_PORT, ENC_B_PIN);

    if (prev_A == 1 && A == 0) {
      if (B == 1) {
        contador++;
        tft_draw_thick_circle(119, 119, 30, 15, GREENYELLOW);
      } else {
        contador--;
        tft_draw_thick_circle(119, 119, 30, 15, RED);
      }
      Delay_Ms(5);
    }

    prev_A = A;
    return contador;
  }
};
