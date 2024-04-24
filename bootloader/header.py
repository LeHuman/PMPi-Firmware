#!/usr/bin/env python3
# Derived from pad_checksum in the Pico SDK, which carries the following
# LICENSE.txt:
# Copyright 2020 (c) 2020 Raspberry Pi (Trading) Ltd.
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
# following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
#    disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# https://github.com/usedbytes/rp2040-serial-bootloader/blob/main/gen_imghdr.py
"""Generate flash header from binary.

This script takes a binary file as input and generates a flash header with
metadata such as load address, size, and CRC.

Usage:
    python header.py input_file output_file [-a ADDR]

Arguments:
    input_file (str): Path to the input binary file.
    output_file (str): Path to the output flash header.
    -a, --addr (int): Load address of the application image.

Example:
    python header.py app.bin app_hdr.bin
"""

import argparse
import binascii
import zlib
import sys


def any_int(x: str) -> int:
    """Convert a string to an integer of any base."""
    try:
        return int(x, 0)
    except ValueError as exc:
        raise argparse.ArgumentTypeError(f"expected an integer, not '{x}'") from exc


def calculate_crc32(filepath, start_address, end_address):
    # Read the ihex file
    try:
        with open(filepath, 'r', encoding='utf-8') as file:
            lines = file.readlines()
    except FileNotFoundError:
        sys.exit(f"Could not open input file '{filepath}'")

    # Initialize variables to keep track of the current address
    upper_address = 0

    # Extract data within the specified address range
    data = b''
    crc32_value = 0

    for line in lines:
        if line.startswith(':'):
            record_type = int(line[7:9], 16)
            if record_type == 0:
                record_address = int(line[3:7], 16) + (upper_address << 16)
                record_data = binascii.unhexlify(line[9:-3])
                if start_address <= record_address < end_address:
                    data += record_data
            elif record_type == 4:  # Extended Linear Address Record
                upper_address = int(line[9:13], 16)

    # Calculate CRC32 checksum
    # crc32_value = zlib.crc32(data)
    crc32_value = binascii.crc32(data)
    # crc32_value = sum(data)

    return len(data), crc32_value


def main() -> None:
    """Convert ihex file to a bootloader header section"""
    parser = argparse.ArgumentParser()
    parser.add_argument("ifile", type=str, help="Input application binary (binary)")
    parser.add_argument("ofile", type=str, help="Output header file (binary)")
    parser.add_argument("address", type=any_int, help="Load address of the main program")
    parser.add_argument("length", type=any_int, help="Size of the main program flash space")
    args = parser.parse_args()

    vtor: int = args.address
    crc_sz, crc = calculate_crc32(args.ifile, args.address, args.address + args.length)

    odata: bytes = vtor.to_bytes(4, byteorder='little') + \
        crc.to_bytes(4, byteorder='little') + \
        crc_sz.to_bytes(4, byteorder='little') +\
        (0xEFBEADDE).to_bytes(4, byteorder='little')  # DEADBEEF in ihex

    try:
        with open(args.ofile, "wb") as ofile:
            ofile.write(odata)
    except OSError:
        sys.exit(f"Could not open output file '{args.ofile}'")

    print(f"Header VTOR:  {hex(vtor)} {vtor}")
    print(f"Header CRC32: {hex(crc)} {crc}")
    print(f"Header CRC32 SZ: {hex(crc_sz)} {crc_sz}")


if __name__ == "__main__":
    main()
