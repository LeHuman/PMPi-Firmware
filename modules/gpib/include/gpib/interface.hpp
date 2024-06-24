#pragma once

#include "bus.hpp"
#include "types.hpp"

namespace GPIB {

class GPIBIOInterface {
public:
    const PinOut pinout;
    const GPIBMask bit;
    const PinOut::Mask mask;

    GPIBBus bus;

public:
    consteval GPIBIOInterface(const PinOut &pinout, const Config &config) : pinout(pinout), bit(pinout.get_pin_mask()), mask(pinout.get_bit_mask()), bus(config) {}

    uint8_t loadDataByte(void);

    void pushDataByte(uint8_t byte);

    void resetBus(GPIBMask_t mask);

    void assertSignal(GPIBMask_t mask);

    void clearSignal(GPIBMask_t mask);

    void setSignal(GPIBMask_t mask, uint32_t value);

    void readyGpibDbus(void);

    uint8_t readGpibDbus(void);

    bool getGpibPinState(GPIBMask_t bitMask) const;

    /***** Detect selected pin state *****/
    bool isAsserted(GPIBMask_t gpibsig) const;
};
} // namespace GPIB