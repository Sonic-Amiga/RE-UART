# Preface

This project is a simple implementation of [1801VP1-035/065 UART](https://github.com/1801BM1/k1801/tree/master/065)
emulator, which runs on [REmulator](https://github.com/1801BM1/remul) hardware. This allows to easily substitute a
broken VP1 chip on MC1201 series of single-board PDP-11-compatible computers using
user ROM socket and some additional wiring for missing signals. The project is originally
developed for [Koszalińskie Muzeum Elektroniki Komputerowej ELKON](https://www.facebook.com/elkonelektronika/) with
the goal of diagnosing and repairing their broken MC1201.02 boards.

# Limitations

Due to limits of REmulator hardware, and also because the project is developed in a spare time,
the emulation is not complete. The following features are currently missing:

* RTS/CTS flow control
* Loopback test mode
* Interrupts

So, only simple polled mode is currently implemented, which is only enough for ROM monitor to run.
Do not expect operating systems to work with this replacement.

At least some of these limitations can be worked on at the expense of, for example, configuration jumpers, but i am
not sure if i am going to do that. Because, again, the main purpose of this project is just troubleshooting the board.
For permanent replacement of a missing UART chip it would be far better to develop a proper in-place substitute
using a CPLD. Nevertheless, pull requests are welcome.

# Installation

This firmware is compatible with the original RE-mulator's bootloader and should be flashed at address 0x08004000
instead of original ROM emulation code. It is possible to use STM32CubeProgrammer or any other tool. It should also
be possible to use bootloader's serial upload mode, however i haven't tested it.

Since /DOUT line is not available on the user ROM socket, it should be wired from your machine to XCS1 pin (marked X2).
It is also strongly adviced to wire "EXT READY" (reset) connector to /DCLO and /ACLO generation circuit of your machine
in order to guarantee stable bootup.

# Configuration

Jumpers on RE-mulator board are used to select UART base address and baud rate, similar to jumpers or
DIP switches on the original machine's board. In tables below "0" means jumper installed, "1" removed:

#### SA5 - select UART base address
| Value | Base address
|-------|-------------
| 0     | 177560 (default console port)
| 1     | 176560

#### SA4...SA1 - select baud rate
| Value | Baud rate
|-------|----------------
| 0000  | 50
| 0001  | 75
| 0010  | 100
| 0011  | 150
| 0100  | 200
| 0101  | 300
| 0110  | 600
| 0111  | 1200
| 1000  | 2400
| 1001  | 4800
| 1010  | 9600
| 1011  | 19200
| 1100  | 57600
| 1101  | 115200
| 1110  | 115200
| 1111  | 115200

Unfortunately there are no more jumpers available on RE-mulator (SA6 is used by the bootloader to enter download mode),
so number of data bits is hardcoded to 8 and parity is not used ("none"). Number of stop bits is always 2, as per
specification of the original UART.

# License

This is a very niche project, so i don't think it makes sense to care a lot about rights. Let's consider this work
public domain. You may always contact me at pavel_fedin@mail.ru
