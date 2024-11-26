#include <sys/inotify.h>
#include <unistd.h>
#include <iostream>
#include <climits>
#include <set>
#include <cstring>

#include <filesystem>
namespace fs = std::filesystem;

#include <foxtalk_handler.hpp>


class CameraStateWatchingHandler : public Handler
{
int fd = -1;
int wd = -1;

char buffer[sizeof(struct inotify_event) + NAME_MAX + 1];

std::set<std::string> active_cameras {};


public:
  bool poll() override {
      int length = read(fd, buffer, sizeof(buffer));
      if (length < 0) {
          // perror("camera_state_watching_handler had error reading from buffer");
          return false;
      }

      bool to_ret = false;

      for (int i = 0; i < length;) {
          auto *event = (struct inotify_event *)&buffer[i];
          if (strncmp(event->name, "video", 5) == 0) {
            if (event->mask & IN_CREATE || event->mask & IN_MOVED_TO) {

              debug << "New camera detected: " << event->name << end;
              active_cameras.insert(event->name);
              to_ret = true;
            }
            if (event->mask & IN_DELETE || event->mask & IN_MOVED_FROM) {

              debug << "Camera removed: " << event->name << end;
              active_cameras.erase(event->name);
              to_ret = true;
            }
          }

          i += sizeof(struct inotify_event) + event->len;
      }
      return to_ret;

  }

  ~CameraStateWatchingHandler() {
    inotify_rm_watch(fd, wd);
    close(fd);
  }
protected:
  void handle(const std::vector<Tuple> &queryResults) override {
    for (auto &q: active_cameras) {
      claim({{{"/dev/" + q}, {"is a"}, {"camera device"}}});
    }
  }

  void init() override {
    fd = inotify_init1(IN_NONBLOCK);
    wd = inotify_add_watch(fd, "/dev", IN_CREATE | IN_MOVED_TO | IN_MOVED_FROM | IN_DELETE);
    for (const auto & entry : fs::directory_iterator("/dev")) {
      std::string q = entry.path().c_str();
      if (q.find("video")  != std::string::npos) {
        active_cameras.insert(q.erase(0, 5));
      }
    }

    claim({{{"foxtalk"}, {"is"}, {"running"}}});

  }


};

FOXTALK_FFI_HANDLER_REG(CameraStateWatchingHandler);