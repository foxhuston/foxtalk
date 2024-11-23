// pkg-config wayland-client

#include "foxtalk_tuple.h"
#include <foxtalk_handler.hpp> 
#include <iostream>
#include <sys/mman.h>
#include <wayland-client-protocol.h>

struct wayland_buffer {
  Tuple surface_tuple;
  int fd;
  void* buffer;
  uint64_t num_bytes;
};

class WaylandSharedMemoryHandler : public Handler
{
public:
  std::vector<wayland_buffer> buffers {};
protected:
  
  void handle(const std::vector<Tuple> &queryResults) override {

    if (queryResults.size() < 2) {
      return;
    }

    wl_shm* shm;

    for (const auto& r: queryResults) {
      if (r.matches(2, std::string("global wayland shm"))) {
        shm = static_cast<wl_shm*>(r.at<void* >(0).value());
      }
    }

    for (const auto& r: queryResults) {
      if (r.matches(2, std::string("a wayland surface"))) {

        // have we created the buffer for this surface already?
        auto needs_to_create = true;
        for (const auto& st: buffers) {
          if (st.surface_tuple == r) {
            needs_to_create = false;
            break;
          }
        }
        if (needs_to_create) {
          std::cout << "Need to create new file buffer matching " << r << std::endl;
          // Does this go out of scope immediately?
          buffers.emplace_back(wayland_buffer {
            .surface_tuple = r,
            .fd = 123, // todo create fd
            .buffer = (void *)"aeoushi", // todo: memmap
            .num_bytes = 123124, // todo: calculate num bytes of double buffer at some chosen pxf
          });
        }
        
      }
    }

    for (auto const& buffer: buffers) {
      claim({{
        {buffer.buffer},
        {"is a"},
        {"wayland memory buffer"},
        {"with num bytes"},
        {buffer.num_bytes},
        {"with fd"},
        {buffer.fd},
        {"for surface id"},
        {"......."}, 
      }});
    }
  }
  void free_tuple(const Tuple &t) override {
  
    auto fd_to_remove = t.at<uint64_t>(6).value();
    int idx_to_remove = -1;
    for (auto i = 0; i < buffers.size(); i++) {
      if (buffers[i].fd == fd_to_remove) {
        idx_to_remove = i;
      }
    }

    if (idx_to_remove < 0) {
      std::cerr << "Could not find buffer to remove in wayland shared memory handler free tuple" << std::endl;
    }

    auto b = buffers[idx_to_remove]; 
    // munmap(b.buffer, b.num_bytes);
    // close(b.fd);
    std::cout << "Freeing buffer at " << idx_to_remove;
    buffers.erase(buffers.begin() + idx_to_remove);
  }

  void init() override {
    claim({{TupleNoun::query(), {"is the"}, {"global wayland shm"}}});
    // todo, make the surface manager pass along the wish requests, don't use the wish directly
    claim({{
      TupleNoun::query(),
      {"wishes for"},
      {"a wayland surface"},
      TupleNoun::prefix()
      }});
    // claim({{TupleNoun::query(), {"is a"}, {"wayland surface"}, {"with surface id"}, TupleNoun::query()}});
  }
};

FOXTALK_FFI_HANDLER_REG(WaylandSharedMemoryHandler);