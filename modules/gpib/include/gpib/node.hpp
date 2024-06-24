#pragma once

#include <common/millis.hpp>

#include "interface.hpp"

namespace GPIB {

class Node {
protected:
    enum State {
        INVALID = 0x0,
        // Controller mode
        CTRL_INIT, // CINI = 0x01, // Initialisation
        CTRL_IDLE, // CIDS = 0x02, // Controller idle state
        CTRL_COMD, // CCMS = 0x03,  // Controller command state
        CTRL_TALK, // CTAS = 0x04, // Controller talker active state
        CTRL_LISN, // CLAS = 0x05, // Controller listner active state
        // Listener/device mode
        DEV_INIT, // DINI = 0x06, // Device initialise state
        DEV_IDLE, // DIDS = 0x07, // Device idle state
        DEV_LISN, // DLAS = 0x08, // Device listener active (listening/receiving)
        DEV_TALK, // DTAS = 0x09, // Device talker active (sending) state
    };

    enum HandshakeState {
        // Common
        HANDSHAKE_START,
        HANDSHAKE_COMPLETE,
        IFC_ASSERTED,
        ATN_ASSERTED,
        // Read
        WAIT_FOR_DATA,
        READ_DATA,
        DATA_ACCEPTED,
        // Write
        WAIT_FOR_RECEIVER_READY,
        PLACE_DATA,
        DATA_READY,
        RECEIVER_ACCEPTING
    };

    enum TransmitModes {
        TM_IDLE,
        TM_RECV,
        TM_SEND
    };

    enum NType {
        Device,
        Controller,
    };

    GPIBIOInterface io;
    State state = State::INVALID;
    const NType type;

    void setTransmitMode(TransmitModes mode);

    HandshakeState readByte(uint8_t *db, bool readWithEoi, bool *eoi);

    HandshakeState writeByte(uint8_t db, bool isLastByte);

    virtual void setState(State state) = 0;

public:
    bool deviceAddressed = false;

    consteval Node(const PinOut &pinout, const Config &config, const NType type) : io(pinout, config), type(type) {}

    virtual int init(void);
};

} // namespace GPIB