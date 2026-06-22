# chip-8

> [!WARNING]
> Please note that this project is a work in progress.

A simple CHIP-8 interpreter in C.

## Build

Just run make. It will create `chip-8-run` in `build` folder.

```
make
```

## Usage

```
chip-8-run [PATH TO YOUR PROGRAM]
```

*Note that very few instructions have been implemented. Your programs will most likely not work on this.*

### Example

Right now, the interpreter can only run the IBM logo program. You may find and download from anywhere on the web. It can be run as follows:

```
chip-8-run ibm-logo-ch.8
```

## Credits & References

- [Guide to making a CHIP-8 emulator | Tvil](https://tobiasvl.github.io/blog/write-a-chip-8-emulator)
- [Cowgod's Chip-8 Technical Reference v1.0](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#2.4
)
