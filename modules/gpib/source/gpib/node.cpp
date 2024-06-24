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

bool isTerminatorDetected(const uint8_t bytes[3], const uint8_t &eorSequence) {
    // Look for specified terminator (CR+LF by default)
    switch (eorSequence) {
        case 0:
            // CR+LF terminator
            if (bytes[0] == LF && bytes[1] == CR) {
                return true;
            }
            break;
        case 1:
            // CR only as terminator
            if (bytes[0] == CR) {
                return true;
            }
            break;
        case 2:
            // LF only as terminator
            if (bytes[0] == LF) {
                return true;
            }
            break;
        case 3:
            // No terminator (will rely on timeout)
            break;
        case 4:
            // Keithley can use LF+CR instead of CR+LF
            if (bytes[0] == CR && bytes[1] == LF) {
                return true;
            }
            break;
        case 5:
            // Solarton (possibly others) can also use ETX (0x03)
            if (bytes[0] == 0x03) {
                return true;
            }
            break;
        case 6:
            // Solarton (possibly others) can also use CR+LF+ETX (0x03)
            if (bytes[0] == 0x03 && bytes[1] == LF && bytes[2] == CR) {
                return true;
            }
            break;
        default:
            // Use CR+LF terminator by default
            if (bytes[0] == LF && bytes[1] == CR) {
                return true;
            }
            break;
    }
    return false;
}

/***** Receive data from the GPIB bus ****/
/*
 * Readbreak:
 * 7 - command received via serial
 */
bool Node::receiveData(std::streambuf &dataStream, bool detectEoi, bool detectEndByte, uint8_t endByte) {

    uint8_t bytes[3] = {0};              // Received byte buffer
    uint8_t eor = io.bus.config.eor & 7; // TODO: What is this
    int x = 0;
    bool readWithEoi = false;
    bool eoiDetected = false;
    HandshakeState state = HANDSHAKE_COMPLETE;

    // endByte = endByte; // meaningless but defeats vcompiler warning!

    // Reset transmission break flag
    // txBreak = false;

    // EOI detection required ?
    if (io.bus.config.eoi || detectEoi || (io.bus.config.eor == Config::EOR::SPACE)) {
        readWithEoi = true; // Use EOI as terminator
    }

    // Set up for reading in Controller mode
    if (type == NType::Controller) { // Controler mode

        // // Address device to talk
        // if (addressDevice(cfg.paddr, 1)) {
        //     // TODO: Failed to address device to talk
        // }

        // Wait for instrument ready
        // Set GPIB control lines to controller read mode
        setState(CTRL_LISN);

        // Set up for reading in Device mode
    } else {                // Device mode
                            // Set GPIB controls to device read mode
        setState(DEV_LISN);
        readWithEoi = true; // In device mode we read with EOI by default
    }

    // Ready the data bus
    io.readyGpibDbus();

    // Perform read of data (r=0: data read OK; r>0: GPIB read error);
    while (state == HANDSHAKE_COMPLETE) {

        // txBreak > 0 indicates break condition
        // if (txBreak)
        //     break;

        // ATN asserted
        if (io.isAsserted(io.bit.ATN)) {
            break;
        }

        // Read the next character on the GPIB bus
        state = readByte(&bytes[0], readWithEoi, &eoiDetected);

        // If IFC or ATN asserted then break here
        if ((state == IFC_ASSERTED) || (state == ATN_ASSERTED)) {
            break;
        }

        if (state != HANDSHAKE_COMPLETE) {
            // Stop (error or timeout)
            break;
        }

        // If successfully received character
        // Output the character to the serial port
        dataStream.sputc((char)bytes[0]);

        // Byte counter
        x++;

        // EOI detection enabled and EOI detected?
        if (readWithEoi) {
            if (eoiDetected) {
                break;
            }
        } else {
            // Has a termination sequence been found ?
            if (detectEndByte) {
                if (bytes[0] == endByte) {
                    break;
                }
            } else {
                if (isTerminatorDetected(bytes, eor)) {
                    break;
                }
            }
        }

        // Shift last three bytes in memory
        bytes[2] = bytes[1];
        bytes[1] = bytes[0];
    }

    // Directly to USB not used
    // Detected that EOI has been asserted
    // if (eoiDetected) {
    //     // If eot_enabled then add EOT character
    //     if (io.bus.config.eot_en)
    //         dataStream.sputc(io.bus.config.eot_ch);
    // }

    // Return controller to idle state
    if (type == NType::Controller) {

        // Untalk bus and unlisten controller
        // if (unAddressDevice()) {
        // }

        // Set controller back to idle state
        setState(CTRL_IDLE);

    } else {
        // Set device back to idle state
        setState(DEV_IDLE);
    }

    // Reset break flag
    // if (txBreak)
    //     txBreak = false;

    return state == HANDSHAKE_COMPLETE;
}

/***** Send a series of characters as data to the GPIB bus *****/
void Node::sendData(const std::u8string_view &str) {

    //  bool err = false;
    uint8_t terminator;
    HandshakeState state;

    // TODO: Ensure this enums match with og
    switch (io.bus.config.eos) {
        case Config::EOS::CR:
        case Config::EOS::LF:
            terminator = 1;
            break;
        case Config::EOS::None:
            terminator = 0;
            break;
        default:
            terminator = 2;
            break;
    }

    // Set control pins for writing data (ATN unasserted)
    if (type == NType::Controller) {
        setState(CTRL_TALK);
    } else {
        setState(DEV_TALK);
    }

    // Write the data string
    for (size_t i = 0; i < str.size(); ++i) {
        if (io.bus.config.eoi && (terminator == 0)) {         // If EOI asserting is on and EOI will not be sent with the terminator
            state = writeByte(str[i], i == (str.size() - 1)); // Send EOI on last character
        } else {                                              // Otherwise ignore non-escaped CR, LF and ESC
            state = writeByte(str[i], false);
        }
        if (state != HandshakeState::HANDSHAKE_COMPLETE) {
            // TODO: Handshake failed
            break;
        }
    }

    // Terminators and EOI
    if (terminator && (state == HANDSHAKE_COMPLETE)) {
        switch (io.bus.config.eos) {
            case Config::EOS::CR:
                writeByte(CR, io.bus.config.eoi);
                break;
            case Config::EOS::LF:
                writeByte(LF, io.bus.config.eoi);
                break;
            case Config::EOS::None:
                break;
            default:
                writeByte(CR, false);
                writeByte(LF, io.bus.config.eoi);
                break;
        }
    }

    if (type == NType::Controller) { // Controller mode
        // Controller - set lines to idle
        setState(CTRL_IDLE);
    } else { // Device mode
        // Set control lines to idle
        setState(DEV_IDLE);
    }
}

int Node::init(void) {
    for (auto &gpio : io.pinout.__raw_pinout) {
        gpio_set_function(gpio, GPIO_FUNC_SIO);
        gpio_set_pulls(gpio, true, false); // TODO: can pullups be left always on?
    }
    return 0;
}

} // namespace GPIB
