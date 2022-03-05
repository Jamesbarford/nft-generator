# NFT Generator (Image manipulation program)

Create `n` number of slightly pixliated images that would make a nice
contribution to clogging up opensea's databases.

<img src="https://github.com/Jamesbarford/image-processing/blob/main/examples/alexander_great_head.png" width="400" />
<img src="https://github.com/Jamesbarford/image-processing/blob/main/examples/out.gif" width="400" />


## Features:
- [x] Edge detection
- [x] Pixilating images
- [x] Messing about with color pallets
- [x] Creating gifs (well kinda, there's a bash script in the `./scripts`)
- [ ] Ability to make millions from selling these....

# Building
This requires installing `libpq` for handling `png` files:

__mac:__
```sh
brew install libpq
```

__Fedora:__
```sh
sudo dnf install libpq-devel

```

__Ubuntu:__
```sh
sudo apt-get install libpq-dev
```

## Compiling
After installing the dependencies:
```sh
make
```

# Usage:
The best way to get to grips with this is play around with some images and see
what happens.
