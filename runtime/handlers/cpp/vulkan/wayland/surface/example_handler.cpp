#include "foxtalk_tuple.h"
#include <foxtalk_handler.hpp>
#include <wayland-client-protocol.h>

class WaylandSurfaceCreationHandler : public Handler
{
public:
protected:
  void handle(const std::vector<Tuple> &queryResults) override {
    if (queryResults.size() != 1) {
      return;
    }

    auto compositor = static_cast<wl_compositor*>(queryResults[0].at<void* >(0).value());
    struct wl_surface *surface = wl_compositor_create_surface(compositor);

  }

  void init() override {
    claim({{TupleNoun::query(), {"is the"}, {"global wayland compositor"}}});
    claim({{
      TupleNoun::query(),
      {"wishes for"},
      {"a wayland surface"},
      TupleNoun::prefix()
      }});
  }
};

FOXTALK_FFI_HANDLER_REG(WaylandSurfaceCreationHandler);