# Foxtalk Binary

This is a binary that runs the Reactor in a loop and listens for new `.so` files to add as handlers. 

For more info on how to use this and get quickly started writing Foxtalk handlers, see the `foxtalk_handlers` folder.

## TODO

This works for the most part, but some things are either missing or buggy:

1. Deleting a `.cpp` file does not remove the handler from the Reactor. This SHOULD be working, but there's a bug somewhere.
2. The watcher for cpp files actually DOES look in the build folder. The build folder should be ignored.
3. In order to not drain the cpu during development, there's a thread sleep between ticks. This should be behind some dev flag. 