// pkg-config: wayland-client

#include "foxtalk_tuple.h"
#include <foxtalk_handler.hpp>
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>
#include <wayland-client.h>

#include "./xdg-shell.h"
#include <vulkan/vulkan_wayland.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

wl_display *display;
wl_registry *registry;

wl_compositor *compositor;
wl_shm *shm;
xdg_wm_base *ft_xdg_wm_base;
wl_seat *seat;

wl_keyboard *keyboard;
wl_surface *surface;
xdg_surface *ft_xdg_surface;
xdg_toplevel *ft_xdg_toplevel;

wl_shm_listener *listener;

std::vector<uint32_t> pixel_formats{};

// zxdg_shell_v6 *xdg_shell;
int display_fd;

void registry_global_handler(void *data, struct wl_registry *registry,
                             uint32_t name, const char *interface,
                             uint32_t version) {
  if (std::string(interface) == "wl_compositor") {
    compositor = static_cast<wl_compositor *>(
        wl_registry_bind(registry, name, &wl_compositor_interface, 1));
  } else if (std::string(interface) == "wl_shm") {
    shm = static_cast<wl_shm *>(
        wl_registry_bind(registry, name, &wl_shm_interface, 1));
  } else if (std::string(interface) == "xdg_wm_base") {
    ft_xdg_wm_base = static_cast<xdg_wm_base *>(
        wl_registry_bind(registry, name, &xdg_wm_base_interface, 1));
  } else if (std::string(interface) == "wl_seat") {
    seat = static_cast<wl_seat *>(
        wl_registry_bind(registry, name, &wl_seat_interface, 1));
  }
}

void registry_global_remove_handler(void *data, struct wl_registry *registry,
                                    uint32_t name) {
  // debug << std::format("removed: {}", name).data() << end;
}

void shm_global_handler(void *data, struct wl_shm *shm, uint32_t format) {
  pixel_formats.emplace_back(format);
}

void handle_xdg_surface_configure(void *data, xdg_surface *surface,
                                  uint32_t serial) {
  xdg_surface_ack_configure(surface, serial);

  // debug << "handle_xdg_surface_configure called" << end;
  //  if (vc->wl.wait_for_configure) {
  //     // redraw
  //     vc->wl.wait_for_configure = false;
  //  }
}

int32_t current_width = 500;
int32_t current_height = 500;
int times_changed = 0;

void handle_xdg_toplevel_configure(void *data, struct xdg_toplevel *toplevel,
                                   int32_t width, int32_t height,
                                   struct wl_array *states) {
  current_width = width;
  current_height = height;
  // std::cout << "New dims:  " << width << "x" << height << std::endl;
}

void handle_xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel) {}

struct wl_registry_listener registry_listener = {
    .global = registry_global_handler,
    .global_remove = registry_global_remove_handler};

struct xdg_toplevel_listener xdg_toplevel_listener = {
    handle_xdg_toplevel_configure,
    handle_xdg_toplevel_close,
};

struct xdg_surface_listener xdg_surface_listener = {
    handle_xdg_surface_configure,
};
class WaylandInstanceHandler : public Handler {
public:
  void claim_pixel_format(uint64_t px) {
    // debug << "px: 0x << end;
    // switch (px) {
    //   case WL_SHM_FORMAT_ARGB8888:
    //     claim({{{"global wayland shm"}, {"supports pixel format"}, {"ARGB
    //     8x8x8x8"}, {"encoded as"}, {px}}}); return;
    //   case WL_SHM_FORMAT_XRGB8888:
    //     claim({{{"global wayland shm"}, {"supports pixel format"}, {"XRGB
    //     8x8x8x8"}, {"encoded as"}, {px}}}); return;
    //   case WL_SHM_FORMAT_XBGR8888:
    //     claim({{{"global wayland shm"}, {"supports pixel format"}, {"XBGR
    //     8x8x8x8"}, {"encoded as"}, {px}}}); return;
    //   case WL_SHM_FORMAT_ABGR8888:
    //     claim({{{"global wayland shm"}, {"supports pixel format"}, {"ABGR
    //     8x8x8x8"}, {"encoded as"}, {px}}}); return;
    //   case WL_SHM_FORMAT_XRGB2101010:
    //     claim({{{"global wayland shm"}, {"supports pixel format"}, {"XRGB
    //     2x10x10x10"}, {"encoded as"}, {px}}}); return;
    //   case WL_SHM_FORMAT_XBGR2101010:
    //     claim({{{"global wayland shm"}, {"supports pixel format"}, {"XBGR
    //     2x10x10x10"}, {"encoded as"}, {px}}}); return;
    //   case WL_SHM_FORMAT_ARGB2101010:
    //     claim({{{"global wayland shm"}, {"supports pixel format"}, {"ARGB
    //     2x10x10x10"}, {"encoded as"}, {px}}}); return;
    //   case WL_SHM_FORMAT_ABGR2101010:
    //     claim({{{"global wayland shm"}, {"supports pixel format"}, {"ABGR
    //     2x10x10x10"}, {"encoded as"}, {px}}}); return;
    //   default:
    //     return;
    // }
  }
  void register_initial_tuples() override {}

  bool display_dispatch_setup = false;
  bool poll() override {
    auto r = wl_display_dispatch_pending(display);
    // std::cout << "Poll! Does wl have dispatch pending? " << r << std::endl;
    display_dispatch_setup = true;
    return r > 0;
  }

  ~WaylandInstanceHandler() {
    wl_display_disconnect(display);
    pixel_formats.clear();
  }

protected:
  void handle(const std::vector<Tuple> &queryResults) override {
    if (!display_dispatch_setup) {
      debug << "Connecting to display" << end;

      display = wl_display_connect(nullptr);
      if (display == nullptr) {
        err << "Wayland display failed to connect" << end;
        return;
      }
      debug << "any error from wl_display? " << wl_display_get_error(display)
            << end;
      display_fd = wl_display_get_fd(display);
      registry = wl_display_get_registry(display);

      wl_registry_add_listener(registry, &registry_listener, nullptr);
      wl_display_roundtrip(display);
      wl_registry_destroy(registry);

      surface = wl_compositor_create_surface(compositor);
      ft_xdg_surface = xdg_wm_base_get_xdg_surface(ft_xdg_wm_base, surface);
      xdg_surface_add_listener(ft_xdg_surface, &xdg_surface_listener, nullptr);
      ft_xdg_toplevel = xdg_surface_get_toplevel(ft_xdg_surface);

      xdg_toplevel_add_listener(ft_xdg_toplevel, &xdg_toplevel_listener,
                                nullptr);
      xdg_toplevel_set_title(ft_xdg_toplevel, "Testing from Foxtalk");
      wl_surface_commit(surface);
    } else {
       wl_display_prepare_read(display);
       wl_display_read_events(display);
    }
    if (display != nullptr && surface != nullptr) {

      claim({{{display},
              {"is a"},
              {"wayland display"},
              {"with wl_surface"},
              {surface}}});
    }

    claim({{
        {"available surface has width"},
        {(uint64_t)current_width},
        {"and height"},
        {(uint64_t)current_height},
    }});
  }

  void free_tuple(const Tuple &t) override {}

  void init() override { claim({{{"foxtalk"}, {"is"}, {"running"}}}); }
};

FOXTALK_FFI_HANDLER_REG(WaylandInstanceHandler);