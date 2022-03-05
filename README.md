# NFT Generator (Image manipulation program)

Create `n` number of slightly pixliated images that would make a nice
contribution to clogging up opensea's databases.

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
