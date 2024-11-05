// pkg-config: sdl3
#include <SDL3/SDL.h>

#include <SDL3/SDL_init.h>
#include <cstdint>
#include <iostream>

#include <foxtalk_handler.hpp>

class SdlHandler : public Handler {
private:
  SDL_Window *gWindow{nullptr};

  // The surface contained by the window
  SDL_Surface *gScreenSurface{nullptr};

  // The image we will load and show on the screen
  SDL_Surface *gHelloWorld{nullptr};

public:
  SDL_Window *window;
  SDL_Renderer *renderer;

  pthread_mutex_t mutex;

  void handle(const std::vector<Tuple> &queryResults) override {
    if(queryResults.size() == 0) { return; }
    assert(queryResults.size() == 1);

    uint64_t clock_count = queryResults[0].at<uint64_t>(2).value();
    auto red = (clock_count) % 256;

    SDL_SetRenderDrawColor(renderer, red, 0, 127, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
  }

  void init() override {
    std::cout << "Hello from SDL Handler!" << std::endl;

    if (SDL_Init(SDL_INIT_VIDEO)) {
      window =
          SDL_CreateWindow("Foxtalk Debug??", 352, 430, SDL_WINDOW_RESIZABLE);
      if (window) {
        renderer = SDL_CreateRenderer(window, NULL);
        if (renderer) {
          SDL_ShowWindow(window);
          {
            int width, height, bbwidth, bbheight;
            SDL_GetWindowSize(window, &width, &height);
            SDL_GetWindowSizeInPixels(window, &bbwidth, &bbheight);
            SDL_Log("Window size: %ix%i", width, height);
            SDL_Log("Backbuffer size: %ix%i", bbwidth, bbheight);
            if (width != bbwidth) {
              SDL_Log("This is a highdpi environment.");
            }
          }

          // claim({{{"foxtalk reactor"}, {"sees tuples"}, TupleNoun::query()}});
          claim({{{"clock"}, {"is at"}, TupleNoun::query()}});
        } else {
          std::cerr << "Couldn't initialize renderer!" << std::endl;
        }
      } else {
        std::cerr << "Couldn't initialize window!" << std::endl;
      }
    } else {
      std::cerr << "Couldn't initialize SDL(SDL_INIT_VIDEO)!" << std::endl;
    }
  }

  ~SdlHandler() {
    std::cout << "SdlHandler destructor called..." << std::endl;
    SDL_Quit();
  }
};

FOXTALK_FFI_HANDLER_REG(SdlHandler);