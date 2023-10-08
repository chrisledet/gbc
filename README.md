# gbc

gameboy color emulator

## Dependencies

* SDL2 (included)
* CMake
* C compiler

## Build

First generate platform build files

	mkdir build
	cd build
	cmake ...

Windows

	msbuild gbc.sln /p:Configuration=Release


Run

	cd Release
	.\gbc.exe



## Helpful Resources


- https://wiki.libsdl.org/SDL3/SDL_CreateRenderer
