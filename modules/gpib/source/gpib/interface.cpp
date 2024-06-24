#include "gpib/interface.hpp"

namespace GPIB {

uint8_t GPIBIOInterface::loadDataByte(void) {
    uint32_t bus_val = bus.get(mask.dio);
    uint8_t byte = 0;

    byte |= bit.DIO1 & bus_val;
    byte |= bit.DIO2 & bus_val;
    byte |= bit.DIO3 & bus_val;
    byte |= bit.DIO4 & bus_val;
    byte |= bit.DIO5 & bus_val;
    byte |= bit.DIO6 & bus_val;
    byte |= bit.DIO7 & bus_val;
    byte |= bit.DIO8 & bus_val;

    return byte;
}

void GPIBIOInterface::pushDataByte(uint8_t byte) {
    uint32_t bus_out = 0;

    bus_out |= bit.DIO1 * uint32_t(bool(byte & (1U << 0U)));
    bus_out |= bit.DIO2 * uint32_t(bool(byte & (1U << 1U)));
    bus_out |= bit.DIO3 * uint32_t(bool(byte & (1U << 2U)));
    bus_out |= bit.DIO4 * uint32_t(bool(byte & (1U << 3U)));
    bus_out |= bit.DIO5 * uint32_t(bool(byte & (1U << 4U)));
    bus_out |= bit.DIO6 * uint32_t(bool(byte & (1U << 5U)));
    bus_out |= bit.DIO7 * uint32_t(bool(byte & (1U << 6U)));
    bus_out |= bit.DIO8 * uint32_t(bool(byte & (1U << 7U)));

    bus.put(mask.dio, bus_out);
}

void GPIBIOInterface::resetBus(GPIBMask_t mask) {
    bus.setMode(mask, InputPullup); // Set all control signals to INPUT_PULLUP
}

void GPIBIOInterface::assertSignal(GPIBMask_t mask) {
    bus.setMode(mask, OutputLow); // Set all signals permitted by mask to LOW (asserted)
}

void GPIBIOInterface::clearSignal(GPIBMask_t mask) {
    bus.setMode(mask, OutputHigh); // Set all signals permitted by mask to HIGH (unasserted)
}

void GPIBIOInterface::setSignal(GPIBMask_t mask, uint32_t value) {
    gpio_set_dir_masked(mask, value);
    gpio_put_masked(mask, value);
}

void GPIBIOInterface::readyGpibDbus(void) {
    resetBus(mask.dio);
}

uint8_t GPIBIOInterface::readGpibDbus(void) {
    return loadDataByte();
}

bool GPIBIOInterface::getGpibPinState(GPIBMask_t bitMask) const {
    return bus.get(bitMask);
}

/***** Detect selected pin state *****/
bool GPIBIOInterface::isAsserted(GPIBMask_t gpibsig) const {
    return getGpibPinState(gpibsig) == false;
}

} // namespace GPIB
