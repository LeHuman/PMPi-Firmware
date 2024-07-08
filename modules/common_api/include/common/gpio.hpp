#pragma once
#include <hardware/gpio.h>
#include <stdint.h>

#include "macro.hpp"

#define gpio_to_mask(GPIO) (1UL << (GPIO))
#define gpio_to_mask_chain(GPIO) gpio_to_mask(GPIO) |
#define gpio_set_multi(GPIOs...) gpio_set_mask(FOR_EACH(gpio_to_mask_chain, GPIOs) 0UL)
#define gpio_clr_multi(GPIOs...) gpio_clr_mask(FOR_EACH(gpio_to_mask_chain, GPIOs) 0UL)
#define gpio_xor_multi(GPIOs...) gpio_xor_mask(FOR_EACH(gpio_to_mask_chain, GPIOs) 0UL)
#define gpio_put_multi(GPIOs...) gpio_put_masked(FOR_EACH(gpio_to_mask_chain, GPIOs) 0UL, value)
