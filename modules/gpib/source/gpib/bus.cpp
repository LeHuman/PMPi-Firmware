#include "gpib/bus.hpp"

namespace GPIB {

void GPIBBus::setMode(GPIBMask_t mask, BusMode state) {
    // NOTE: mask should be bits 0-29, not internally verified
    switch (state) {
        case InputPullup:
            gpio_set_dir_in_masked(mask);
            // gpio_set_pulls(gpio, true, false);
            break;
        case OutputHigh:
            gpio_set_dir_out_masked(mask);
            gpio_set_mask(mask);
            // gpio_set_pulls(gpio, false, false);
            break;
        case OutputLow:
            gpio_set_dir_out_masked(mask);
            gpio_clr_mask(mask);
            // gpio_set_pulls(gpio, false, false);
            break;
        default:
            // TODO: Warn about nothing happening in switches
            break;
    }
}

} // namespace GPIB