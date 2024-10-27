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

## ammy's Notes For Running This On A Macbook
i'm basically just using devcontainers in vscode
- `.devcontainer/devcontainer.json` is a tiny wrapper that slurps up `.devcontainer/Dockerfile`
- so you should probably install Docker and the vscode dev container extension
- i have not tested this anywhere else yet
