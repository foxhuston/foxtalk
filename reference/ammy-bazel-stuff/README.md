## Meow~

Ser asked that i document anything Bazel-shaped, so this is that documentation. :3

All of this lives in a dev container running Ubuntu 24.04. All of its configuration is in `.devcontainer/Dockerfile` at the top level. Running these is well-documented by people who are not me, but i just used the [official VSCode docs](https://code.visualstudio.com/docs/devcontainers/create-dev-container).

The dev container has Bazel 7 installed, and, at time of writing, there are roughly three things you can do:
- `bazel build binary_example`, which compiles `example.cxx` and spits it out into `bazel-bin/binary_example`
- `bazel build static_binary_example`, which does the same but generates `bazel-bin/shared_binary_example.so` (which is probably important for Handler Things™)
- `bazel build ...`, which, for now, does both of the above but is actually just shorthand for "build all the things in `BUILD.bazel`"

Now kitten can write handlers, wheeeeee~