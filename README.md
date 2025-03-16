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
./build/reic <source_file.reic>
```
