//
// Created by fox on 10/7/24.
//

#ifndef REACTOR_FOXTALK_HANDLER_H
#define REACTOR_FOXTALK_HANDLER_H

#include <cassert>
#include <cstring>
#include <string>
#include <unistd.h>
#include <cstdint>

constexpr size_t buffer_size = 4096;
constexpr size_t size_offset = sizeof(size_t) / sizeof(uint8_t);

extern "C" {
size_t handler_id;
uint8_t ipc_triple_buffer[buffer_size];
}

template<typename T>
inline size_t write_t_to_buffer(size_t index, T what) {
    *((T*)(ipc_triple_buffer + index)) = what;
    return sizeof(T) / sizeof(uint8_t);
}

extern "C" {

void register_query(size_t handler_id);
void free_tuple();

};

struct TripleNoun {
    TripleNoun(const TripleNoun &) = delete;

    void operator=(const TripleNoun &) = delete;

    TripleNoun(const TripleNoun &&other)
            : type{other.type}, data{other.data} {}

    void operator=(const TripleNoun &&other) {
        type = other.type;
        data = other.data;
    }

    enum class NounType : uint8_t {
        Query = 0,
        Symbol = 1,
        CPtr = 2,
        U64 = 3,
        I64 = 4,
        MAX
    } type;

    union NounData {
        char *symbol;
        void *cptr;
        uint64_t u64;
        int64_t i64;
    } data;

    TripleNoun(NounType type, const NounData &data) : type(type), data(data) {}

    size_t write_to_buffer(size_t buffer_position) {
        // size to the 0 position
        size_t current_position = buffer_position;

        ipc_triple_buffer[current_position] = static_cast<uint8_t>(type);
        current_position++;

        switch(type) {
            case NounType::Query:
                break;
            case NounType::Symbol: {
                auto starting_position = current_position;
                current_position += size_offset;

                char *ptr = data.symbol;
                while(*ptr != 0) {
                    ipc_triple_buffer[current_position] = *ptr;
                    ptr++;
                    current_position++;
                }

                write_t_to_buffer(starting_position, current_position - starting_position + size_offset);
                break;
            }
            case NounType::CPtr:
                current_position += write_t_to_buffer(current_position, (size_t)data.cptr);
                break;
            case NounType::U64:
                std::cout << "Writing U64 " << data.u64 << " at pos " << current_position << std::endl;
                current_position += write_t_to_buffer(current_position, data.u64);
                break;
            case NounType::I64:
                ((int64_t *)ipc_triple_buffer)[current_position] = data.i64;
                current_position += sizeof(int64_t) / sizeof(uint8_t);
                break;
            default:
                throw std::runtime_error("Unknown NounType!");
        }

        assert(current_position < buffer_size);
        return current_position - buffer_position;
    }
};

struct Triple {
    Triple(const Triple &) = delete;

    void operator=(const Triple &) = delete;

    TripleNoun subject_;
    TripleNoun predicate_;
    TripleNoun object_;

    Triple(const TripleNoun &&subject, const TripleNoun &&predicate, const TripleNoun &&object)
            : subject_(std::move(subject)),
              predicate_(std::move(predicate)),
              object_(std::move(object)) {}

    void write_to_buffer() {
        // size to the 0 position
        memset(ipc_triple_buffer, 0, buffer_size);

        size_t buffer_position = sizeof (size_t) / sizeof (uint8_t);
        buffer_position += subject_.write_to_buffer(buffer_position);
        buffer_position += predicate_.write_to_buffer(buffer_position);
        buffer_position += object_.write_to_buffer(buffer_position);

        write_t_to_buffer(0, buffer_position);
    }
};


#endif //REACTOR_FOXTALK_HANDLER_H
