# gbc

GameBoy Color emulator

## Requirements

* CMake
* C compiler toolchain


## Dependencies

* SDL2 (included)


## Build

First generate platform build files

	mkdir build
	cd build
	cmake ..
	cmake --build . --config Release

You should see `gbc` and `SDL3` binaries in the Release directory within build.


## Run

	gbc <rom filepath>


## Helpful Resources

- https://gbdev.io/pandocs/single.html
- https://wiki.libsdl.org/SDL3/SDL_CreateRenderer
