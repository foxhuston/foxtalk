# Building

  $ conan install . --output-folder=build --build=missing
  $ cd build
  $ cmake .. -GNinja -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
  $ ninja
