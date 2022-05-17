# WAIT: Bringing WebAssembly to Resource-constrained IoT Devices

WAIT is a lightweight WebAssembly AOT compiler for resource-constrained IoT devices.

WAIT is conditionally accepted by ACM MobiSys'22, see [MobiSys homepage](https://www.sigmobile.org/mobisys/2022/) for more details.

## Introduction
> we propose WAIT, a lightweight WebAssembly runtime on resource-constrained IoT devices for device-cloud integrated applications. WAIT is the first work to enable the Ahead-of-Time (AOT) compilation of WebAssembly on resource-constrained devices by leveraging several approaches to reduce memory usage.

## Usage

We implemented WAIT for AVR ATMega128 MCU.
To facilitate easy build and test, we present a simulated execution environment for ATMega128 MCU (based on [Avrora simulation framework](http://compilers.cs.ucla.edu/avrora/)) along with our WAIT's implementationin this repository.

### Preparation
1. We suggest you use Ubuntu 20.04 LTS to run the simulation of WAIT.
2. Clone this repo
3. Install dependencies for building WAIT:
```bash
apt install git build-essential wget curl unzip gcc-multilib xxd cmake wabt python3
apt install gcc-avr avr-libc avrdude gdb-avr openjdk-11-jdk
```

### Compile the WebAssembly module
1. First, download the WASI SDK to compile `*.c` to WebAssembly.
Go into `libs` folder, and
```bash
wget https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-12/wasi-sdk-12.0-linux.tar.gz
tar xvf wasi-sdk-12.0-linux.tar.gz 
```
The WASI SDK should reside in `libs/wasi-sdk-12.0` folder.

2. 
Modifiy the `CMakeLists.txt` in `example` folder, and
```bash
./compile.sh
```

3. (Optional) 
python 3.7
```bash
pip3 install wasmer wasmer_compiler_cranelift
```

```bash
python3.7 instance.py helloworld
```

### Run WAIT with avrora

1. Compile WAIT
In root
```bash
rm -rf build
mkdir build&&cd build
cmake ..
make
```

2. Run WAIT with avrora
```bash
bash third-party/runwait.sh
```

You can see the avrora output.

## TODOs
- [ ] Code comments in the `src`
- [ ] Tree layout of this folder
- [ ] Compiling WASM do not generate the native benchmark
- [ ] Complete gitignore
- [ ] wasmer test for helloworld
- [ ] (maybe) docker env?
- [ ] finalize this readme
- [ ] stub.txt what for?
- [ ] abstract the modifications scattered in the code (e.g., example/Cmakelist, src/main.c)
- [ ] what wasmbench.sh for?