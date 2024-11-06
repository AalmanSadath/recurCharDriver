# recurCharDriver: recur.c

## Overview
recur.c is a Linux kernel module that creates a character device to output recursive patterns. This device can track sequences, reset them, and provide an index-based pattern of numbers. The module allows multiple device instances and offers configurable parameters for ease of use.

## Features
Recursive Sequence Generation: Outputs a sequence based on initial inputs (similar to a Fibonacci sequence).
Device Reset: Supports resetting the sequence via an IOCTL command.
Dynamic Major Number Allocation: Configurable through a module parameter.
Multiple Device Instances: Allows setting the number of /dev/recur instances.

## Installation
### Clone repo
```
git clone https://github.com/AalmanSadath/recurCharDriver && cd recurCharDriver
```

### Compile the Module:
```
make
```

### Insert the Module:
```
sudo insmod recur.ko
```
or for multiple devices(5) and specific major number(273)
```
sudo insmod recur.ko max_devices=5 param_major_num=273
```
The module will automatically create and intitialize file permissions to 666 for /dev/recur*

## Module Parameters
max_devices: Sets the number of /dev/recur device instances.
param_major_num: Sets the major device number (use 0 for dynamic allocation).

## Usage

This section provides details on how to use `recur.c`, including initialising the recursive function with a custom tool sendToRecur.

### Compiling `sendToRecur`

To compile `sendToRecur.c`, ensure you have `gcc` installed and run the following command:

```bash
make sendToRecur
```

This command will compile `sendToRecur.c` and produce an executable named `sendToRecur` in the current directory.

### Using `sendToRecur`

`sendToRecur` is used to output 64-bit binary representations of integers passed as command-line arguments. You can pipe this output directly to a device using `dd` for writing to `/dev/recur*`.

#### Example Command

```bash
./sendToRecur 1 2 | dd of=/dev/recur* count=2 bs=8
```

- **`./sendToRecur 1 2`**: Runs the `sendToRecur` program, which outputs the 64-bit binary representation of the integers `1` and `2`.
- **`| dd of=/dev/recur* count=2 bs=8`**:
  - **`of=/dev/recur*`**: Specifies the output device (replace `recur*` with the appropriate device name (recur0, recur1, etc.)).
  - **`count=2`**: Limits the number of 8-byte blocks to write.
  - **`bs=8`**: Specifies a block size of 8 bytes (matching the 64-bit integer size).

### Testing output of /dev/recur*

The output of /dev/recur* can be viewed using dd and xxd

```bash
dd if=/dev/recur* count=20 bs=8 | xxd -g 8
```

- **`dd if=/dev/recur* count=20 bs=8`**:
  - **`if=/dev/recur*`**: Specifies the input device (e.g., `recur0`, `recur1`).
  - **`count=20`**: Limits the read operation to 20 blocks.
  - **`bs=8`**: Specifies a block size of 8 bytes.

- **`| xxd -g 8`**:
  - **`xxd`**: Converts binary data into a hex dump.
  - **`-g 8`**: Groups the output into 8-byte chunks.
