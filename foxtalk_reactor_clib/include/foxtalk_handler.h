//
// Created by fox on 10/7/24.
//

#ifndef REACTOR_FOXTALK_HANDLER_H
#define REACTOR_FOXTALK_HANDLER_H

#include <iostream>
#include <format>
#include <cassert>
#include <variant>
#include <cstring>
#include <string>
#include <unistd.h>
#include <cstdint>
#include <cmath>

constexpr size_t _foxtalk_ipc_buffer_size = 4096;

///// DEBUGGING HELPER FUNCTIONS ///////////////////////////////////////////////
void dbg_dump_buffer_region(uint8_t *buffer, size_t start, size_t length) {
    std::cout << "      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F        0 1 2 3 4 5 6 7 8 9 A B C D E F";
    auto n_rows = static_cast<size_t>(std::ceil(length / 16.0));

    for (auto row = 0; row < n_rows; row++) {
        std::cout << std::endl << std::format("0x{:02x} ", row * 16);
        for (auto column = 0; column < 16; column++) {
            auto offset = row * 16 + column;
            if (offset > length) {
                std::cout << " - ";
            } else {
                std::cout << std::format("{:02x} ", buffer[start + offset]);
            }
        }

        std::cout << "      ";

        for (auto column = 0; column < 16; column++) {
            auto offset = row * 16 + column;
            if (offset > length) {
                std::cout << " -";
            } else {
                if (buffer[start + offset] >= 0x20 && buffer[start + offset] <= 0x7E) {
                    std::cout << std::format("{: >2c}", buffer[start + offset]);
                } else {
                    std::cout << " .";
                }
            }
        }
    }

    std::cout << std::endl;
}

///// C FFI ////////////////////////////////////////////////////////////////////

extern "C" {
typedef uint32_t foxtalk_size_t;
typedef foxtalk_size_t foxtalk_handler_id_t;

///// MY HANDLER ID /////
foxtalk_handler_id_t _foxtalk_handler_id;

///// RUNTIME COMMUNICATION BUFFER /////
uint8_t _foxtalk_ipc_triple_buffer[_foxtalk_ipc_buffer_size];

///// FROM THE RUNTIME /////
void _foxtalk_register_query(foxtalk_handler_id_t handler_id);
void _foxtalk_next_query_result(foxtalk_handler_id_t handler_id);
void _foxtalk_claim(foxtalk_handler_id_t handler_id);
void _foxtalk_remove(foxtalk_handler_id_t handler_id);

///// USER MUST IMPLEMENT /////
void free_tuple();
void handle();
void teardown();
}

///// C++ API //////////////////////////////////////////////////////////////////

template<typename T>
inline size_t write_t_to_buffer(uint8_t *buffer, size_t index, T what) {
    *((T *) (buffer + index)) = what;
    return sizeof(T) / sizeof(uint8_t);
}

template<typename T>
inline std::pair<T, size_t> read_t_from_buffer(uint8_t *buffer, size_t index) {
    return {
            *((T *) (buffer + index)),
            sizeof(T) / sizeof(uint8_t)
    };
}

struct TripleNoun {
    typedef std::variant<
            std::monostate, // Query
            std::string, // Symbol
            void *, // Cptr
            uint64_t, // U64
            int64_t  // I64
    > NounData;

    TripleNoun(const TripleNoun &) = delete;

    void operator=(const TripleNoun &) = delete;

    TripleNoun(const TripleNoun &&other) noexcept
            : type{other.type}, data{other.data} {}

    void operator=(const TripleNoun &&other) {
        type = other.type;
        data = other.data;
    }

    TripleNoun() : type(NounType::Query), data(std::monostate()) {}

    TripleNoun(const NounData &data) : type(static_cast<NounType>(data.index())), data(data) {
//        std::cout << std::boolalpha
//                  << "ND string? " << std::holds_alternative<std::string>(data)
//                  << " With index " << data.index()
//                  << std::endl;
    }

    bool operator==(const TripleNoun &other) const {
        return type == other.type && data == other.data;
    }

    enum class NounType : uint8_t {
        Query = 0,
        Symbol = 1,
        CPtr = 2,
        U64 = 3,
        I64 = 4,
        MAX
    } type;

    NounData data;

    static std::pair<TripleNoun, size_t> read_from_buffer(uint8_t *buffer, size_t buffer_position) {
        auto start_position = buffer_position;
        auto [type, offset] = read_t_from_buffer<uint8_t>(buffer, buffer_position);
        buffer_position += offset;

//        std::cout << "Reading TripleNoun of type " << (uint32_t)type << std::endl;

        switch ((NounType) type) {
            case NounType::Query:
                return {
                        TripleNoun{std::monostate()},
                        buffer_position - start_position
                };
            case NounType::Symbol: {
                auto [str_length, read_bytes] = read_t_from_buffer<foxtalk_size_t>(buffer, buffer_position);
                buffer_position += read_bytes;
                auto str = std::string((char *) (buffer + buffer_position), str_length);
                return {
                        TripleNoun{str},
                        (buffer_position - start_position) + str_length
                };
            }
            case NounType::CPtr: {
                auto [dat, read_bytes] = read_t_from_buffer<void *>(buffer, buffer_position);
                buffer_position += read_bytes;

                return {
                        TripleNoun{dat},
                        buffer_position - start_position
                };
            }
            case NounType::U64: {
                auto [dat, read_bytes] = read_t_from_buffer<uint64_t>(buffer, buffer_position);
                buffer_position += read_bytes;

                return {
                        TripleNoun{dat},
                        buffer_position - start_position
                };
            }
            case NounType::I64: {
                auto [dat, read_bytes] = read_t_from_buffer<int64_t>(buffer, buffer_position);
                buffer_position += read_bytes;

                return {
                        TripleNoun{dat},
                        buffer_position - start_position
                };
            }
            default:
                throw std::runtime_error("Unknown NounType!");
        }
    }

