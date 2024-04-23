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

# https://github.com/usedbytes/rp2040-serial-bootloader/blob/main/mkasm.py

"""Converts a binary file to an assembly file.

This script takes a binary file as input and generates an assembly file
(.S) containing the hexadecimal representation of the binary data.

Usage:
    python asm.py input_file output_file

Arguments:
    input_file (str): Path to the input binary file.
    output_file (str): Path to the output assembly file.
    section (str): Section this assembly file should use.
    attributes (str): Attributes this assembly should use.

Example:
    python asm.py input.bin output.s
"""

import argparse
import sys
from typing import List


def write_assembly(outfile: object, ifile: str, section: str, attributes: str, idata: bytes) -> None:
    """Write assembly code to the output file.

    Args:
        outfile (object): Output file object.
        ifile (str): Path of the input file.
        section (str): Section this assembly file should use.
        attributes (str): Attributes this assembly should use.
        idata (bytes): Binary data read from the input file.

    Returns:
        None
    """
    outfile.write(f"// Generated assembly file from: {ifile}\n\n")
    outfile.write(".cpu cortex-m0plus\n")
    outfile.write(".thumb\n\n")
    outfile.write(f".section .{section}, \"{attributes}\"\n\n")
    for offs in range(0, len(idata), 16):
        chunk: List[int] = idata[offs:min(offs + 16, len(idata))]
        outfile.write(f".byte {', '.join(f'0x{b:02x}' for b in chunk)}\n")


def main() -> None:
    """Convert binary file to assembly file."""
    parser = argparse.ArgumentParser(description="Convert binary file to assembly file")
    parser.add_argument("ifile", type=str, help="Input file (binary)")
    parser.add_argument("ofile", type=str, help="Output file (assembly)")
    parser.add_argument("section", type=str, help="Section this assembly file should use")
    parser.add_argument("attributes", type=str, help="Attributes this assembly should use")
    args = parser.parse_args()

    try:
        with open(args.ifile, "rb") as infile:
            idata: bytes = infile.read()
    except FileNotFoundError:
        sys.exit(f"Error: Input file '{args.ifile}' not found")
    except OSError as e:
        sys.exit(f"Error: Failed to read input file '{args.ifile}': {e}")

    try:
        with open(args.ofile, "w", encoding="utf-8") as outfile:
            write_assembly(outfile, args.ifile, args.section, args.attributes, idata)
    except OSError as e:
        sys.exit(f"Error: Failed to write output file '{args.ofile}': {e}")


if __name__ == "__main__":
    main()
