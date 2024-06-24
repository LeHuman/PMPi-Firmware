#pragma once

#include <common/delay.hpp>

#include "node.hpp"
#include "routines.hpp"
#include "types.hpp"

namespace GPIB {

class Controller : public Node {
private:
    void sendAllClear(void);

    void sendIFC(void);

    void setState(State state) override;

    int sendCmd(uint8_t cmdByte);

    /**
     * @brief
     * Untalk bus then address a device
     * talk: false=listen; true=talk;
     */
    int addressDevice(uint8_t addr, bool talk);

    int unAddressDevice();

public:
    // IMPROVE: Can we pass just the GPIBIO struct?
    consteval Controller(const PinOut &pinout, const Config &config) : Node(pinout, config, NType::Controller) {}
    // consteval Controller(const PinOut &pinout) : pinout(pinout), bit(getPinBits(pinout)), config(default_config), mask(getBitMasks(pinout)) {}

    void deinit(void);

    int init(void) override;
};

} // namespace GPIB