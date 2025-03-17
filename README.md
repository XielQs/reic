# reic - Rei C

A programming language written in C++

This project is NOT intended to be a serious programming language. It is a toy project to learn about programming languages and compilers.

I call it Rei C because it compiles to C, and Rei is for Rei Ayanami from Neon Genesis Evangelion!!

[License](LICENSE)

## Build

You can use `make` to build the project. It will create a `reic` executable in the `build` directory.

```bash
make -j$(nproc) # use all available cores
```

## Usage

```bash
Usage: <filename> [-v|--verbose] [-h|--help] [-o <output_file>] [--compile] [--run]
Options:
  -v, --verbose   Enable verbose output
  -h, --help      Show this help message
  -o <filename>   Specify output file name
      --compile   Compile the output file using clang
      --run       Run file after compilation
```

It uses `clang` to compile the generated C code because f#ck GCC.

```bash

## Features

- Basic arithmetic operations (addition, subtraction, multiplication, division)
- Variables (not really lol)
- Printing to console (can't print numbers yet)
