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

