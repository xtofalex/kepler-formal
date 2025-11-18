# kepler-formal

## Introduction

Kepler-Formal is a logic equivalence checking (LEC) tool that operates on the naja interchange format(https://github.com/najaeda/naja-if). It is designed for verifying incremental edits produced by the najaeda Python library(https://pypi.org/project/najaeda/) or any workflow that preserves stable indices across modifications so that corresponding items retain the same identifiers. Kepler-Formal focuses today on combinational equivalence checking only — sequential boundary changes are not supported yet and remain planned work.

### Acknowledgement

[<img src="https://nlnet.nl/logo/banner.png" width=100>](https://nlnet.nl/project/Naja)
[<img src="https://nlnet.nl/image/logos/NGI0Entrust_tag.svg" width=100>](https://nlnet.nl/project/Naja)

This project is supported and funded by NLNet through the [NGI0 Entrust](https://nlnet.nl/entrust) Fund.

## Build

```bash
git clone --recurse-submodules https://github.com/keplertech/kepler-formal.git
cd kepler-formal
mkdir build
cd build
cmake ..
make
```

## Usage

```bash
"build/src/bin/kepler_formal <naja-if-dir-1> <naja-if-dir-2> [<liberty-file>...]"
```

## Contact

contact@keplertech.io

#### Dependencies

Mandatory dependencies:

1. Boost
2. [cmake](https://cmake.org): at least 3.22 version.
For system-specific cmake installation options, please refer to [this link](https://cmake.org/download/).
3. Python3: for building the SNL Python3 interface. This interface is used to load primitive cells (associated to Verilog parsing)
and their associated characteristics (for instance: ressource count, timing characteristics, ...).

Optional dependencies:

1. [Doxygen](https://www.doxygen.nl): for the documentation generation.

Embedded dependencies, through git sub modules:

1. [naja-verilog](https://github.com/najaeda/naja-verilog): for verilog parsing.
2. [google test](https://github.com/google/googletest) for unit testing.

On Ubuntu:

```bash
sudo apt-get install g++ libboost-dev python3.9-dev capnproto libcapnp-dev libtbb-dev pkg-config bison flex doxygen
```

Using [nix-shell](https://nixos.wiki/wiki/Development_environment_with_nix-shell):

```bash
nix-shell -p cmake boost python3 doxygen capnproto bison flex pkg-config tbb_2021_8
```

On macOS, using [Homebrew](https://brew.sh/):

```bash
brew install cmake doxygen capnp tbb bison flex boost
```

Ensure the versions of `bison` and `flex` installed via Homebrew take precedence over the macOS defaults by modifying your $PATH environment variable as follows:

```bash
export PATH="/opt/homebrew/opt/flex/bin:/opt/homebrew/opt/bison/bin:$PATH"
```

#### Building and Installing

```bash
#First define an env variable that points to the directory where you want naja to be installed:
export NAJA_INSTALL=<path_to_installation_dir>
# Create a build dir and go inside it
mkdir build
cd build
cmake <path_to_naja_sources_dir> -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$NAJA_INSTALL
#For instance: cmake ~/srcs/naja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$NAJA_INSTALL
make
make test
make install
```

#### Building and Installing Documentation

```bash
#make sure that doxygen was available when launching the cmake command
cd build
make docs
make install
```

Documentation will be installed in $NAJA_INSTALL/doc directory. Starting file to open in browser is: $NAJA_INSTALL/doc/html/index.html.

<div align="right">[ <a href="#Introduction">↑ Back to top ↑</a> ]</div>

---

### Use

#### Environment

After building and installing, start by setting up a runtime environment.

```bash
export NAJA_INSTALL=<path_to_installation_dir>
#For Naja python interface and in particular primitives loading
export PYTHONPATH=$PYTHONPATH:$NAJA_INSTALL/lib/python
```

