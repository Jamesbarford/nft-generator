# NFT Generator (Image manipulation program)

Fancy buying a super yacht but are lacking in finances? This aims to fix that.
Create `n` number of slightly modified images that would make a nice
contribution to opensea's databases.

_only works on png files for the time being_

Before ($unsellable)             |  After $1,000,000 (appx)
:-------------------------:|:-------------------------:
<img src="https://github.com/Jamesbarford/nft-generator/blob/main/examples/alexander_great_head.png" width="350" /> | <img src="https://github.com/Jamesbarford/nft-generator/blob/main/examples/out.gif" width="350" />


## Features:
- [x] Edge detection
- [x] Pixilating images
- [x] Messing about with color pallets
- [x] Creating gifs (well kinda, there's a bash script in the `./scripts`)
- [ ] Ability to make millions from selling these....

# Building
This requires installing `libpng` for handling `png` files:

__mac:__
```sh
brew install libpng
```

__Fedora:__
```sh
sudo dnf install libpng-devel

```

__Ubuntu:__
```sh
sudo apt-get install libpng-dev
```

## Compiling
After installing the dependencies:
```sh
make
```

# Usage:
The best way to get to grips with this is play around with some images and see
what happens.

```sh
# some starter commands:

./src/nftgen --file ./example/alexander_great_head.png --block-size 10

./src/nftgen --file ./example/alexander_great_head.png  --block-size 0

# This will seg-fault after creating 3 images.
./src/nftgen --file ./example/alexander_great_head.png --edge-detection

# A precanned script
./scripts/mixcolors.sh ./example/alexander_great_head.png
```

## Disclaimer

This is joke and just for fun, I was interested in experimenting with images
and pixel manipulation. I'd strongly advise against trying to sell anything
created by this program on opensea as listing something on the website will
set you back a couple hundred pounds. Though if you do and make millions,
I'd be like the program to be mentioned :).

