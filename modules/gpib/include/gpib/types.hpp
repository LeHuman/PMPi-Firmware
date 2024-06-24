#pragma once

#include <array>

#include <stdint.h>

namespace GPIB {

template <typename T>
struct [[gnu::packed]] GPIBIO {
    T DIO1; // Digital IO 1
    T DIO2; // Digital IO 2
    T DIO3; // Digital IO 3
    T DIO4; // Digital IO 4
    T DIO5; // Digital IO 5
    T DIO6; // Digital IO 6
    T DIO7; // Digital IO 7
    T DIO8; // Digital IO 8

    T REN;  // Remote Enable
    T EOI;  // End or Identify
    T IFC;  // Interface Clear
    T SRQ;  // Service Request
    T ATN;  // Attention

    T DAV;  // Data Valid
    T NRFD; // Not Read for Data
    T NDAC; // Not Data Accepted
};

using GPIBPins = GPIBIO<uint8_t>;
using GPIBMask_t = uint32_t;
using GPIBMasks = GPIBIO<GPIBMask_t>;

typedef union {
    GPIBPins pin;
    const std::array<const uint8_t, 16> __raw_pinout;

    struct GroupMask {
        const GPIBMask_t dio;
        const GPIBMask_t ctrl;
        const GPIBMask_t hand;
        const GPIBMask_t all;
    };

    consteval uint32_t dio_mask(void) const {
        return (1UL << pin.DIO1) | (1UL << pin.DIO2) | (1UL << pin.DIO3) | (1UL << pin.DIO4) | (1UL << pin.DIO5) | (1UL << pin.DIO6) | (1UL << pin.DIO7) | (1UL << pin.DIO8);
    }
    consteval uint32_t ctrl_mask(void) const {
        return (1UL << pin.REN) | (1UL << pin.EOI) | (1UL << pin.IFC) | (1UL << pin.SRQ) | (1UL << pin.ATN);
    }
    consteval uint32_t hand_mask(void) const {
        return (1UL << pin.DAV) | (1UL << pin.NRFD) | (1UL << pin.NDAC);
    }
    consteval uint32_t all_mask(void) const {
        return dio_mask() | ctrl_mask() | hand_mask();
    }

    consteval GPIBMasks get_pin_mask(void) const {
        return {
            .DIO1 = (uint32_t)(uint32_t(1U) << pin.DIO1),
            .DIO2 = (uint32_t)(uint32_t(1U) << pin.DIO2),
            .DIO3 = (uint32_t)(uint32_t(1U) << pin.DIO3),
            .DIO4 = (uint32_t)(uint32_t(1U) << pin.DIO4),
            .DIO5 = (uint32_t)(uint32_t(1U) << pin.DIO5),
            .DIO6 = (uint32_t)(uint32_t(1U) << pin.DIO6),
            .DIO7 = (uint32_t)(uint32_t(1U) << pin.DIO7),
            .DIO8 = (uint32_t)(uint32_t(1U) << pin.DIO8),
            .REN = (uint32_t)(uint32_t(1U) << pin.REN),
            .EOI = (uint32_t)(uint32_t(1U) << pin.EOI),
            .IFC = (uint32_t)(uint32_t(1U) << pin.IFC),
            .SRQ = (uint32_t)(uint32_t(1U) << pin.SRQ),
            .ATN = (uint32_t)(uint32_t(1U) << pin.ATN),
            .DAV = (uint32_t)(uint32_t(1U) << pin.DAV),
            .NRFD = (uint32_t)(uint32_t(1U) << pin.NRFD),
            .NDAC = (uint32_t)(uint32_t(1U) << pin.NDAC),
        };
    }

    consteval GroupMask get_group_mask(void) const {
        return {dio_mask(), ctrl_mask(), hand_mask(), all_mask()};
    }

} PinOut;

typedef struct {
    struct EOS_s {
        enum EOS_e : uint8_t {
            CRLF,
            CR,
            LF,
            None,
        };
    };
    using EOS = EOS_s::EOS_e;
    struct AMode_s {
        enum AMode_e : uint8_t {
            Off,
            Prologix,
            Onquery,
            Continuous,
        };
    };
    using AMode = AMode_s::AMode_e;
    struct EOR_s {
        enum EOR_e : uint8_t {
            CRLF,
            CR,
            LF,
            None,
            LFCR,
            ETX,
            CRLF_ETX, // CRLF+ETX
            SPACE,
        };
    };
    using EOR = EOR_s::EOR_e;
    struct IDN_s {
        enum IDN_e : uint8_t {
            disable,     // don't respond to *idn?
            name,        // send name
            name_serial, // send name+serial
        };
    };
    using IDN = IDN_s::IDN_e;

    const bool eoi;        // Assert EOI on last data char written to GPIB - 0-disable, 1-enable
    const uint8_t caddr;   // This interface address
    const uint8_t paddr;   // Primary address to use when addressing device
    const uint8_t saddr;   // Secondary address to use when addressing device
    const EOS eos;         // EOS (end of send to GPIB) characters [0=CRLF, 1=CR, 2=LF, 3=None]
    const uint8_t stat;    // Status byte to return in response to a serial poll
    const AMode amode;     // Auto mode setting (0=off; 1=Prologix; 2=onquery; 3=continuous);
    const int rtmo;        // Read timout (read_tmo_ms) in milliseconds - 0-3000 - value depends on instrument
    const char vstr[48];   // Custom version string
    const EOR eor;         // EOR (end of receive from GPIB instrument) characters [0=CRLF, 1=CR, 2=LF, 3=None, 4=LFCR, 5=ETX, 6=CRLF+ETX, 7=SPACE]
    const char sname[16];  // Interface short name
    const uint32_t serial; // Serial number
    const IDN idn;         // Send ID in response to *idn? 0=disable, 1=send name; 2=send name+serial
} Config;

static_assert(sizeof(PinOut) == 16U, "Size of PinOut union is not 16 bytes.");

} // namespace GPIB