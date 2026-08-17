#include <stdint.h>

static inline uint16_t isqrt32(uint32_t n) {
  uint32_t res = 0;
  uint32_t bit =
      1u << 30; // El bit más alto posible para un cuadrado de 32 bits

  // "bit" empieza en la potencia de 4 más alta <= n
  while (bit > n)
    bit >>= 2;

  while (bit != 0) {
    if (n >= res + bit) {
      n -= res + bit;
      res = (res >> 1) + bit;
    } else {
      res >>= 1;
    }
    bit >>= 2;
  }
  return (uint16_t)res;
}

/* White Noise Generator State */
#define NOISE_BITS 8
#define NOISE_MASK ((1 << NOISE_BITS) - 1)
#define NOISE_POLY_TAP0 31
#define NOISE_POLY_TAP1 21
#define NOISE_POLY_TAP2 1
#define NOISE_POLY_TAP3 0
static uint32_t lfsr = 1;

/*
 * random byte generator
 */
static inline uint8_t rand8(void) {
  uint8_t bit;
  uint32_t new_data;

  for (bit = 0; bit < NOISE_BITS; bit++) {
    new_data = ((lfsr >> NOISE_POLY_TAP0) ^ (lfsr >> NOISE_POLY_TAP1) ^
                (lfsr >> NOISE_POLY_TAP2) ^ (lfsr >> NOISE_POLY_TAP3));
    lfsr = (lfsr << 1) | (new_data & 1);
  }

  return lfsr & NOISE_MASK;
}
