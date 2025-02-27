# FauxCombinator

C++ implementation of FauxCombinator.

Uses CMake and Conan.

Commands to run:
```
# Install deps via conan
$ conan install . --output-folder=build --build=missing -s build_type=Release
$ cmake --preset conan-release
$ cd build
$ make
```

Make SURE your `-DCMAKE_BUILD_TYPE` and Conan profile build type match, otherwise nothing works for no reason.
