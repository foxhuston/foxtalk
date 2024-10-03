//
// Created by fox on 10/2/24.
//

#ifndef REACTOR_DYNAMICHANDLER_H
#define REACTOR_DYNAMICHANDLER_H

#include <optional>
#include <iostream>
#include <filesystem>

#include "Reactor.h"
#include "FsWatcher.h"
#include "SharedObjectHandler.h"

namespace fs = std::filesystem;

namespace foxtalk {
    struct DynamicHandler : FsDirectoryWatcher {
    private:
        SharedObjectHandler *soh = nullptr;
        std::string watch_file_name;
        const fs::path watch_dir;
        Reactor *reactor;

    public:
        DynamicHandler(Reactor *reactor, const char *watch_file)
            : reactor { reactor }
            , watch_file_name { fs::path(watch_file).filename().string() }
            , watch_dir { fs::path(watch_file).remove_filename() }
        {
            watch_directory(watch_dir);

            if (fs::exists(watch_file)) {
                // If so, load it when we start.
                soh = new SharedObjectHandler(watch_file);
                reactor->add_handler(soh);
            }
        }

    protected:
        void file_notification(fs::path event_file, uint32_t mask_flags) override {
            if (event_file == watch_file_name) {
                if (mask_flags & IN_CLOSE_WRITE) {
                    std::cout << "Detected " << event_file << " IN_CLOSE_WRITE" << std::endl;
                    if (soh != nullptr) {
                        reactor->remove_handler(soh);
                        delete soh;
                        soh = nullptr;
                    }

                    auto full_path = absolute(watch_dir / event_file);
                    soh = new SharedObjectHandler(full_path.c_str());
                    reactor->add_handler(soh);

                } else if (mask_flags & IN_DELETE) {
                    std::cout << "Detected " << event_file << " IN_DELETE" << std::endl;

                    if (soh != nullptr) {
                        reactor->remove_handler(soh);
                        delete soh;
                        std::cout << "     Deleted and set soh to nullptr." << std::endl;
                        soh = nullptr;
                    }
                } else if (mask_flags & IN_CREATE) {
                    std::cout << "Detected " << event_file << " IN_CREATE" << std::endl;
                    //
                } else {
                    std::cout << "Detected " << event_file << " unknown" << std::endl;
                }
            }
        }
    };
}

#endif //REACTOR_DYNAMICHANDLER_H
