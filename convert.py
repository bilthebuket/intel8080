#!/usr/bin/env python3
"""
convert_bin_to_hex.py
Read a binary file and print its bytes as hex values separated by spaces.
Usage:
    python3 convert_bin_to_hex.py inputfile
"""

import sys
from pathlib import Path

def main():
    if len(sys.argv) != 2:
        print(f"Usage: {Path(sys.argv[0]).name} <binary-file>")
        sys.exit(1)

    filename = sys.argv[1]
    try:
        with open(filename, "rb") as f:
            data = f.read()
    except OSError as e:
        sys.exit(f"Error reading {filename}: {e}")

    # Format: two hex digits per byte, uppercase, separated by spaces
    hex_bytes = " ".join(f"{b:02X}" for b in data)
    print(hex_bytes)

if __name__ == "__main__":
    main()
