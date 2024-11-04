//
// Created by fox on 10/11/24.
//

#ifndef REACTOR_FOXTALK_TUPLE_H
#define REACTOR_FOXTALK_TUPLE_H

#include <vector>
#include <iostream>
#include <format>
#include <cassert>
#include <variant>
#include <string>
#include <cstdint>
#include <cmath>
#include <cstring>

using namespace std::literals;

#include "debug_utils.h"

extern "C"
{
    typedef uint32_t foxtalk_size_t;
    typedef foxtalk_size_t foxtalk_id_t;
};

template <typename T>
inline size_t write_t_to_buffer(uint8_t *buffer, size_t index, T what)
{
    *((T *)(buffer + index)) = what;
    return sizeof(T) / sizeof(uint8_t);
}

template <typename T>
inline std::pair<T, size_t> read_t_from_buffer(const uint8_t *buffer, size_t index)
{
    return {
        *((T *)(buffer + index)),
        sizeof(T) / sizeof(uint8_t)};
}

template <typename T>
inline size_t write_tuple_noun_vec_to_buffer(uint8_t *buffer, size_t start_position, const std::vector<T> &vec)
{
    auto count_bytes = write_t_to_buffer<foxtalk_size_t>(buffer, start_position, (foxtalk_size_t)vec.size());
    auto current_position = start_position + count_bytes;

    for (auto n : vec)
    {
        current_position += n.write_to_buffer(buffer, current_position);
    }

    return current_position - start_position;
}

template <typename T>
inline std::pair<std::vector<T>, size_t> read_tuple_noun_vec_from_buffer(uint8_t *buffer, size_t start_position)
{
    size_t current_position = start_position;
    auto [vec_size, read_bytes] = read_t_from_buffer<foxtalk_size_t>(buffer, current_position);
    current_position += read_bytes;

    //    std::cout << std::format("Reading vector of size: {0:d} ({0:#x})", vec_size) << std::endl;

    std::vector<T> out{};
    for (int i = 0; i < vec_size; i++)
    {
        auto [noun, s_read_bytes] = T::read_from_buffer(buffer, current_position);
        current_position += s_read_bytes;

        out.push_back(noun);
    }

    return std::pair<std::vector<T>, size_t>(out, current_position - start_position);
}

struct TupleNoun
{
    typedef std::variant<
        std::monostate, // Query
        std::string,    // Symbol
        void *,         // Cptr
        uint64_t,       // U64
        int64_t,        // I64
        std::vector<uint8_t> // Bytes
        >
        NounData;

    // TODO: During performance testing, see if we're copying triples and triplenouns like crazy
    // TripleNoun(const TripleNoun &) = delete;
    // void operator=(const TripleNoun &) = delete;
    //
    // TripleNoun(const TripleNoun &&other) noexcept
    //         : type{other.type}, data{other.data} {}

    // TripleNoun& operator=(TripleNoun &&other) noexcept {
    //     type = other.type;
    //     data = other.data;
    //     return *this;
    // }

    enum class NounType : uint8_t
    {
        Query = 0,
        Symbol = 1,
        CPtr = 2,
        U64 = 3,
        I64 = 4,
        Bytes = 5,
        Prefix = 6,
        MAX
    };

    TupleNoun(TupleNoun::NounType t) : type(t), data(std::monostate()) {}

    static TupleNoun query() { return TupleNoun{NounType::Query}; }
    static TupleNoun prefix() { return TupleNoun{NounType::Prefix}; }

    // static TupleNoun prefix() { return TupleNoun{ NounType::Prefix, std::monostate() }; }


    template<typename T>
    static TupleNoun from_struct(T t)
    {
        std::vector<uint8_t> bytes(sizeof(T));
        memcpy(bytes.data(), &t, sizeof(T));
        return { bytes };
    }

    template<typename T>
    std::optional<T> into_struct() const
    {
        if (std::holds_alternative<std::vector<uint8_t>>(data))
        {
            T out {};
            memcpy(&out, std::get<std::vector<uint8_t>>(data).data(), sizeof(T));
            return out;
        }

        return std::nullopt;
    }

