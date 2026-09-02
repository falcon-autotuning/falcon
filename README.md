# Falcon Lib

**Falcon Lib** is a comprehensive C++ and DSL-based framework for quantum device autotuning. It bridges measurement requests from the Falcon core system with physical hardware instruments, providing a complete suite of libraries, tools, and examples for building robust quantum device control workflows.

## What's Inside

Falcon Lib is organized into several key components:

- **DSL (Domain-Specific Language)**: A high-level language for defining quantum device autotuning state machines
- **Database**: C++ PostgreSQL backend for storing device characteristics
- **Communications (comms)**: NATS-based messaging infrastructure for device control
- **QArray Device Module**: Specialized support for quantum dot array devices
- **Core Libraries**: Collections, math utilities, physics models, and device structures
- **Typing System**: Runtime type system and FFI (Foreign Function Interface) support

## Quick Start

Currently this application only works on Linux. This may work on windows with WSL2.
Windows support is currently deprecated, but much of the structures for support are already in place.

See the [documentation](https://falcon-autotuning.github.io/falcon-lib/).

### **COMING BACK ONLINE SOON** For Users: Run the Demo

```bash
cd demos/qarray-charge-tuning
make docker-up
falcon-test ./tests/run_tests.fal --log-level info
```

[Try the Demo on GitHub](https://github.com/falcon-autotuning/falcon-lib/tree/main/demos/qarray-charge-tuning)

### For Users: Install the latest release

To install this onto your system, please run one of the following commands where you replace **<VERSION>** with the version that you want to install:

```bash
curl -fsSL https://github.com/falcon-autotuning/falcon/releases/download/v<VERSION>/install.sh | sudo bash
```

### For Algorithm/Backend Developers: Build from Source

To develop analysis and other runtime features, you must build FAlCon from source.
**Ubuntu 24.04** is the recommended platform for development.
The rest of this install assumes [Ubuntu](https://releases.ubuntu.com/noble/).
This install mirrors the install of the real docker container used for release.
It installs the packages to `/opt/falcon`, and is why **sudo** is required.
For the versions of the packages, please see the ubuntu release for the ones tested.

#### Prerequisites

Vcpkg is our package manager and must be installed and available in your path to successfully build.
See [vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-bash) for Microsoft's tutorial on installing vcpkg.

#### Build Steps on Ubuntu 24.04

```bash
apt update && apt install -y build-essential cmake bison flex pkg-config git curl zip unzip tar linux-libc-dev autoconf autoconf-archive automake libtool ninja-build lld coreutils gfortran mono-complete clang llvm

sudo make install PRESET=linux-clang-release
```

To find the packages on your system after the install two global variables need to be updated.
This will set for your immediate shell session.
To make this permanent, add these to your `~/.bashrc` or `~/.zshrc` file.

```bash
export PATH="/opt/falcon/bin:$PATH"
export PKG_CONFIG_PATH="/opt/falcon/lib/pkgconfig:$PKG_CONFIG_PATH"
```

And that is it! **falcon-run** and **falcon-test** should now be available in your path. Any questions reach out to the main developers or open an issue on GitHub.

## Main Features

✨ Quantum Device Autotuning: Define complex control workflows as declarative state machines

🔌 Hardware Integration: FFI support for binding C++ measurement functions to .fal autotuners

📊 Device Database: PostgreSQL-backed storage for device characteristics and global tuning parameters

🌐 NATS Messaging: Distributed communication for multi-device systems

🧪 Built-in Testing: Test runner with setup/teardown fixtures and detailed diagnostics

📚 Language Server: IDE support via LSP for .fal files (Neovim)

## Contributing

We welcome contributions! Please refer to our contributing guidelines in the documentation.

## License

MPL-2.0
