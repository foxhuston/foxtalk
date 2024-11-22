// pkg-config: vulkan wayland-client

#include <iostream>
#include "foxtalk_tuple.h"
#include <foxtalk_handler.hpp>
#include <vector>
#include <vulkan/vulkan.h>
#include <wayland-client.h>

#include <vulkan/vulkan_wayland.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
// #include "xdg-shell.h"


wl_display *display;
wl_registry *registry;
wl_registry_listener *registry_listener;

wl_compositor *compositor;
wl_shm *shm;

// zxdg_shell_v6 *xdg_shell;
int display_fd;

void registry_global_handler
(
    void *data,
    struct wl_registry *registry,
    uint32_t name,
    const char *interface,
    uint32_t version
) {
  std::cout << "interface: '" << interface << "', version: " << version << ", name: " << name << std::endl;
  if (std::string(interface) == "wl_compositor" ) {
    compositor = static_cast<wl_compositor*>(wl_registry_bind(registry, name, &wl_compositor_interface, 4));
  }
  else if (std::string(interface) == "wl_shm" ) {
    shm = static_cast<wl_shm*>(wl_registry_bind(registry, name, &wl_shm_interface, 1));
  }
  // else if (std::string(interface) == "zxdg_shell_v6" ) {
  //   xdg_shell = static_cast<zxdg_shell_v6*>(wl_registry_bind(registry, name, &zxdg_shell_v6_interface, 1));
  // }
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

  void register_initial_tuples() override {

    std::cout << "Connecting to display" << std::endl;
    
    display = wl_display_connect(nullptr);
    if (display == nullptr) {
      std::cerr << "Wayland display failed to connect" << std::endl;
      return;
    }
    // std::cout << "testing "<< wl_display_get_error(display) << std::endl;
    display_fd = wl_display_get_fd(display);
    registry = wl_display_get_registry(display); 
    registry_listener = new wl_registry_listener {
        .global = registry_global_handler,
        .global_remove = registry_global_remove_handler
    };

    wl_registry_add_listener(registry, registry_listener, nullptr);
    wl_display_roundtrip(display); // Do we need this?
  }

  bool display_dispatch_setup = false;
  bool poll() override {
    // std::cout << "Poll! About to dispatch display..." << std::endl;
    display_dispatch_setup = true;
    return wl_display_dispatch_pending(display) > 0;
  }

  ~WaylandInstanceHandler() {
    wl_display_disconnect(display);
    delete registry;
    delete registry_listener;
    delete compositor;
    delete shm;
    // delete shell;
  }
protected:
  void handle(const std::vector<Tuple> &queryResults) override {
    if (display != nullptr) {
      claim({{{display}, {"is the"}, {"wayland display"}, {"with fd"}, {display_fd}}});
    }
    if (registry != nullptr) {
      claim({{{registry}, {"is the"}, {"global wayland registry"}}});
    }
    
    if (registry_listener != nullptr) {
      claim({{{registry_listener}, {"is the"}, {"global wayland registry_listener"}}});
    }
    if (compositor != nullptr) {
      claim({{{compositor}, {"is the"}, {"global wayland compositor"}}});
    }
    if (shm != nullptr) {
      claim({{{shm}, {"is the"}, {"global wayland shm"}}});
    }
    // if (xdg_shell != nullptr) {
    //   claim({{{xdg_shell}, {"is the"}, {"global wayland xdg_shell"}}});
    // }
    if (queryResults.size() != 1 || !display_dispatch_setup) { return; }
    wl_display_dispatch(display);
  }

  void free_tuple(const Tuple &t) override {
  }

  void init() override {
    claim({{{"foxtalk"}, {"is"}, {"running"}}});
  }

};

FOXTALK_FFI_HANDLER_REG(WaylandInstanceHandler);