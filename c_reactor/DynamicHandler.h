//
// Created by fox on 10/2/24.
//

#ifndef REACTOR_DYNAMICHANDLER_H
#define REACTOR_DYNAMICHANDLER_H

#include <optional>
#include <iostream>
#include <thread> // Oh boy.

#include <sys/inotify.h>
#include <poll.h>
#include <unistd.h>

#include "Reactor.h"
#include "SharedObjectHandler.h"

namespace foxtalk {
    struct DynamicHandler {
    private:
        bool running;

        Reactor* reactor;
        std::thread thread_handle;
        const char* const watch_file;

        SharedObjectHandler* soh = nullptr;

        void reload() {
            if(soh != nullptr) {
                reactor->remove_handler(soh);
            }

            soh = new SharedObjectHandler(watch_file);
            reactor->add_handler(soh);
        }

    public:
        DynamicHandler(Reactor* reactor, const char* watch_file) : reactor { reactor }, watch_file { watch_file } {
            reload();

            thread_handle = std::thread([this, watch_file]() {
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
                int wd = inotify_add_watch(fd, watch_file, IN_CLOSE);

                std::cout << "Thread will wait for change..." << std::endl;
                while(running) {
                    len = read(fd, &buf, sizeof(buf));
                    if(len == -1 && errno != EAGAIN) {
                        std::cerr << "Could not read file descriptor for " << watch_file <<
                            ": " << strerror(errno) << std::endl;

                        std::flush(std::cerr);
                        std::terminate();
                    }

                    // What the hell, linux -_-
                    for (char *ptr = buf; ptr < buf + len;
                         ptr += sizeof(struct inotify_event) + event->len) {

                        event = (const struct inotify_event *) ptr;
                        std::cout << "Detected a change!" << ((event->mask & IN_CLOSE_WRITE) == IN_CLOSE_WRITE) << std::endl;
                        reload();
                    }

                }

                std::cout << "Thread stopped!" << std::endl;
            });
        }

        ~DynamicHandler() {
            running = false;
        }
    };
}

#endif //REACTOR_DYNAMICHANDLER_H
