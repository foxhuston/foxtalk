# First time setup

- Clone repo
- `mkdir build`
- Install dependencies found in `PKGBUILD`
  - At time of writing, `sudo pacman -S stb glm glfw vulkan-headers vulkan-extra-layers vulkan-tools argparse podofo`
- `cd build`
- `cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug ..`

# Building

    $ cd build
    $ cmake -GNinja ..
    $ ninja
