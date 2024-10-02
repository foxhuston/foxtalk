#!/usr/bin/env bash

clang++ -I ../vendor/gc-8.2.4/include -I ../lib -shared -fPIC ReactorTestHandler.cpp -o libreactor_test_handler.so