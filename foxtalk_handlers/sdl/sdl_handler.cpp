// //foxtalk-link sdl3
// #include <SDL3/SDL.h> 

// #include <iostream>
// #include <thread>
// #include <atomic> 
// #include <string>
 
// #include <foxtalk_handler.hpp>
 
// class SdlHandler : public Handler
// {
//   // std::atomic<bool> running(true); 
//   // std::string displayText = "Hello, World!";

//   // int main(int argc, char* argv[]) {
//       // Initialize SDL
      

//       // Load font
//       // TTF_Font* font = TTF_OpenFont("path/to/font.ttf", 24);
//       // if (!font) {
//       //     std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
//       //     return 1;
//       // }

//       // Start the text update thread
//       // std::thread updater(updateText);

//   //     // Main loop
//   //     SDL_Event e;
//   //     while (running) {
//   //         while (SDL_PollEvent(&e) != 0) {
//   //             if (e.type == SDL_QUIT) {
//   //                 running = false;
//   //             }
//   //         }

//   //         // Clear screen
//   //         SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
//   //         SDL_RenderClear(renderer);

//   //         // Render text
//   //         SDL_Color textColor = {0, 0, 0, 255};
//   //         SDL_Surface* textSurface = TTF_RenderText_Solid(font, displayText.c_str(), textColor);
//   //         SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
//   //         SDL_Rect renderQuad = { 50, 50, textSurface->w, textSurface->h };
//   //         SDL_RenderCopy(renderer, textTexture, NULL, &renderQuad);

//   //         // Update screen
//   //         SDL_RenderPresent(renderer);

//   //         // Clean up
//   //         SDL_FreeSurface(textSurface);
//   //         SDL_DestroyTexture(textTexture);
//   //     }

//   //     // Clean up
//   //     running = false;
//   //     updater.join();
//   //     // TTF_CloseFont(font);
//   //     SDL_DestroyRenderer(renderer);
//   //     SDL_DestroyWindow(window);
//   //     // TTF_Quit();
//   //     SDL_Quit();

//   //     return 0;
//   // }
// protected:

//   SDL_Window* window;
//   SDL_Renderer* renderer;

//   pthread_mutex_t mutex;

//   void handle(const std::vector<Tuple> &queryResults) override {
    
//   }

//   void* render_thread(void* arg) {

//     if (!SDL_Init(SDL_INIT_VIDEO)) {
//       std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
//     }

//     window = SDL_CreateWindow("SDL3 Hello World", 640, 480, SDL_WINDOW_OPENGL);
//     if (!window) {
//       std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
//     }

//     renderer = SDL_CreateRenderer(window, NULL);
//     if (!renderer) {
//       std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
//     }

//     while (1) {
//         // Lock mutex to safely update shared resources
//         pthread_mutex_lock(&mutex);


//         pthread_mutex_unlock(&mutex);
//         SDL_Delay(16); // Simulate frame delay
//     }
//     return NULL;
//   }

//   void init() override {

//     // sdlThread = std::thread(sdlWindowThread);
//     // display_text = "Hello world";
//     // is_updating.store(false);
    

//     pthread_t thread;
//     pthread_mutex_init(&mutex, NULL);

//     // Start rendering thread
//     pthread_create(&thread, NULL, render_thread, NULL);

//     claim({{{"foxtalk reactor"}, {"sees tuples"}, TupleNoun::query()}});
//   }
// };

// FOXTALK_FFI_HANDLER_REG(SdlHandler);