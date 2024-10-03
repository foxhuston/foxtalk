//
// Created by fox on 10/2/24.
//

#ifndef REACTOR_DYNAMICHANDLER_H
#define REACTOR_DYNAMICHANDLER_H

#include <optional>
#include <format>
#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <sstream>

#include "Reactor.h"
#include "FsWatcher.h"
#include "SharedObjectHandler.h"

namespace fs = std::filesystem;

namespace foxtalk {
    struct DynamicCompilingHandler : FsDirectoryWatcher {
    private:
        const fs::path watch_dir;
        Reactor *reactor;

        std::unordered_map<fs::path, SharedObjectHandler *> handlers;

        void remove_handler_for_path(const fs::path& path) {
            if(handlers[path] != nullptr) {
                reactor->remove_handler(handlers[path]);
                delete handlers[path];
                handlers[path] = nullptr;
            }
        }

        void add_handler_for_path(const fs::path& path) {
            remove_handler_for_path(path);
            handlers[path] = new SharedObjectHandler(path.c_str());
            reactor->add_handler(handlers[path]);
        }

        fs::path compile_cpp_file(const fs::path& path) {
            auto out_file_path = path.filename();
            out_file_path.replace_extension(".so");

            std::stringstream command;
            command << "clang++ -I lib/ -I vendor/gc-8.2.4/include -shared -fPIC "
                    << fs::absolute (watch_dir / path)
                    << " -o " << fs::absolute(watch_dir / out_file_path);


            std::array<char, 1024> buff;
            FILE *pipe = popen(command.str().c_str(), "r");
            if(!pipe) {
                throw std::runtime_error(
                        std::format("Couldn't run command: {0}", command.str()));
            }

            while(fgets(buff.data(), buff.size(), pipe) != NULL) {
                std::cout << buff.data();
            }

            auto returnCode = pclose(pipe);
            std::cout << "command returned with code " << returnCode << std::endl;

            return out_file_path;
        }
    public:
        DynamicCompilingHandler(Reactor *reactor, const char *dir)
                : reactor{reactor}, watch_dir{fs::path(dir)} {
            watch_directory(watch_dir.c_str());
        }

    protected:
        void file_notification(fs::path event_file, uint32_t mask_flags) override {
            if(event_file.extension() != ".cpp") return;

            if (mask_flags & IN_CREATE) {
                std::cout << "Detected " << event_file << " IN_CREATE" << std::endl;
                auto out_file = compile_cpp_file(event_file);

            } else if (mask_flags & IN_CLOSE_WRITE) {
                std::cout << "Detected " << event_file << " IN_CLOSE_WRITE" << std::endl;
                auto out_file = compile_cpp_file(event_file);

            } else if (mask_flags & IN_DELETE) {
                std::cout << "Detected " << event_file << " IN_DELETE" << std::endl;

            } else {
                std::cout << "Detected " << event_file << " unknown" << std::endl;
            }
        }
    };
}

#endif //REACTOR_DYNAMICHANDLER_H
