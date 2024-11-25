//
// Created by fox on 10/22/24.
//

#ifndef REACTOR_FOXTALK_HANDLER_HPP
#define REACTOR_FOXTALK_HANDLER_HPP

#include "foxtalk_handler.h"
#include <exception>
#include <ostream>
#include <sstream>


enum class LogState
{
    End
};

class Handler
{
protected:
    void claim(Tuple&& n) { claims.push_back(std::move(n)); }
    virtual void handle(const std::vector<Tuple>& queryResults) = 0;

    virtual void free_tuple(const Tuple&)
    {
    }

    virtual void init() = 0;

    enum class LogLevel
    {
        Debug,
        Error
    };

    class FoxtalkLoggingBuffer : public std::streambuf
    {
    public:
        FoxtalkLoggingBuffer() = default;
        std::ostringstream buffer;

        [[nodiscard]] std::string get_string() const
        {
            return buffer.str();
        }

    protected:
        int overflow(int c) override
        {
            if (c != EOF)
            {
                buffer.put(static_cast<char>(c));
            }
            return c;
        }

        std::streamsize xsputn(const char* s, std::streamsize n) override
        {
            buffer.write(s, n);
            return n;
        }
    };

    class FoxtalkLogger : public std::ostream
    {
    public:
        FoxtalkLogger(
            LogLevel level,
            Handler* handler) : std::ostream(&buf)
        {
            _level = level;
            outer = handler;
        }

        FoxtalkLogger& operator<<(const LogState state)
        {
            if (state == LogState::End)
            {
                std::string captured_string = buf.get_string();
                buf = FoxtalkLoggingBuffer(); // Reset the buffer
                handle_string(captured_string);
            }

            return *this;
        }

    private:
        LogLevel _level = LogLevel::Debug;

        Handler* outer;
        FoxtalkLoggingBuffer buf{};

        void handle_string(const std::string& str)
        {
            if (_level == LogLevel::Debug)
            {
                outer->log("has debug message", str);
            }
            else if (_level == LogLevel::Error)
            {
                outer->log("has error message", str);
            }
            else
            {
                throw std::logic_error("Unhandled log level in the root handler class");
            }
        }
    };

    void log(std::string log_type, std::string message)
    {
        claim({
            {
                {std::string(this->name)},
                {log_type},
                {message}
            }
        });
    }

public:
    std::string name = "Uninitialized Handler";
    std::vector<Tuple> claims{}; // should be private, but I need to test...

    void set_logger_name(const char* name) { this->name = name; }
    FoxtalkLogger err = FoxtalkLogger(LogLevel::Error, this);
    FoxtalkLogger debug = FoxtalkLogger(LogLevel::Debug, this);
    LogState end = LogState::End;

    void log_existing_error()
    {
        // err << FoxtalkLoggingAction::End;
    }

    void log_existing_debug()
    {
        // claim({
        //     {
        //         {std::string(this->name)},
        //         {"has debug message"},
        //         {std::string(debug.str())}
        //     }
        // });
        // debug.clear();
    }

    void log_error(const char* msg)
    {
        claim({
            {
                {std::string(this->name)},
                {"has error message"},
                {std::string(msg)}
            }
        });
    }

    void log_debug(const char* msg)
    {
        claim({
            {
                {std::string(this->name)},
                {"has debug message"},
                {std::string(msg)}
            }
        });
    }

    virtual void register_initial_tuples()
    {
    }

    virtual bool poll() { return false; }

    void ffi_free_tuple(uint8_t* buffer)
    {
        auto [t, read_bytes] = Tuple::read_from_buffer(buffer, 0);
        free_tuple(t);
    }

    void ffi_init(uint8_t* buffer)
    {
        claims.clear();
        init();
        write_tuple_noun_vec_to_buffer<Tuple>(buffer, 0, claims);
    }

    void ffi_register_init(uint8_t* buffer)
    {
        claims.clear();
        register_initial_tuples();
        write_tuple_noun_vec_to_buffer<Tuple>(buffer, 0, claims);
    }

    void ffi_handle(uint8_t* buffer)
    {
        // Initialize
        claims.clear();

        // How many tuples in query result set?
        auto [query_results, read_bytes] =
            read_tuple_noun_vec_from_buffer<Tuple>(buffer, 0);

        // Actually work with the results
        handle(query_results);

        // Serialize
        write_tuple_noun_vec_to_buffer<Tuple>(buffer, 0, claims);
    }
};

