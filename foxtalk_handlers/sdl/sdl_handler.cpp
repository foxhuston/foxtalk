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
#include <tuple>
#include <vector>

class SdlHandler : public Handler {
public:
  SDL_Window *window;
  SDL_Renderer *renderer;
  uint8_t image_data_rgb[1920 * 1080 * 3];
  cv::Ptr<cv::SimpleBlobDetector> blob;


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
    // temp for easy testing: Remove if you want to move the window
    // SDL_SetWindowPosition(window, 0, 0);


    if (queryResults.size() == 0) {
      SDL_SetRenderDrawColor(renderer, 30, 30, 30, SDL_ALPHA_OPAQUE);
      SDL_RenderClear(renderer);
      SDL_RenderPresent(renderer);
      std::cout << "No query results in sdl handler" << std::endl;
      //if(SDL_WindowHasSurface(window)) {
        // auto channels = 3;
        // SDL_Surface *camera_surface = SDL_CreateSurfaceFrom(
        //                 1920,
        //                 1080,
        //                 SDL_PIXELFORMAT_RGB24,
        //                 image_data_rgb,
        //                 1920 * channels      // pitch
        //                 );

        //   auto window_surface = SDL_GetWindowSurface(window);
        // std::cout << "Window surface in |q|=0 is: " << window_surface << std::endl;
        //   SDL_Rect camera_rect {0, 0, 1920, 1080};
        //   SDL_Rect window_rect {};
        //   auto xs = SDL_GetWindowSize(window, &window_rect.w, &window_rect.h);
        //   SDL_BlitSurface(camera_surface, &camera_rect, window_surface, &window_rect);

        //   SDL_UpdateWindowSurface(window);
        //}
        return;
      }

    assert(queryResults.size() == 1);

    auto q = queryResults[0];
    auto camera = q.at<std::string>(0).value();
    auto pixel_format = q.at<uint64_t>(2).value();
    auto width = q.at<uint64_t>(4).value();
    auto height = q.at<uint64_t>(6).value();
    // auto image_buffer = q.at<std::vector<uint8_t>>(8).value();
    auto image_buffer = q.at<void *>(8).value();
    auto frame_count_num = q.at<uint64_t>(12).value();
    // std::cout << frame_count_num << std::endl;

    // auto t = queryResults[0];
    // auto x = t.at<uint64_t>(4).value();
    // auto y = t.at<uint64_t>(6).value();
    // auto width = t.at<uint64_t>(8).value();
    // auto height = t.at<uint64_t>(10).value();



    // SDL_SetRenderDrawColor(renderer, 30, 30, 30, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    SDL_Surface *camera_surface = SDL_CreateSurfaceFrom(
                    width,
                    height,
            SDL_PIXELFORMAT_YUY2,
                    image_buffer,
                    width * 2
                    );

    auto rgb_camera_surface = SDL_ConvertSurface(camera_surface, SDL_PIXELFORMAT_RGB24);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, rgb_camera_surface);
    SDL_DestroySurface(camera_surface);
    SDL_RenderTexture(renderer, texture, nullptr, nullptr); // These nulls can be the rects

    cv::Mat img(height, width, CV_8UC3, rgb_camera_surface->pixels);
    cv::Mat gray, jet;
    cv::cvtColor(img, gray, cv::COLOR_RGB2GRAY);

    cv::GaussianBlur(gray, gray, {5, 5}, 4);

    std::vector<cv::KeyPoint> keypoints;
    blob->detect(gray, keypoints);

    // // std::cout << "Found " << keypoints.size() << " keypoint(s)" << std::endl;

    SDL_SetRenderDrawColor(renderer, 255, 30, 30, 127);
    for(auto kp : keypoints) {
      const SDL_FRect rect(kp.pt.x, kp.pt.y, kp.size, kp.size);
      SDL_RenderFillRect(renderer, &rect);
    }


    // auto window_surface = SDL_GetWindowSurface(window);
    // SDL_Rect camera_rect {0, 0, (int)width, (int)height};
    // SDL_Rect window_rect {};
    // auto xs = SDL_GetWindowSize(window, &window_rect.w, &window_rect.h);
    // SDL_BlitSurface(rgb_camera_surface, &camera_rect, window_surface, &window_rect);

    // std::cout << "image buffer " << std::endl
    //  << "  width: " << camera_surface->w << std::endl
    //  << "  height: " << camera_surface->h << std::endl
    //  << std::endl;

    // for(int i = 0; i < 32; i++) {
    //   std::cout << (uint32_t)static_cast<uint8_t*>(camera_surface->pixels)[i] << " ";
    // }
    // std::cout << std::endl;

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
      // SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
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


          // claim({{{"foxtalk reactor"}, {"sees tuples"},
          // TupleNoun::query()}});
          // claim({{{"illumination"},
          //         {"rectangle"},
          //         {"at"},
          //         {"x"},
          //         TupleNoun::query(),
          //         {"y"},
          //         TupleNoun::query(),
          //         {"width"},
          //         TupleNoun::query(),
          //         {"height"},
          //         TupleNoun::query()}});

          claim({
            {TupleNoun::query(),
            {"with pixel format"},
            TupleNoun::query(),
            {"with resolution width"},
            TupleNoun::query(),
            {"with resolution height"},
            TupleNoun::query(),
            {"has image"},
            TupleNoun::prefix(),
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