    TupleNoun(const NounData &data) : type(static_cast<NounType>(data.index())), data(data)
    {
        //        std::cout << std::boolalpha
        //                  << "ND string? " << std::holds_alternative<std::string>(data)
        //                  << " With index " << data.index()
        //                  << std::endl;
    }

    bool operator==(const TupleNoun &other) const
    {
        return type == other.type && data == other.data;
    }

    NounType type;

    NounData data;

    //// TO STRING /////
    friend std::ostream &operator<<(std::ostream &os, const TupleNoun &noun)
    {
        switch (noun.type)
        {
        case NounType::Query:
            os << "Query";
            break;
        case NounType::Symbol:
            os << std::get<std::string>(noun.data);
            break;
        case NounType::CPtr:
            os << std::get<void *>(noun.data);
            break;
        case NounType::U64:
            os << std::get<uint64_t>(noun.data);
            break;
        case NounType::I64:
            os << std::get<int64_t>(noun.data);
            break;
        case NounType::Bytes:
            os << "Bytes[" << std::get<std::vector<uint8_t>>(noun.data).size() << "]";
            break;
        case NounType::Prefix:
            os << "Prefix";
            break;
        case NounType::MAX:
            os << "ERROR! TUPLE_NOUN TYPE WAS `MAX`!";
            break;
        }

        return os;
    }

    static std::pair<TupleNoun, size_t> read_from_buffer(uint8_t *buffer, size_t buffer_position)
    {
        auto start_position = buffer_position;
        auto [type, offset] = read_t_from_buffer<uint8_t>(buffer, buffer_position);
        buffer_position += offset;

        //        std::cout << "Reading TripleNoun of type " << (uint32_t)type << std::endl;

        switch (static_cast<NounType>(type))
        {
        case NounType::Query:
            return {
                TupleNoun{std::monostate()},
                buffer_position - start_position};
        case NounType::Prefix:
            return {
                TupleNoun{std::monostate()},
                buffer_position - start_position};
        case NounType::Symbol:
        {
            auto [str_length, read_bytes] = read_t_from_buffer<foxtalk_size_t>(buffer, buffer_position);
            buffer_position += read_bytes;
            auto str = std::string((char *)(buffer + buffer_position), str_length);
            return {
                TupleNoun{str},
                (buffer_position - start_position) + str_length};
        }
        case NounType::Bytes:
            {
                auto [length, read_bytes] = read_t_from_buffer<foxtalk_size_t>(buffer, buffer_position);
                buffer_position += read_bytes;

                std::vector<uint8_t> bytes {};
                for (int i = 0; i < length; i++)
                {
                    auto [byte, read_bytes] = read_t_from_buffer<uint8_t>(buffer, buffer_position);
                    buffer_position += read_bytes;
                    bytes.push_back(byte);
                }

                return {
                    TupleNoun{bytes},
                    buffer_position - start_position};
            }
        case NounType::CPtr:
        {
            auto [dat, read_bytes] = read_t_from_buffer<void *>(buffer, buffer_position);
            buffer_position += read_bytes;

            return {
                TupleNoun{dat},
                buffer_position - start_position};
        }
        case NounType::U64:
        {
            auto [dat, read_bytes] = read_t_from_buffer<uint64_t>(buffer, buffer_position);
            buffer_position += read_bytes;

            return {
                TupleNoun{dat},
                buffer_position - start_position};
        }
        case NounType::I64:
        {
            auto [dat, read_bytes] = read_t_from_buffer<int64_t>(buffer, buffer_position);
            buffer_position += read_bytes;

            return {
                TupleNoun{dat},
                buffer_position - start_position};
        }
        default:
            throw std::runtime_error("Unknown NounType!");
        }
    }

