#include "gpib/device.hpp"

namespace GPIB {
void Device::setState(State state) {
    // switch (state) {
    //     // Listener states
    //     case INIT: // Listener initialisation
    //         resetBus(mask.ctrl | mask.hand);
    //         // Signal SRQ, listen to IFC, REN and ATN
    //         clearSignal(bit.REN);
    //         setBusMode(bit.IFC | bit.REN | bit.ATN, InputPullup);
    //         setBusMode(bit.SRQ, OutputHigh);
    //         // Set data bus to idle state
    //         readyGpibDbus();
    //         break;
    //     case IDLE: // Device idle state
    //         setTransmitMode(TM_IDLE);
    //         // Set data bus to idle state
    //         readyGpibDbus();
    //         break;
    //     case LISTEN: // Device listner active (actively listening - can handshake)
    //         setTransmitMode(TM_RECV);
    //         break;
    //     case TALK: // Device talker active (sending data)
    //         setTransmitMode(TM_SEND);
    //         break;
    //     default:
    //         state = State::INVALID;
    //         return;
    // }
    // state = state;
}

int Device::init(void) {
    // int rcode = Node::init();
    // if (rcode != 0) {
    //     return rcode;
    // }
    // // Send request to clear all devices on bus to local mode
    // sendAllClear();
    // // Stop current mode
    // deinit();
    // Common::delay_ms(200); // Allow settling time
    // // Set GPIB control bus to controller idle mode
    // setState(INIT);
    // // Initialise GPIB data lines (sets to INPUT_PULLUP)
    // io.readyGpibDbus();
    // // Assert IFC to signal controller in charge (CIC)
    // sendIFC();
    // // Attempt to address device to listen
    // if (io.bus.config.paddr > 1U) {
    //     addressDevice(io.bus.config.paddr, 0);
    // }

    // return 0;
    return -1;
}

// void Device::startDeviceMode(void) {
//     // Stop current mode
//     deinit();
//     Common::delay_ms(200); // Allow settling time
//     // Start device mode
//     status.cmode = Config::NType::Device;
//     // Set GPIB control bus to device idle mode
//     setState(DEV_INIT);
//     // Initialise GPIB data lines (sets to INPUT_PULLUP)
//     readyGpibDbus();
// }
} // namespace GPIB