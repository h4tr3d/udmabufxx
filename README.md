UDMABUFXX
=========

C++ interface for [udmabuf](https://github.com/ikwzm/udmabuf) kernel module.

## Build

```bash
mkdir build
cd build
cmake ..
cmake --build
ctest -v
```

## Link to project

1. CMake-based project: as git submodule and connect via `add_subdirectory()`. Link with `udmabufxx::udmabuxx` or with `udmabufxx::udmabuxx_static`
2. CMake-based project (**untested**): system-wide installation via `find_package(Udmabufxx)`. Linkx with `udmabufxx::udmabufxx`.
3. Generic project: via `pkg-config --cflags --libs udmabufxx`

## Usage

```c++
udmabuf buf{"udmabuf0"};
auto ptr = reinterpret_cast<uint8_t*>(buf.get());
// some action from HW
// ...
buf.sync_to_cpu(0, buf.size()); // un-needed on x86_64
// access to `ptr`
```

For supported platforms see [Supported platforms](https://github.com/ikwzm/udmabuf#supported-platforms).
