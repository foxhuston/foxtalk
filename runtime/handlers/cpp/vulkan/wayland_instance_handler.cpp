// pkg-config: vulkan wayland-client

#include <iostream>
#include "foxtalk_tuple.h"
#include <foxtalk_handler.hpp>
#include <vector>
#include <vulkan/vulkan.h>
#include "wayland-client.h"

#include <vulkan/vulkan_wayland.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>


void registry_global_handler
(
    void *data,
    struct wl_registry *registry,
    uint32_t name,
    const char *interface,
    uint32_t version
) {
    std::cout << "interface: '" << interface << "', version: " << version << ", name: " << name << std::endl;
}

void registry_global_remove_handler
(
    void *data,
    struct wl_registry *registry,
    uint32_t name
) {
    std::cout << "removed: " << name << std::endl;
}

class WaylandInstanceHandler : public Handler
{
public:

  bool poll() override {
    std::cout << "Test" << std::endl;
    wl_display_dispatch(display);
    return false;
  }
protected:

  struct wl_display* display {};
  void handle(const std::vector<Tuple> &queryResults) override {

    if (queryResults.size() != 1) { return; }

    display = wl_display_connect(nullptr);
    if (display == nullptr) {
      std::cerr << "Wayland display failed to connect" << std::endl;
      return;
    }
    std::cout << "testing "<< wl_display_get_error(display) << std::endl;
    auto display_fd = wl_display_get_fd(display);
    claim({{{display}, {"is the"}, {"wayland display"}, {"with fd"}, {display_fd}}});

    struct wl_registry *registry = wl_display_get_registry(display);
    struct wl_registry_listener registry_listener = {
        .global = registry_global_handler,
        .global_remove = registry_global_remove_handler
    };
    wl_registry_add_listener(registry, &registry_listener, nullptr);
  }

  void free_tuple(const Tuple &t) override {
    // auto display = static_cast<wl_display*>(t.at<void *>(0).value());
    // wl_display_disconnect(display);
    // std::cout << "Disconnecting wayland display" << std::endl;
  }

  void init() override {
    claim({{{"foxtalk"}, {"is"}, {"running"}}});
  }
};

FOXTALK_FFI_HANDLER_REG(WaylandInstanceHandler);