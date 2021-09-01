#!/usr/bin/env python3

import argparse

hex_table = {
    "0": 0,
    "1": 1,
    "2": 2,
    "3": 3,
    "4": 4,
    "5": 5,
    "6": 6,
    "7": 7,
    "8": 8,
    "9": 9,
    "a": 10, "A": 10,
    "b": 11, "B": 11,
    "c": 12, "C": 12,
    "d": 13, "D": 13,
    "e": 14, "E": 14,
    "f": 15, "F": 15
};

def hex_to_rgb(hexstr):
    if len(hexstr) != 7 and hexstr[0] != '#':
        return
    rgb_str = " "

    rgb_str += str((hex_table[hexstr[1]] << 4) | hex_table[hexstr[2]]) + ", "
    rgb_str += str ((hex_table[hexstr[3]] << 4) | hex_table[hexstr[4]]) + ", "
    rgb_str += str((hex_table[hexstr[5]] << 4) | hex_table[hexstr[6]]) + " "
    return rgb_str

def main():
    parser = argparse.ArgumentParser(description="Create a c struct")
    parser.add_argument("--hex-vals", type=str, nargs='?')
    args = parser.parse_args()
    rbgvals = []
    if args.hex_vals == None:
        print("Usage: ./<progname> --hex-vals '#FFFFFF,#000000'")
        exit(0)

    for hexstr in args.hex_vals.split(","):
        rbgvals.append(hex_to_rgb(hexstr))

    outstr = f"static int palette[][{len(rbgvals)}] = " + "{\n"

    for rgbstr in rbgvals:
        outstr += "  {" + f"{rgbstr}" + "},\n"

    outstr += "};"
    print(outstr)

if __name__ == "__main__":
    main()
