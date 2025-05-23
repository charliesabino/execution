# Minimal std::execution

## Overview

Written as the final project for Advanced C++ (Spring 2025) by Charlie Sabino.

## Features

- Header-only, minimal implementation of the Senders & Receivers proposal (P2300)
- `just`: create a sender that immediately sends values
- `then`: transform values from a sender with a callable
- `inline_scheduler`: schedule tasks on the calling thread
- `thread_pool`: multi-threaded scheduling with a thread pool
- Pipe syntax (`|`) for chaining senders and operations

## Requirements

- C++23-compatible compiler (e.g., GCC 11+, Clang 14+, MSVC 2019+)
- CMake 3.20 or higher
- Thread support via `<thread>`, `<mutex>`, etc.

## Building

Generate build files and compile the library and examples:

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

This produces:

- `libmyexecution.a`: the static library
- `exec_examples`: example executable

## Usage

Include the headers and link against `myexecution` in your CMake project:

```cmake
add_subdirectory(path/to/myexecution)
target_link_libraries(your_target PRIVATE myexecution)
```

In your source code:

```c++
#include <myexecution/myexecution.hpp>
using namespace execution;

inline_scheduler sched;
auto sender = sched.schedule() | then([] { return 42; });
sender.connect(your_receiver{}).start();

```

### Running the Quickstart Example

From the `build` directory:

```bash
./exec_examples
thread 0x...: hello from inline
thread 0x...: 42
```

## Project Structure

- `include/myexecution/`: Public headers
  - `concepts.hpp` Core concepts (`sender`, `receiver`, etc.)
  - `myexecution.hpp` Common entry point header
  - `schedulers/` Inline and thread-pool schedulers
  - `sender/` `just` and `then` implementations
- `src/`: Library source stubs
- `examples/`: Example applications
- `CMakeLists.txt`: Build configuration
