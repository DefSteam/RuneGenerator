# RuneGeneratorOS

A C++/X11 Linux port of Watabou's simple rune generator. The application starts in alphabet mode, where clicking a rune replaces that one glyph, and includes a table mode that shows a 10×10 set of generated runes.

## Target platform

- Linux on x86_64
- X11 development headers and libraries
- A C++20 compiler
- `make` and `pkg-config`

On Debian/Ubuntu systems, the native build dependencies are typically available with:

```sh
sudo apt install build-essential pkg-config libx11-dev
```

## Build

```sh
make
```

The default build targets x86_64 Linux by passing `-march=x86-64`. Override `ARCH` if you need a different local tuning option:

```sh
make ARCH=native
```

## Run

```sh
make run
```

or:

```sh
./RuneGenerator
```

## Controls

- **Enter** regenerates the current view.
- **Esc** quits.
- Click the left button to switch to the 10×10 rune table.
- Click the right button to switch to the alphabet view.
- In alphabet view, click an individual rune to replace it while keeping the rest of the alphabet.

## Clean

```sh
make clean
```
