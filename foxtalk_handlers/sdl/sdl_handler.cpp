// pkg-config: sdl3 opencv4
// cppstd: 23


#include "foxtalk_tuple.h"
#include "opencv2/core/base.hpp"
#include "opencv2/core/hal/interface.h"
#include <SDL3/SDL.h>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>


#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>


#include <cstdint>
#include <cstdlib>
#include <iostream>

#include <foxtalk_handler.hpp>
#include <string>
#include <tuple>
#include <vector>

class SdlHandler : public Handler {
public:
  SDL_Window *window;
  SDL_Renderer *renderer;
  uint8_t image_data_rgb[1920 * 1080 * 3];
  cv::Ptr<cv::SimpleBlobDetector> blob;

 uint64_t last_frame_rendered = 0;

  bool poll() override {
    if (window == nullptr) {
      return false;
    }
    return SDL_PollEvent(nullptr);
  }

  void handle(const std::vector<Tuple> &queryResults) override {
    SDL_Event event {};
    while(SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
          claim({{{"mouse"}, {"button"}, {(uint64_t)event.button.button}}});
        break;

        case SDL_EVENT_QUIT:
          exit(0);
          break;

        case SDL_EVENT_MOUSE_MOTION:
          claim({{{"mouse"}, {"is"}, {"at"},
            {"x"}, {(uint64_t)event.motion.x},
            {"y"}, {(uint64_t)event.motion.y},
          }});
        break;

        default:
          break;
      }
    }

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    bool skip_cam_image = false;
    for(auto q : queryResults) {
      if(q.at<std::string>(0).has_value() && q.at<std::string>(0).value() == "cv debug image") {
        skip_cam_image = true;
        auto image_buffer = q.at<std::vector<uint8_t>>(1).value();
        auto width = q.at<uint64_t>(3).value();
        auto height = q.at<uint64_t>(5).value();

          SDL_Surface *camera_surface = SDL_CreateSurfaceFrom(
                      width,
                      height,
              SDL_PIXELFORMAT_RGB24,
                      image_buffer.data(),
                      width * 3
                      );


          auto rgb_camera_surface = SDL_ConvertSurface(camera_surface, SDL_PIXELFORMAT_RGB24);

          SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, rgb_camera_surface);
          SDL_RenderTexture(renderer, texture, nullptr, nullptr); // These nulls can be the rects

          SDL_DestroySurface(camera_surface);
          SDL_DestroySurface(rgb_camera_surface);
      }
    }


    // First find the camera image, and draw that as the base.
    if(!skip_cam_image) {
      for(auto q : queryResults) {
        if(q.at<std::string>(1).has_value() && q.at<std::string>(1).value() == "with pixel format") {
          auto camera = q.at<std::string>(0).value();
          auto pixel_format = q.at<uint64_t>(2).value();
          auto width = q.at<uint64_t>(4).value();
          auto height = q.at<uint64_t>(6).value();
          auto image_buffer = q.at<void *>(8).value();
          auto frame_count_num = q.at<uint64_t>(12).value();

          SDL_Surface *camera_surface = SDL_CreateSurfaceFrom(
                      width,
                      height,
              SDL_PIXELFORMAT_YUY2,
                      image_buffer,
                      width * 2
                      );


          auto rgb_camera_surface = SDL_ConvertSurface(camera_surface, SDL_PIXELFORMAT_RGB24);

          SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, rgb_camera_surface);
          SDL_RenderTexture(renderer, texture, nullptr, nullptr); // These nulls can be the rects

          SDL_DestroySurface(camera_surface);
          SDL_DestroySurface(rgb_camera_surface);
          break;
        }
      }
    }

    // Now draw everything else.
    for(auto q : queryResults) {
      if(q.at<std::string>(2).has_value() && q.at<std::string>(2).value() == "has dot at") {
        // it's a dot!
        auto x = q.at<double>(4).value();
        auto y = q.at<double>(6).value();
        auto size = q.at<double>(8).value();

        SDL_SetRenderDrawColor(renderer, 30, 255, 127, SDL_ALPHA_OPAQUE);
        const SDL_FRect rect(x, y, size, size);
        SDL_RenderFillRect(renderer, &rect);
      }
    }


    SDL_RenderPresent(renderer);
    SDL_UpdateWindowSurface(window);
  }

  void init() override {
    std::cout << "Hello from SDL Handler 4!" << std::endl;

    for(int i = 0; i < 1920 * 1080; i++) {
      image_data_rgb[i * 3    ] = 0x00;
      image_data_rgb[i * 3 + 1] = 0xAA;
      image_data_rgb[i * 3 + 2] = 0xFF;
    }

    if (SDL_Init(SDL_INIT_VIDEO)) {
      SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
      window =
          SDL_CreateWindow("Foxtalk Debug", 640, 480, SDL_WINDOW_RESIZABLE);

      if (window) {
        renderer = SDL_CreateRenderer(window, nullptr);
        if (renderer) {

          SDL_ShowWindow(window);
          SDL_SetWindowPosition(window, 1500, 0);

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

          cv::SimpleBlobDetector::Params params {};
          params.minArea = 150;
          params.filterByCircularity = true;
          params.minCircularity = 0.75;

          blob = cv::SimpleBlobDetector::create(params);

          claim({{
            {"cv debug image"},
            TupleNoun::query(),
            {"width"},
            TupleNoun::query(),
            {"height"},
            TupleNoun::query()
          }});

          claim({
            {TupleNoun::query(),
            {"with pixel format"},
            TupleNoun::query(),
            {"with resolution width"},
            TupleNoun::query(),
            {"with resolution height"},
            TupleNoun::query(),
            {"has image"},
            TupleNoun::query(),
            TupleNoun::query(),
            TupleNoun::query(),
            TupleNoun::query(),
            TupleNoun::query(),
          }});

          claim({{
            {"camera"},
            TupleNoun::query(),
            {"has dot at"},
            {"x"},
            TupleNoun::query(),
            {"y"},
            TupleNoun::query(),
            {"size"},
            TupleNoun::query(),
          }});

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