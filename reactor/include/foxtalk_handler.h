//
// Created by fox on 10/7/24.
//

#ifndef REACTOR_FOXTALK_HANDLER_H
#define REACTOR_FOXTALK_HANDLER_H

#include <variant>
#include <cstring>
#include <string>
#include <unistd.h>
#include <cstdint>

constexpr size_t __foxtalk_ipc_buffer_size = 4096;

///// C FFI ////////////////////////////////////////////////////////////////////

extern "C" {
///// MY HANDLER ID /////
size_t handler_id;

///// RUNTIME COMMUNICATION BUFFER /////
uint8_t __foxtalk_ipc_triple_buffer[__foxtalk_ipc_buffer_size];

///// FROM THE RUNTIME /////
void register_query(size_t handler_id);
void next_query_result(size_t handler_id);

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

    TripleNoun(const TripleNoun &&other)
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

        switch ((NounType)type) {
            case NounType::Query:
                return {
                        TripleNoun{std::monostate()},
                        buffer_position - start_position
                };
            case NounType::Symbol: {
                auto [str_length, read_bytes] = read_t_from_buffer<size_t>(buffer, buffer_position);
                buffer_position += read_bytes;
//                std::cout << "Going to read " << str_length << " bytes for string..." << std::endl;

                auto str = std::string((char *) (buffer + buffer_position), str_length);

//                std::cout << "Got string: " << str << std::endl;

                return {
                        TripleNoun{str},
                        read_bytes + str_length
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
                current_position += write_t_to_buffer(buffer, current_position, sym.length());
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

    Triple(const TripleNoun &&subject, const TripleNoun &&predicate, const TripleNoun &&object)
            : subject_(std::move(subject)),
              predicate_(std::move(predicate)),
              object_(std::move(object)) {}

    void write_to_buffer(uint8_t *buffer) {
        size_t buffer_position = sizeof(size_t) / sizeof(uint8_t);

        buffer_position += subject_.write_to_buffer(buffer, buffer_position);
        buffer_position += predicate_.write_to_buffer(buffer, buffer_position);
        buffer_position += object_.write_to_buffer(buffer, buffer_position);

        write_t_to_buffer(buffer, 0, buffer_position);
    }

    void write_to_ipc_buffer() {
        // size to the 0 position
        memset(__foxtalk_ipc_triple_buffer, 0, __foxtalk_ipc_buffer_size);
        write_to_buffer(__foxtalk_ipc_triple_buffer);
    }
};


#endif //REACTOR_FOXTALK_HANDLER_H
