# FauxCombinator

C++ implementation of FauxCombinator.

Uses CMake and Conan.

Commands to run:
```
# Install deps via conan
$ conan install . --output-folder=build --build=missing
$ cd build
$ cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
$ cmake --build
```

Make SURE your `-DCMAKE_BUILD_TYPE` and Conan profile build type match, otherwise nothing works for no reason.
