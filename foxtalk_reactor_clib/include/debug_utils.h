//
// Created by fox on 10/11/24.
//

#ifndef REACTOR_DEBUG_UTILS_H
#define REACTOR_DEBUG_UTILS_H

#include <cstdint>
// ReSharper disable once CppUnusedIncludeDirective
#include <cstdlib>

void dbg_dump_buffer_region(uint8_t *buffer, size_t start, size_t length);

#endif //REACTOR_DEBUG_UTILS_H
