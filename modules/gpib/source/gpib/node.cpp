#include "gpib/node.hpp"

namespace GPIB {

void Node::setTransmitMode(TransmitModes mode) {
    switch (mode) {
        case TM_IDLE:
            // All handshake signals to INPUT_PULLUP
            io.resetBus(io.mask.hand); // FIXME: What exactly is handshake vs control in terms of pins.
            break;
        case TM_RECV:
            // Signal NRFD and NDAC, listen to DAV and EOI
            io.bus.setMode(io.bit.DAV | io.bit.EOI, InputPullup);
            io.bus.setMode(io.bit.NRFD | io.bit.NDAC, OutputHigh);
            break;
        case TM_SEND:
            // Signal DAV and EOI, listen to NRFD and NDAC
            io.bus.setMode(io.bit.NRFD | io.bit.NDAC, InputPullup);
            io.bus.setMode(io.bit.DAV | io.bit.EOI, OutputHigh);
            break;
        default:
            break;
    }
}

Node::HandshakeState Node::readByte(uint8_t *db, bool readWithEoi, bool *eoi) {
    uint32_t startMillis = Common::millis();
    uint32_t currentMillis = startMillis + 1UL;
    const uint32_t timeval = io.bus.config.rtmo;
    HandshakeState gpibState = HANDSHAKE_START;
    bool run = true;

    bool atnStat = io.isAsserted(io.bit.ATN); // Capture state of ATN
    *eoi = false;

    // Wait for interval to expire
    while (run && ((currentMillis - startMillis) < timeval)) {
        // NOTE: Device only check
        if (type == NType::Device) {
            // If IFC has been asserted then abort
            if (io.isAsserted(io.bit.IFC)) {
                gpibState = IFC_ASSERTED;
            }
            // ATN unasserted during handshake - not ready yet so abort (and exit ATN loop)
            if (atnStat && !io.isAsserted(io.bit.ATN)) {
                gpibState = ATN_ASSERTED;
            }
        }

        switch (gpibState) {
            case HANDSHAKE_START:
                // Unassert NRFD (we are ready for more data)
                io.clearSignal(io.bit.NRFD);
                gpibState = WAIT_FOR_DATA;
                break;
            case WAIT_FOR_DATA:
                // Wait for DAV to go LOW indicating talker has finished setting data lines..
                if (io.getGpibPinState(io.bit.DAV) == false) {
                    // Assert NRFD (Busy reading data)
                    io.assertSignal(io.bit.NRFD);
                    gpibState = READ_DATA;
                }
                break;
            case READ_DATA:
                // Check for EOI signal
                if (readWithEoi && io.isAsserted(io.bit.EOI)) {
                    *eoi = true;
                }
                // read from DIO
                *db = io.loadDataByte();
                // Unassert NDAC signalling data accepted
                io.clearSignal(io.bit.NDAC);
                gpibState = DATA_ACCEPTED;
                break;
            case DATA_ACCEPTED:
                // Wait for DAV to go HIGH indicating data no longer valid (i.e. transfer complete)
                if (io.getGpibPinState(io.bit.DAV) == true) {
                    // Re-assert NDAC - handshake complete, ready to accept data again
                    io.assertSignal(io.bit.NDAC);
                    gpibState = HANDSHAKE_COMPLETE;
                    break;
                }
                break;
            case ATN_ASSERTED:
            case IFC_ASSERTED:
                run = false;
                break;
            default:
                // TODO: Note default being called
                run = false;
                break;
        }

        // Increment time
        currentMillis = Common::millis();
    }

    return gpibState;
}

Node::HandshakeState Node::writeByte(uint8_t db, bool isLastByte) {
    uint32_t startMillis = Common::millis();
    uint32_t currentMillis = startMillis + 1UL;
    const uint32_t timeval = io.bus.config.rtmo;
    HandshakeState gpibState = HANDSHAKE_START;
    bool run = true;

    // Wait for interval to expire
    while (run && ((currentMillis - startMillis) < timeval)) {
        // NOTE: Device only check
        if (type == NType::Device) {
            // If IFC has been asserted then abort
            if (io.isAsserted(io.bit.IFC)) {
                gpibState = IFC_ASSERTED;
            }

            // If ATN has been asserted we need to abort and listen
            if (io.isAsserted(io.bit.ATN)) {
                gpibState = ATN_ASSERTED;
            }
        }

        switch (gpibState) {
            case HANDSHAKE_START: // Wait for NDAC to go LOW (indicating that devices (stage==4) || (stage==8) ) are at attention)
                if (io.getGpibPinState(io.bit.NDAC) == false) {
                    gpibState = WAIT_FOR_RECEIVER_READY;
                }
                break;
            case WAIT_FOR_RECEIVER_READY: // Wait for NRFD to go HIGH (indicating that receiver is ready)
                if (io.getGpibPinState(io.bit.NRFD) == true) {
                    gpibState = PLACE_DATA;
                }
                break;
            case PLACE_DATA: // Place data on the bus
                io.pushDataByte(db);
                if (io.bus.config.eoi && isLastByte) {
                    // If EOI enabled and this is the last byte then assert DAV and EOI
                    io.assertSignal(io.bit.DAV | io.bit.EOI);
                } else {
                    // Assert DAV (data is valid - ready to collect)
                    io.assertSignal(io.bit.DAV);
                }
                gpibState = DATA_READY;
                break;
            case DATA_READY: // Wait for NRFD to go LOW (receiver accepting data)
                if (io.getGpibPinState(io.bit.NRFD) == false) {
                    gpibState = RECEIVER_ACCEPTING;
                }
                break;
            case RECEIVER_ACCEPTING: // Wait for NDAC to go HIGH (data accepted)
                if (io.getGpibPinState(io.bit.NDAC) == true) {
                    gpibState = HANDSHAKE_COMPLETE;
                }
                break;
            case IFC_ASSERTED:
            case ATN_ASSERTED:
                setTransmitMode(TM_RECV);
                run = false;
                break;
            default:
                run = false;
                break;
        }

        // Increment time
        currentMillis = Common::millis();
    }

    // Handshake complete
    if (gpibState == HANDSHAKE_COMPLETE) {
        if (io.bus.config.eoi && isLastByte) {
            // If EOI enabled and this is the last byte then un-assert both DAV and EOI
            io.clearSignal(io.bit.DAV | io.bit.EOI);
        } else {
            // Unassert DAV
            io.clearSignal(io.bit.DAV);
        }
        // Reset the data bus
        io.bus.put(io.mask.dio, 0);
        io.resetBus(io.mask.dio);
    }

    return gpibState;
}

int Node::init(void) {
    for (auto &gpio : io.pinout.__raw_pinout) {
        gpio_set_function(gpio, GPIO_FUNC_SIO);
        gpio_set_pulls(gpio, true, false); // TODO: can pullups be left always on?
    }
    return 0;
}

} // namespace GPIB