    size_t write_to_buffer(uint8_t *buffer, size_t buffer_position) {
        size_t current_position = buffer_position;

        current_position += write_t_to_buffer(buffer, current_position, type);

        switch (type) {
            case NounType::Query:
                break;

            case NounType::Symbol: {
                auto sym = std::get<std::string>(data);
                current_position += write_t_to_buffer(buffer, current_position, (foxtalk_size_t) sym.length());
                sym.copy((char *) (buffer + current_position), sym.length());
                current_position += sym.length();
                break;
            }
            case NounType::CPtr:
                current_position += write_t_to_buffer(buffer, current_position, (size_t) std::get<void *>(data));
                break;
            case NounType::U64:
                current_position += write_t_to_buffer(buffer, current_position, std::get<uint64_t>(data));
                break;
            case NounType::I64:
                current_position += write_t_to_buffer(buffer, current_position, std::get<int64_t>(data));
                break;
            default:
                throw std::runtime_error("Unknown NounType!");
        }

        return current_position - buffer_position;
    }
};

struct Triple {
    Triple(const Triple &) = delete;

    void operator=(const Triple &) = delete;

    TripleNoun subject_;
    TripleNoun predicate_;
    TripleNoun object_;

    Triple(const Triple &&other)
            : subject_(std::move(other.subject_)), predicate_(std::move(other.predicate_)),
              object_(std::move(other.object_)) {}

    void operator=(const Triple &&other) {
        subject_ = std::move(other.subject_);
        predicate_ = std::move(other.predicate_);
        object_ = std::move(other.object_);
    }

    Triple(TripleNoun &&subject, TripleNoun &&predicate, TripleNoun &&object)
            : subject_(std::move(subject)),
              predicate_(std::move(predicate)),
              object_(std::move(object)) {}

    bool operator==(const Triple &other) const {
        return subject_ == other.subject_
               && predicate_ == other.predicate_
               && object_ == other.object_;
    }

    void write_to_buffer(uint8_t *buffer, size_t start_position) {
        auto size_bytes = write_t_to_buffer(buffer, start_position, (foxtalk_size_t) 0);
        auto current_position = size_bytes;

        current_position += subject_.write_to_buffer(buffer, current_position);
        current_position += predicate_.write_to_buffer(buffer, current_position);
        current_position += object_.write_to_buffer(buffer, current_position);

        write_t_to_buffer(buffer, start_position, (foxtalk_size_t)(current_position - start_position));
    }

    static std::pair<Triple, foxtalk_size_t> read_from_buffer(uint8_t *buffer, size_t start_position) {
        size_t current_position = start_position;
        auto [triple_size, read_bytes] = read_t_from_buffer<foxtalk_size_t>(buffer, current_position);

        std::cout << std::format("Reading triple with size: {0:d} ({0:#x})", triple_size) << std::endl;
        dbg_dump_buffer_region(buffer, start_position, triple_size + 1);

        current_position += read_bytes;

        std::cout << "Reading subject @ " << std::hex << current_position << std::endl;
        auto [subj, s_read_bytes] = TripleNoun::read_from_buffer(buffer, current_position);
        std::cout << std::format("Read {:d} bytes for subject.", s_read_bytes) << std::endl;
        current_position += s_read_bytes;

        std::cout << "Reading predicate @ " << std::hex << current_position << std::endl;
        auto [pred, p_read_bytes] = TripleNoun::read_from_buffer(buffer, current_position);
        current_position += p_read_bytes;
        std::cout << std::format("Read {:d} bytes for predicate.", p_read_bytes) << std::endl;

        std::cout << "Reading object @ " << std::hex << current_position << std::endl;
        auto [obj, o_read_bytes] = TripleNoun::read_from_buffer(buffer, current_position);
        current_position += o_read_bytes;
        std::cout << std::format("Read {:d} bytes for object.", o_read_bytes) << std::endl;

        assert(current_position == triple_size);

        return std::pair<Triple, size_t>(Triple{std::move(subj), std::move(pred), std::move(obj)}, 0ul);
    }

    void write_to_ipc_buffer() {
        // size to the 0 position
        memset(_foxtalk_ipc_triple_buffer, 0, _foxtalk_ipc_buffer_size);
        write_to_buffer(_foxtalk_ipc_triple_buffer, 0);
    }
};


#endif //REACTOR_FOXTALK_HANDLER_H
