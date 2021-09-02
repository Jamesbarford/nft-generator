#!/usr/bin/env python3
import random

chars = "abcdef0123456789"

def main() -> None:
    hexstr = "#"
    hexstr += chars[random.randint(0, 15)]
    hexstr += chars[random.randint(0, 15)]
    hexstr += chars[random.randint(0, 15)]
    hexstr += chars[random.randint(0, 15)]
    hexstr += chars[random.randint(0, 15)]
    hexstr += chars[random.randint(0, 15)]
    print(hexstr)


if __name__ == "__main__":
    main()
