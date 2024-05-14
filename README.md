<!-- PROJECT: PMPi -->
<!-- TITLE: PMPi-Firmware -->
<!-- FONT: IBM Plex -->
<!-- KEYWORDS: Controller, Raspberry Pi Pico W, Embedded, Firmware -->
<!-- LANGUAGES: C, C++, Python -->
<!-- TECHNOLOGY: Mongoose Embedded Web Server, RESTful API -->
<!-- STATUS: Work In Progress -->

![PMPi-Firmware-Logo](<images/PMPi Firmware.png>)

[About](#about) - [Usage](#usage) - [Related](#related) - [License](#license)

## Status

**`Work In progress`**
> *Current main focus has been with the bootloader - May 2024*

## About
<!-- DESCRIPTION START -->
This is the Firmware repository for the PMPi Project, a wireless PM2813 GPIB Controller using a Raspberry Pi Pico W.

Please refer to the [parent repository](https://github.com/LeHuman/PMPi)
<!-- DESCRIPTION END -->

## Usage

### Requirements

- [CMake](https://cmake.org/) >= 3.14
- [Pico C/C++ SDK](https://github.com/raspberrypi/pico-sdk) >= v1.5.1
- [Raspberry Pi Pico W](https://datasheets.raspberrypi.com/picow/pico-w-product-brief.pdf)
  - Should be adaptable to other RP2040 boards with wifi connectivity, but not tested
- [Raspberry Pi Debug Probe](https://www.raspberrypi.com/products/debug-probe/)
  - Optional

### Building

This project uses Raspberry Pi's [C/C++ SDK](https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html) setup to build this project.

As such, ensure to open VSCode using the Pico shortcut (on Windows) to properly set environment variables.\
Otherwise, make sure to set the `PICO_SDK_PATH` environment variable and have a version of `OpenOCD` available on path for debugging.

The following build commands are presented particularly perfunctorily.

```sh
mkdir build
cd build
export PICO_SDK_PATH=../../pico-sdk
cmake ..
make
```

### Run clang-format

Use the following commands from the project's root directory to check and fix C++ and CMake source style.
This requires *clang-format*, *cmake-format* and *pyyaml* to be installed on the current system.

```bash
cmake -S test -B build/test

# view changes
cmake --build build/test --target format

# apply changes
cmake --build build/test --target fix-format
```

See [Format.cmake](https://github.com/TheLartians/Format.cmake) for details.
These dependencies can be easily installed using pip.

```bash
pip install clang-format==14.0.6 cmake_format==0.6.11 pyyaml
```

<!-- ### Build the documentation

The documentation is automatically built and [published](https://thelartians.github.io/ModernCppStarter) whenever a [GitHub Release](https://help.github.com/en/github/administering-a-repository/managing-releases-in-a-repository) is created.
To manually build documentation, call the following command.

```bash
cmake -S documentation -B build/doc
cmake --build build/doc --target GenerateDocs
# view the docs
open build/doc/doxygen/html/index.html
```

To build the documentation locally, you will need Doxygen, jinja2 and Pygments installed on your system. -->

## Related

- vha3/[Serial_bootloader](https://github.com/vha3/Hunter-Adams-RP2040-Demos/tree/master/Bootloaders/Serial_bootloader)
- usedbytes/[rp2040-serial-bootloader](https://github.com/usedbytes/rp2040-serial-bootloader)

## License

GNU General Public License, version 2

2020 Raspberry Pi (Trading) Ltd.

2013 Sergey Lyubka

2024 Cesanta Software Limited
