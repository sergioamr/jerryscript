### About

This folder contains files to run JerryScript on Zephyr with
[Arduino 101 / Genuino 101](https://www.arduino.cc/en/Main/ArduinoBoard101)

Zephyr project arduino 101
[Zephyr Arduino 101](https://www.zephyrproject.org/doc/board/arduino_101.html)

### How to build

#### 1. Preface

1, Directory structure

Assume `harmony` as the path to the projects to build.
The folder tree related would look like this.

```
harmony
  + jerry
  |  + targets
  |      + arduino_101
  + zephyr
```


2, Target board

Assume [Arduino 101 / Genuino 101](https://www.arduino.cc/en/Main/ArduinoBoard101)
as the target board.


#### 2. Prepare Zephyr

Follow [this](https://www.zephyrproject.org/doc/getting_started/getting_started.html) page to get
the Zephyr source and configure the environment.

Follow "Building a Sample Application" and check that you can flash the Arduino 101

Remember to source the zephyr environment.

```
source zephyr-env.sh

export ZEPHYR_GCC_VARIANT=zephyr

export ZEPHYR_SDK_INSTALL_DIR=<sdk installation directory>
```

#### 3. Build JerryScript for Zephyr

```
# assume you are in harmony folder
cd jerry
make -f ./targets/arduino_101/Makefile.arduino_101
```

This will generate the following libraries:
```
./build/arduino_101/librelease-cp_minimal.jerry-core.a
./build/arduino_101/librelease-cp_minimal.jerry-libm.lib.a
./build/arduino_101/librelease.external-cp_minimal-entry.a
```

The final Zephyr image will be located here:
```
./build/arduino_101/zephyr/zephyr.strip
```

#### 5. Flashing

1. Connect Flyswatter2 Jtag to Arduino 101 following Zephyr's manual

2. Connect Serial TTL for console output to Pin 0 & 1

3. Connect Power supply.

Should look something like this, depending on your TTL

![alt tag](docs/arduino_101.jpg?raw=true "Example")

Flash the code
```
make -f ./targets/arduino_101/Makefile.arduino_101 flash
```

#### 6. Serial terminal

Test command line in serial terminal.

```
> help
```
