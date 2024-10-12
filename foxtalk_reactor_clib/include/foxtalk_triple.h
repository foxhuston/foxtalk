//
// Created by fox on 10/11/24.
//

#ifndef REACTOR_FOXTALK_TRIPLE_H
#define REACTOR_FOXTALK_TRIPLE_H

#include <iostream>
#include <format>
#include <cassert>
#include <variant>
#include <cstring>
#include <string>
#include <unistd.h>
#include <cstdint>
#include <cmath>
#include "debug_utils.h"

extern "C" {
    typedef uint32_t foxtalk_size_t;
    typedef foxtalk_size_t foxtalk_id_t;
};

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

    static TripleNoun query() { return TripleNoun(); }

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

    //// TO STRING /////
    friend std::ostream &operator<<(std::ostream &os, const TripleNoun &noun) {
        switch (noun.type) {
            case NounType::Query:
                os << "Query";
                break;
            case NounType::Symbol:
                os << std::get<std::string>(noun.data);
                break;
            case NounType::CPtr:
                os << std::get<void*>(noun.data);
                break;
            case NounType::U64:
                os << std::get<uint64_t>(noun.data);
                break;
            case NounType::I64:
                os << std::get<int64_t>(noun.data);
                break;
            case NounType::MAX:
                os << "ERROR! TUPLE_NOUN TYPE WAS `MAX`!";
                break;
        }

        return os;
    }

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

    size_t write_to_buffer(uint8_t *buffer, size_t buffer_position) const {
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
private:

public:
    const TripleNoun subject_;
    const TripleNoun predicate_;
    const TripleNoun object_;

    //// CONSTRUCTORS /////
    Triple(const Triple &) = delete;

    void operator=(const Triple &) = delete;

    Triple(const Triple &&other)
            : subject_(std::move(other.subject_)), predicate_(std::move(other.predicate_)),
              object_(std::move(other.object_)) {}

    Triple(TripleNoun &&subject, TripleNoun &&predicate, TripleNoun &&object)
            : subject_(std::move(subject)),
              predicate_(std::move(predicate)),
              object_(std::move(object)) {}

    bool operator==(const Triple &other) const {
        return subject_ == other.subject_
               && predicate_ == other.predicate_
               && object_ == other.object_;
    }

    //// TO STRING /////
    friend std::ostream &operator<<(std::ostream &os, const Triple &triple) {
        os << "<" << triple.subject_ << ", " << triple.predicate_ << ", " << triple.object_ << ">";
        return os;
    }

    //// ACCESSORS /////
    template<typename T>
    std::optional<const T> get_subject() {
        if(std::holds_alternative<T>(subject_.data)) {
            return { std::get<T>(subject_.data) };
        } else {
            std::cout << "WARNING: subject was not a " << typeid(T).name()
                      << " (It had the foxtalk type id " << (size_t)subject_.type << ")"
                      << std::endl;

            return std::nullopt;
        }
    }

    template<typename T>
    std::optional<const T> get_predicate() {
        if(std::holds_alternative<T>(predicate_.data)) {
            return std::get<T>(predicate_.data);
        } else {
            std::cout << "WARNING: predicate was not a " << typeid(T).name()
                      << " (It had the foxtalk type id " << (size_t)predicate_.type << ")"
                      << std::endl;

            return std::nullopt;
        }
    }

    template<typename T>
    std::optional<const T> get_object() {
        if(std::holds_alternative<T>(object_.data)) {
            return std::get<T>(object_.data);
        } else {
            std::cout << "WARNING: object was not a " << typeid(T).name()
                      << " (It had the foxtalk type id " << (size_t)object_.type << ")"
                      << std::endl;

            return std::nullopt;
        }
    }

    //// SERIALIZATION / DESERIALIZATION /////
    void write_to_buffer(uint8_t *buffer, size_t start_position) const {
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
};

#endif //REACTOR_FOXTALK_TRIPLE_H
