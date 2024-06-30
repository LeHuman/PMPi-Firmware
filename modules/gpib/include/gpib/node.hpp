#pragma once

#include <streambuf>
#include <string>

#include <common/millis.hpp>

#include "interface.hpp"

namespace GPIB {

/***** Control characters *****/
constexpr uint8_t ESC = 0x1B;  // The USB escape char
constexpr uint8_t CR = 0xD;    // Carriage return
constexpr uint8_t LF = 0xA;    // Newline/linefeed
constexpr uint8_t PLUS = 0x2B; // '+' character

constexpr uint8_t MAX_DATA_LEN = UINT8_MAX;
constexpr uint8_t MAX_DATA_POOL = 16;

class Data {
private:
    std::array<uint8_t, MAX_DATA_LEN + 1> data = {0};
    // std::array<uint8_t, 3U> last = {0};
    bool taken = false;
    size_t length = 0;

public:
    bool give() {
        if (taken == false) {
            return false;
        }
        taken = false;
        length = 0;
        return true;
    }

    bool take() {
        if (taken == true) {
            return false;
        }
        taken = true;
        length = 0;
        return true;
    }

    bool push(uint8_t val) {
        if (length >= data.size() - 1) {
            return false;
        }
        data[length++] = val;
        // last[2] = last[1];
        // last[1] = last[0];
        // std::rotate(last.rbegin(), last.rbegin() + 1, last.rend());
        // last[0] = val;
        return true;
    }

    std::string_view to_string() {
        data[length] = '\0';
        return reinterpret_cast<char *>(data.data());
    }

    bool has_terminator(Config::EOR eorSequence) {
        if (length == 0) {
            return false;
        }
        auto last = data.begin() + (length - 1);
        // Look for specified terminator (CR+LF by default)
        switch (eorSequence) {
            case Config::EOR::CRLF:
                // CR+LF terminator
                if (length > 1 && last[0] == LF && last[-1] == CR) {
                    return true;
                }
                break;
            case Config::EOR::CR:
                // CR only as terminator
                if (last[0] == CR) {
                    return true;
                }
                break;
            case Config::EOR::LF:
                // LF only as terminator
                if (last[0] == LF) {
                    return true;
                }
                break;
            case Config::EOR::None:
                // No terminator (will rely on timeout)
                break;
            case Config::EOR::LFCR:
                // Keithley can use LF+CR instead of CR+LF
                if (length >= 2 && last[0] == CR && last[-1] == LF) {
                    return true;
                }
                break;
            case Config::EOR::ETX:
                // Solarton (possibly others) can also use ETX (0x03)
                if (last[0] == 0x03) {
                    return true;
                }
                break;
            case Config::EOR::CRLF_ETX:
                // Solarton (possibly others) can also use CR+LF+ETX (0x03)
                if (length >= 3 && last[0] == 0x03 && last[-1] == LF && last[-2] == CR) {
                    return true;
                }
                break;
            default:
                // Use CR+LF terminator by default
                if (length >= 2 && last[0] == LF && last[-1] == CR) {
                    return true;
                }
                break;
        }
        return false;
    }
};

// std::array<Data, MAX_DATA_POOL> data_pool;

class Node {
protected:
    enum State {
        INVALID = 0x0,
        INIT,
        IDLE,
        TALK,
        LISTEN,
        COMMAND, // NOTE: Only available to Controller
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

    bool receiveData(Data &data);
    bool receiveData(Data &data, bool detectEoi, bool detectEndByte, uint8_t endByte);

    void sendData(const std::string_view &str);

    virtual int init(void);
};

} // namespace GPIB