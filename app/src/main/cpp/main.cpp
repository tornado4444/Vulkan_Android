#include "Render.h"

extern "C" int SDL_main(int argc, char* argv[]) {
    LOGI("SDL_main called");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        LOGE("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    if (!SDL_Vulkan_LoadLibrary(nullptr)) {
        LOGE("Failed to load Vulkan library: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Vulkan Triangle", 800, 600,
                                          SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        LOGE("Failed to create window: %s", SDL_GetError());
        return 1;
    }

    Render vulkan(window);

    try {
        vulkan.Init();
        SDL_ShowWindow(window);
        LOGI("App initialized successfully!");
    } catch (const std::exception& e) {
        LOGE("Initialization failed: %s", e.what());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", e.what(), window);
        return 1;
    }

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        try {
            vulkan.drawFrame();
        } catch (const std::exception& e) {
            LOGE("Draw frame failed: %s", e.what());
            break;
        }
    }

    vulkan.cleanup();
    SDL_DestroyWindow(window);
    SDL_Quit();

    LOGI("App quit complete");
    return 0;
}