# Foxtalk Handler playground

Write C++ code, have it automatically loaded into Foxtalk, all with intellisense.

# How to use

1. Navigate to `../foxtalk_rust_binary`
2. Create a `.env` file (See .env section for example)
3. Run `cargo run`
4. In this folder (`foxtalk_handlers`), copy the example handler and paste it either in the root, or in any nested folder except `build`.
5. Every cpp file will be treated as a separate handler for Foxtalk. The `foxtalk_rust_binary` will watch for changes in this folder, and automatitally build it, include the necessary Foxtalk SDK lib files, and then update the `build/compile_commands.json` file so that you will have intellisense while working in these new, small, one-off files.

## .env example file

```
SO_PATH=/home/lexi/work/foxtalk/foxtalk_handlers/build
CPP_PATH=/home/lexi/work/foxtalk/foxtalk_handlers
HANDLER_INCLUDE_PATH=/home/lexi/work/foxtalk/foxtalk_cpp_handler/include
```
