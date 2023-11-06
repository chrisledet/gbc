# gbc

Game Boy Color emulator

## Requirements

* CMake
* C compiler toolchain


## Dependencies

* SDL3 (included)


## Layout

- externals: third party libraries
- include: header files for emulator
- src: source code for emulator
- test: tests and data

## Build

Run the following commands inside gbc directory:

	cmake -S . -B build
	cmake --build build [--config Release]


You should see `gbc` and `SDL3` binaries in the Debug (or Release) within build directory.


## Run

	gbc <rom filepath>


## Helpful Resources

- https://gbdev.io/pandocs/single.html
- https://wiki.libsdl.org/SDL3/SDL_CreateRenderer
- https://github.com/wheremyfoodat/Gameboy-logs
- https://bitwisecmd.com/
- GameBoyProgManVer1.1.pdf
