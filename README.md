# syncwav

A high-performance, singlesource-to-multisource audio routing application built in C++. 

> **⚠️ Status: Active Development (Alpha)** > This project is currently in active development. The architecture is being refined, and core features are actively being built. Expect bugs, missing features, and breaking changes. 

## Overview

`syncwav` is designed to handle complex audio routing scenarios, allowing users to map single audio input sources to multiple output destinations efficiently. It focuses on low latency and robust system-level audio management.

*(Note: This is the active repository. The older architecture previously housed under the name `SyncWave` has been deprecated in favor of this rebuilt routing system.)*

## ✨ Planned Features
* **Multisource Routing:** Map any input to any output seamlessly.
* **Low-Level Performance:** Built in C++ for minimal overhead and latency.
* **[Add another specific feature, e.g., TUI interface, specific audio framework support like PipeWire/PulseAudio, etc.]**

## 🛠️ Building from Source

*Note: Build instructions will be formalized as the architecture stabilizes.*

### Prerequisites
* C++ compiler (GCC/Clang)
* [Add your build system, e.g., CMake >= 3.10, Make]
* [Add your audio dependencies, e.g., PipeWire development headers, ALSA, etc.]

### Quick Start
```bash
# Clone the repository
git clone https://github.com/CodeWithDevesh/syncwav.git
cd syncwav
mkdir build && cd build
cmake ..
make