//
// Created by fox on 10/3/24.
//

#ifndef REACTOR_FSWATCHER_H
#define REACTOR_FSWATCHER_H

#include <iostream>
#include <filesystem>
#include <thread> // Oh boy.

#include <sys/inotify.h>
#include <unistd.h>
#include <string>
#include <cstring>

namespace fs = std::filesystem;

class FsDirectoryWatcher {
private:
    bool running;
    std::thread thread_handle;

protected:
    virtual void file_notification(std::string file_name, uint32_t mask_flags) = 0;
    void watch_directory(fs::path watch_directory) {
        thread_handle = std::thread([this, watch_directory]() {
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

//                std::cout << "Adding watch for directory " << watch_dir << std::endl;
            int wd = inotify_add_watch(fd, watch_directory.c_str(), IN_DELETE | IN_CLOSE_WRITE | IN_CREATE);
            if(wd < 0) {
                std::cerr << "Could not add inotify watch! " << strerror(errno) << std::endl;
                std::terminate();
            }

//                std::cout << "Got watch id " << wd << std::endl;

            while(running) {
//                    std::cout << "Calling read for fd..." << std::endl;
                len = read(fd, &buf, sizeof(buf));
                if(len == -1 && errno != EAGAIN) {
                    std::cerr << "Could not read file descriptor for " << watch_directory <<
                              ": " << strerror(errno) << std::endl;

                    std::flush(std::cerr);
                    std::terminate();
                }

                // What the hell, linux -_-
                for (char *ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
                    event = (const struct inotify_event *) ptr;
                    auto event_file = std::string(event->name);
                    file_notification(event_file, event->mask);
                }
            }

            inotify_rm_watch(fd, wd);
            close(fd);

            std::cout << "Thread stopped!" << std::endl;
        });
    }

public:
    ~FsDirectoryWatcher() {
        running = false;
    }
};

#endif //REACTOR_FSWATCHER_H
