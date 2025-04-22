# mymuduo Network Library  

This repository is a re-implementation of the [muduo Network Library](https://github.com/chenshuo/muduo), a high-performance C++ network library based on the Reactor pattern. Compared to the original muduo, this version removes redundant assertions and adopts modern C++ syntax while maintaining a cleaner code style.

**Compatibility**: Linux only (tested on Ubuntu Server 24.04).  
**Status**: Actively under development and debugging. Contributions and discussions are welcome.

---

## Project Structure

- `build/`: Generated binaries  
- `src/`: Source code  
- `include/`: Header files  
- `test/`: Test cases  
- `examples/`: Example programs  

---

## Features
- Simplified implementation focusing on core Reactor logic  
- Modern C++ syntax (C++11/14/17 features where applicable)  
- Removed non-essential assertions from original implementation  
- Improved code readability and consistency  

---

## Quick Start

```bash
# Build the project
./build.sh