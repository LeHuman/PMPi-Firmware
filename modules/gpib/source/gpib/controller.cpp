#include "gpib/controller.hpp"

namespace GPIB {
void Controller::deinit(void) {
    state = State::INVALID;
    // Set control and data bus to idle state (all lines input_pULlup)
    io.resetBus(io.mask.all);
}

int Controller::init(void) {
    int rcode = Node::init();
    if (rcode != 0) {
        return rcode;
    }
    // Send request to clear all devices on bus to local mode
    sendAllClear();
    // Stop current mode
    deinit();
    Common::delay_ms(200); // Allow settling time
    // Set GPIB control bus to controller idle mode
    setState(INIT);
    // Initialise GPIB data lines (sets to INPUT_PULLUP)
    io.readyGpibDbus();
    // Assert IFC to signal controller in charge (CIC)
    sendIFC();
    // Attempt to address device to listen
    if (io.bus.config.paddr > 1U) {
        addressDevice(io.bus.config.paddr, 0);
    }

    return 0;
}

void Controller::sendAllClear(void) {
    // Un-assert REN
    io.clearSignal(io.bit.REN);
    Common::delay_ms(40);
    // Simultaneously assert ATN and REN
    io.assertSignal(io.bit.ATN | io.bit.REN);
    Common::delay_ms(40);
    // Unassert ATN
    io.clearSignal(io.bit.ATN);
}

void Controller::sendIFC(void) {
    // Assert IFC
    io.assertSignal(io.bit.IFC);
    Common::delay_ms(150);
    // De-assert IFC
    io.clearSignal(io.bit.IFC);
}

void Controller::setState(State state) {
    switch (state) {
        case INIT: // Initialisation
            // Signal IFC, REN and ATN, listen to SRQ
            io.bus.setMode(io.bit.SRQ, InputPullup);
            io.bus.setMode(io.bit.IFC | io.bit.REN | io.bit.ATN, OutputHigh);
            setTransmitMode(TM_IDLE);
            io.assertSignal(io.bit.REN);
            break;
        case IDLE: // Controller idle state
            if (unAddressDevice()) {
                // TODO: Failed to address device to talk
            }

            setTransmitMode(TM_IDLE);
            io.clearSignal(io.bit.ATN);
            break;
        case COMMAND: // Controller active - send commands
            setTransmitMode(TM_SEND);
            io.assertSignal(io.bit.ATN);
            break;
        case LISTEN: // Controller - read data bus
                     // Address device to talk
            if (addressDevice(io.bus.config.paddr, true)) {
                // TODO: Failed to address device to talk
            }
            // Set state for receiving data
            setTransmitMode(TM_RECV);
            io.clearSignal(io.bit.ATN);
            break;
        case TALK: // Controller - write data bus
            setTransmitMode(TM_SEND);
            io.clearSignal(io.bit.ATN);
            break;
        default:
            this->state = State::INVALID;
            return;
    }
    this->state = state;
}

int Controller::sendCmd(uint8_t cmdByte) {
    // Set lines for command and assert ATN
    if (this->state != COMMAND) {
        setState(COMMAND);
    }

    // Send the command
    if (writeByte(cmdByte, false) == HANDSHAKE_COMPLETE) {
        return 0;
    }

    return -1;
}

int Controller::addressDevice(uint8_t addr, bool talk) {
    if (sendCmd(GC_UNL) < 0) {
        return -1;
    }
    if (talk) {
        // Device to talk, controller to listen
        if (sendCmd(GC_TAD + addr) < 0) {
            return -1;
        }
    } else {
        // Device to listen, controller to talk
        if (sendCmd(GC_LAD + addr) < 0) {
            return -1;
        }
    }
    deviceAddressed = true;
    return 0;
}

int Controller::unAddressDevice() {
    if (deviceAddressed == false) {
        return 1;
    }
    // De-bounce
    Common::delay_us(30);
    // Utalk/unlisten
    if (sendCmd(GC_UNL) < 0) {
        return -1;
    }
    if (sendCmd(GC_UNT) < 0) {
        return -1;
    }
    deviceAddressed = false;
    return 0;
}

} // namespace GPIB