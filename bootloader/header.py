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
import sys


def any_int(x: str) -> int:
    """Convert a string to an integer of any base."""
    try:
        return int(x, 0)
    except ValueError as exc:
        raise argparse.ArgumentTypeError(f"expected an integer, not '{x}'") from exc


parser = argparse.ArgumentParser()
parser.add_argument("ifile", type=str, help="Input application binary (binary)")
parser.add_argument("ofile", type=str, help="Output header file (binary)")
parser.add_argument("-a", "--addr", type=any_int, help="Load address of the application image")
args = parser.parse_args()

try:
    with open(args.ifile, "rb") as infile:
        idata: bytes = infile.read()
except FileNotFoundError:
    sys.exit(f"Could not open input file '{args.ifile}'")

vtor: int = args.addr
size: int = len(idata)
crc: int = binascii.crc32(idata)

odata: bytes = vtor.to_bytes(4, byteorder='little') + \
    size.to_bytes(4, byteorder='little') + \
    crc.to_bytes(4, byteorder='little')

try:
    with open(args.ofile, "wb") as ofile:
        ofile.write(odata)
except OSError:
    sys.exit(f"Could not open output file '{args.ofile}'")
