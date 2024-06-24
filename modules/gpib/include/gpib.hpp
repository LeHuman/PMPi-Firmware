#pragma once

#include "gpib/controller.hpp"
#include "gpib/device.hpp"

namespace GPIB {

constexpr Config default_config = {
    .eoi = false,
    .caddr = 0x0,
    .paddr = 0x0,
    .saddr = 0x0,
    .eos = Config::EOS::CRLF,
    .stat = 0,
    .amode = Config::AMode::Off,
    .rtmo = 2000,
    // .vstr = "1.0.0",
    .eor = Config::EOR::CRLF,
    // .sname = "picoGPIB",
    .serial = 1337,
    .idn = Config::IDN::name,
};

} // namespace GPIB