    size_t write_to_buffer(uint8_t *buffer, size_t buffer_position) const
    {
        size_t current_position = buffer_position;

        current_position += write_t_to_buffer(buffer, current_position, type);

        switch (type)
        {
        case NounType::Query:
        case NounType::Prefix:
            break;

        case NounType::Symbol:
        {
            auto sym = std::get<std::string>(data);
            current_position += write_t_to_buffer(buffer, current_position, (foxtalk_size_t)sym.length());
            sym.copy((char *)(buffer + current_position), sym.length());
            current_position += sym.length();
            break;
        }
        case NounType::CPtr:
            current_position += write_t_to_buffer(buffer, current_position, (size_t)std::get<void *>(data));
            break;
        case NounType::U64:
            current_position += write_t_to_buffer(buffer, current_position, std::get<uint64_t>(data));
            break;
        case NounType::I64:
            current_position += write_t_to_buffer(buffer, current_position, std::get<int64_t>(data));
            break;
        case NounType::Bytes:
            {
                auto bytes = std::get<std::vector<uint8_t>>(data);
                current_position += write_t_to_buffer(buffer, current_position, (foxtalk_size_t)bytes.size());
                memcpy((buffer + current_position), bytes.data(), sizeof(uint8_t) * bytes.size());
                current_position += sizeof(uint8_t) * bytes.size();
                break;
            }
        default:
            throw std::runtime_error("Unknown NounType!");
        }

        return current_position - buffer_position;
    }
};

struct Tuple
{
private:
    std::vector<TupleNoun> nouns_;

public:
    //// CONSTRUCTORS ////

    // TODO: During performance testing, see if we're copying triples and triplenouns like crazy
    // Triple(const Triple &) = delete;
    // void operator=(const Triple &) = delete;

    // Triple(const Triple &&other) noexcept
    //         : subject_(std::move(other.subject_)), predicate_(std::move(other.predicate_)),
    //           object_(std::move(other.object_)) {}

    Tuple(std::vector<TupleNoun> &&nouns) : nouns_{std::move(nouns)} {}

    bool operator==(const Tuple &other) const
    {
        if (nouns_.size() != other.nouns_.size())
        {
            return false;
        }

        for (int i = 0; i < nouns_.size(); i++)
        {
            if (nouns_[i] != other.nouns_[i])
            {
                return false;
            }
        }

        return true;
    }

    //// TO STRING /////
    friend std::ostream &operator<<(std::ostream &os, const Tuple &triple)
    {
        os << "<";
        auto s = triple.nouns_.size();
        for (int i = 0; i < s; i++)
        {
            os << triple.nouns_[i];
            if (i + 1 < s)
            {
                os << ", ";
            }
        }
        os << ">";
        return os;
    }

    //// ACCESSORS /////
    template <typename T>
    std::optional<const T> at(size_t i) const
    {
        if (nouns_.size() < i)
            return std::nullopt;

        if (std::holds_alternative<T>(nouns_[i].data))
        {
            return {std::get<T>(nouns_[i].data)};
        }
        else
        {
            // This isn't really a warning... we use at<T>
            // std::cout << "WARNING: noun at " << i << " was not a " << typeid(T).name()
            //           << " (It had the foxtalk type id " << (size_t)nouns_[i].type << ")"
            //           << std::endl;

            return std::nullopt;
        }
    }

    template<typename T>
    std::optional<T> struct_at(size_t i) const
    {
        if (nouns_.size() < i)
            return std::nullopt;

        return nouns_[i].into_struct<T>();
    }

    template <typename T>
    bool matches(size_t i, T to_match) const
    {
        auto v = at<T>(i);
        if (v.has_value())
        {
            return v.value() == to_match;
        }

        return false;
    }

    //// SERIALIZATION / DESERIALIZATION /////
    size_t write_to_buffer(uint8_t *buffer, size_t start_position) const
    {
        return write_tuple_noun_vec_to_buffer(buffer, start_position, nouns_);
    }

    static std::pair<Tuple, foxtalk_size_t> read_from_buffer(uint8_t *buffer, size_t start_position)
    {
        auto [nouns, read_bytes] = read_tuple_noun_vec_from_buffer<TupleNoun>(buffer, start_position);
        return std::pair<Tuple, size_t>(Tuple{std::move(nouns)}, read_bytes);
    }
};

#endif // REACTOR_FOXTALK_TUPLE_H