#define FOXTALK_FFI_HANDLER_REG(T)                                             \
    static T *T##_instance = nullptr;                                          \
    void foxtalk_init(uint8_t *buffer)                                       \
    {                                                                          \
        try                                                                    \
        {                                                                      \
            T##_instance = new T();                                            \
            T##_instance->ffi_init(buffer);                                    \
            T##_instance->set_logger_name(#T);                                 \
        }                                                                      \
        catch (std::exception const &e)                                        \
        {                                                                      \
            std::cerr << "CRASH in [" << #T << "] init():" << e.what() << std::endl;          \
        }                                                                      \
        catch (...)                                                            \
        {                                                                      \
            std::cerr << "CRASH in [" << #T << "] init()" << std::endl;                       \
        }                                                                      \
    }                                                                          \
    void foxtalk_free_tuple(uint8_t *buffer)                                                          \
    {                                                                          \
        try                                                                    \
        {                                                                      \
            T##_instance->ffi_free_tuple(buffer);                 \
        }                                                                      \
        catch (std::exception const &e)                                        \
        {                                                                      \
            std::cerr << "CRASH in [" << #T << "] free_tuple():" << e.what() << std::endl;    \
        }                                                                      \
        catch (...)                                                            \
        {                                                                      \
            std::cerr << "CRASH in [" << #T << "] free_tuple()" << std::endl;                 \
        }                                                                      \
    }                                                                          \
    void foxtalk_handle(uint8_t *buffer)                                                              \
    {                                                                          \
        try                                                                    \
        {                                                                      \
            T##_instance->ffi_handle(buffer);                     \
        }                                                                      \
        catch (std::exception const &e)                                        \
        {                                                                      \
            std::cerr << "CRASH in [" << #T << "] handle():" << e.what() << std::endl;        \
        }                                                                      \
        catch (...)                                                            \
        {                                                                      \
            std::cerr << "CRASH in [" << #T << "] handle()" << std::endl;                     \
        }                                                                      \
    }                                                                          \
    void foxtalk_register_initial_tuples(uint8_t *buffer)                                             \
    {                                                                          \
        try                                                                    \
        {                                                                      \
            T##_instance->ffi_register_init(buffer);              \
        }                                                                      \
        catch (std::exception const &e)                                        \
        {                                                                      \
            std::cerr << "CRASH in [" << #T << "] register_init():" << e.what() << std::endl; \
        }                                                                      \
        catch (...)                                                            \
        {                                                                      \
            std::cerr << "CRASH in [" << #T << "] register_init()" << std::endl;              \
        }                                                                      \
    }                                                                          \
    void foxtalk_teardown()                                                            \
    {                                                                          \
        try                                                                    \
        {                                                                      \
            delete T##_instance;                                               \
        }                                                                      \
        catch (std::exception const &e)                                        \
        {                                                                      \
            std::cerr << "CRASH in [" << #T << "] teardown():" << e.what() << std::endl;      \
        }                                                                      \
        catch (...)                                                            \
        {                                                                      \
            std::cerr << "CRASH in [" << #T << "] teardown()" << std::endl;                   \
        }                                                                      \
    }                                                                          \
    bool foxtalk_poll()                                                                \
    {                                                                          \
        try                                                                    \
        {                                                                      \
            return T##_instance->poll();                                       \
        }                                                                      \
        catch (std::exception const &e)                                        \
        {                                                                      \
            std::cerr << "CRASH in [" << #T << "] poll():" << e.what() << std::endl;          \
            return false;                                                      \
        }                                                                      \
        catch (...)                                                            \
        {                                                                      \
            std::cerr << "CRASH in [" << #T << "] poll()" << std::endl;                       \
            return false;                                                      \
        }                                                                      \
    }

//#define FOXTALK_FFI_HANDLER(T, qr) \
//    FOXTALK_HANDLER_DEF(T, qr);    \
//    FOXTALK_FFI_HANDLER_REG(T)     \
//    FOXTALK_HANDLER_BOD(T, qr)

#endif // REACTOR_FOXTALK_HANDLER_HPP
