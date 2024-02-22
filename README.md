# libspike-interfaces

**libspike-interfaces** is a C language API interface for the RISC-V ISA Simulator (Spike), designed to facilitate usage of Spike from other languages via Foreign Function Interface (FFI).

## Overview

This project provides a set of C language functions and data structures that serve as a bridge between Spike and other programming languages. By using these interfaces, developers can seamlessly integrate Spike functionality into their projects without needing to directly interact with Spike's internals.

## Features

- **FFI Compatibility**: Designed for easy integration with other programming languages through Foreign Function Interface mechanisms.
- **Nix Build System**: Utilizes the Nix package manager for easy dependency management and reproducible builds.
- **Example Applications**: Includes example applications demonstrating how to use the libspike-interfaces library.

## Getting Started

### Set Up the Development Environment

We use nix flake as our primary build system. If you have not installed nix, install it following the [guide](https://nixos.org/manual/nix/stable/installation/installing-binary.html), and enable flake following the [wiki](https://nixos.wiki/wiki/Flakes#Enable_flakes). Or you can try the [installer](https://github.com/DeterminateSystems/nix-installer) provided by Determinate Systems, which enables flake by default.

### Build the Library

To build the library, run:

```bash
nix build .#libspike-interfaces
```

The resulting shared library (`libspike-interfaces.so`) will be available in the `result/lib`.

### Run & Test Example Code

You can run and test the example code as follows:

```bash
nix build .#elfloader
./result/bin/elfloader ./resources/conv-mlir.elf
```

Alternatively, you can use nix to run in one line:

```bash
nix run .#elfloader ./resources/conv-mlir.elf
```

## Contributing

Contributions are welcome! Please feel free to open an issue or submit a pull request for any improvements or fixes you'd like to see.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
