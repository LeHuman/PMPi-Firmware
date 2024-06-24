#pragma once

#include <stdint.h>

namespace GPIB {

/***** Universal Multiline commands (apply to all devices) *****/
enum UniMultiCMD : uint32_t {
    GC_GTL = 0x01,
    GC_SDC = 0x04,
    GC_PPC = 0x05,
    GC_GET = 0x08,
    GC_TCT = 0x09,
    GC_LLO = 0x11,
    GC_DCL = 0x14,
    GC_PPU = 0x15,
    GC_SPE = 0x18,
    GC_SPD = 0x19,
    GC_LAD = 0x20,
    GC_UNL = 0x3F,
    GC_UNT = 0x5F,
    GC_TAD = 0x40,
    GC_PPE = 0x60,
    GC_PPD = 0x70,
};

} // namespace GPIB