# FAlCon

**FAlCon** is an open-source framework for quantum device autotuning, measurement automation, and experimental orchestration.

Built around the FAlCon DSL and a modular runtime architecture, FAlCon enables researchers to develop reproducible autotuning workflows that integrate directly with laboratory instrumentation, data acquisition systems, simulation environments, and quantum device control software.

> 🚀 **Latest Release:** [FAlCon v2.0.0](https://github.com/falcon-autotuning/falcon/releases/tag/v2.0.0)

## Documentation

Full documentation is available at:

https://falcon-autotuning.github.io/falcon-lib/

---

## Quick Start

### Install FAlCon v2.0.0

Currently, FAlCon is supported on Linux.

Install the latest release with:

```bash
curl -fsSL https://github.com/falcon-autotuning/falcon/releases/download/v2.0.0/install.sh | sudo bash
```

After installation, the primary executables should be available:

falcon-run
falcon-test

### Verify Your Installation

FAlCon v2.0.0 includes a built-in wellness validation tool that verifies your installation and runtime environment.

Run:
```bash
/opt/falcon/wellness/wellness-check.sh /opt/falcon/wellness/
```

The wellness checker validates:

FAlCon installation integrity
Runtime dependencies
Shared library discovery
Package configuration
DSL tooling availability
Executable availability
General environment health

This is the recommended first step after installation and whenever diagnosing environment issues.

---

## Core Components
### FAlCon DSL

A domain-specific language for defining measurement procedures, autotuning algorithms, state machines, and experimental workflows.

### Runtime & Execution Engine

The execution environment responsible for running FAlCon workflows against simulation backends or physical hardware.

### Instrument Integration

Interfaces and communication infrastructure that connect FAlCon workflows to laboratory instrumentation.

### Testing & Simulation

Frameworks for developing and validating workflows before deployment on physical devices.

---

## Main Features

### ✨ Quantum Device Autotuning
 Develop complex tuning procedures using a high-level DSL and reusable workflows.

### 🔌 Hardware Integration
 Connect experimental hardware and measurement systems through modular instrument interfaces.

### 📊 Data Management
 Store and manage characterization data, device metadata, and experimental results.

### 🧪 Built-In Testing
 Develop and validate workflows using automated testing infrastructure.

### 🧬 Quantum Device Models
 Leverage reusable models and abstractions for quantum-dot devices and related systems.

### 📚 Language Tooling
 IDE tooling and language-server support for developing FAlCon workflows.

### ⚙️ Simulation Support
 Test workflows and algorithms before deploying them to laboratory hardware.

---

## For Algorithm and Backend Developers

Building from source is recommended for developers extending the runtime, language tooling, simulation environment, or backend infrastructure.

Ubuntu 24.04 is the recommended development platform.

FAlCon installs into /opt/falcon and therefore requires administrator privileges during installation.

### Prerequisites

FAlCon uses vcpkg for package management.

Install and configure vcpkg before attempting to build:

https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-bash

#### Build Dependencies
```bash
sudo apt update

sudo apt install -y \
    build-essential \
    cmake \
    bison \
    flex \
    pkg-config \
    git \
    curl \
    zip \
    unzip \
    tar \
    linux-libc-dev \
    autoconf \
    autoconf-archive \
    automake \
    libtool \
    ninja-build \
    lld \
    coreutils \
    gfortran \
    mono-complete \
    clang \
    llvm
```

Build and Install
```bash
sudo make install PRESET=linux-clang-release
```

Environment Configuration

For the current shell:

```bash
export PATH="/opt/falcon/bin:$PATH"
export PKG_CONFIG_PATH="/opt/falcon/lib/pkgconfig:$PKG_CONFIG_PATH"
```

To make these settings permanent, add them to:

~/.bashrc


or

~/.zshrc

Contributing

Contributions are welcome.

Areas where contributions are particularly valuable include:

Autotuning algorithms
Instrument integrations
Documentation
Testing infrastructure
Simulation environments
User interfaces and visualization
Language tooling and developer experience

Please open an issue or pull request to get involved.

License

MPL-2.0
