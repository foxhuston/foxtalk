//
// Created by fox on 10/2/24.
//

#ifndef REACTOR_DYNAMICHANDLER_H
#define REACTOR_DYNAMICHANDLER_H

#include <optional>
#include <iostream>
#include <filesystem>
#include <thread> // Oh boy.

#include <sys/inotify.h>
#include <poll.h>
#include <unistd.h>

#include "Reactor.h"
#include "SharedObjectHandler.h"

namespace fs = std::filesystem;

namespace foxtalk {
    struct DynamicHandler {
    private:
        bool running;
        std::thread thread_handle;

    public:
        DynamicHandler(Reactor* reactor, const char* watch_file) {

            thread_handle = std::thread([this, reactor, watch_file]() {
                SharedObjectHandler *soh;

                // Does the file exist?
                if(fs::exists(watch_file)) {
                    // If so, load it when we start.
                    soh = new SharedObjectHandler(watch_file);
                    reactor->add_handler(soh);
                }

                char buf[4096]
                        __attribute__ ((aligned(__alignof__(struct inotify_event))));
                const struct inotify_event *event;
                ssize_t len;

                /* Create the file descriptor for accessing the inotify API. */

                int fd = inotify_init();
                if (fd == -1) {
                    perror("inotify_init");
                    exit(EXIT_FAILURE);
                }

                /* Allocate memory for watch descriptors. */

                auto watch_path = fs::path(watch_file);
                auto watch_file_name = watch_path.filename().string();
                auto watch_dir = watch_path.remove_filename();
//                std::cout << "Adding watch for directory " << watch_dir << std::endl;
                int wd = inotify_add_watch(fd, watch_dir.c_str(), IN_DELETE | IN_CLOSE_WRITE | IN_CREATE);
                if(wd < 0) {
                    std::cerr << "Could not add inotify watch! " << strerror(errno) << std::endl;
                    std::terminate();
                }

//                std::cout << "Got watch id " << wd << std::endl;

                while(running) {
//                    std::cout << "Calling read for fd..." << std::endl;
                    len = read(fd, &buf, sizeof(buf));
                    if(len == -1 && errno != EAGAIN) {
                        std::cerr << "Could not read file descriptor for " << watch_file <<
                            ": " << strerror(errno) << std::endl;

                        std::flush(std::cerr);
                        std::terminate();
                    }

                    // What the hell, linux -_-
//                    std::cout << "Got something! Looping through events..." << std::endl;
                    for (char *ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
                        event = (const struct inotify_event *) ptr;
                        auto event_file = std::string(event->name);

                        if(event_file == watch_file_name) {
                            std::cout << "Detected a change for file " << event->name << std::endl;
                            if(event->mask & IN_CLOSE_WRITE) {
                                if(soh != nullptr) {
                                    reactor->remove_handler(soh);
                                    delete soh;
                                    soh = nullptr;
                                }

                                soh = new SharedObjectHandler(watch_file);

                                std::cout << "   CloseWrite" << std::endl;
                                std::cout << "     soh = " << soh << std::endl;

                                reactor->add_handler(soh);
                            } else if (event->mask & IN_DELETE) {
                                std::cout << "   Delete" << std::endl;
                                std::cout << "     soh = " << soh << std::endl;
                                if(soh != nullptr) {
                                    reactor->remove_handler(soh);
                                    delete soh;
                                    std::cout << "     Deleted and set soh to nullptr." << std::endl;
                                    soh = nullptr;
                                }
                            } else if (event->mask & IN_CREATE) {
//                                while(true) {
//                                    try {
//                                        if (soh == nullptr) {
//                                            std::cout << "   Create" << std::endl;
//                                            soh = new SharedObjectHandler(watch_file);
//                                            reactor->add_handler(soh);
//                                        }
//                                    }
//                                }
                            }
                        }
                    }
//                    std::cout << "Events looped." << std::endl;
                }

                inotify_rm_watch(fd, wd);
                close(fd);

                std::cout << "Thread stopped!" << std::endl;
            });
        }

        ~DynamicHandler() {
            running = false;
        }
    };
}

#endif //REACTOR_DYNAMICHANDLER_H
