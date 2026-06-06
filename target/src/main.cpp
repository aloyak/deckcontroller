#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <string>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <imgui_impl_sdlrenderer3.cpp>

#include "paths.h"

// TODO: fix ini file

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // TODO: check this
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImFont* font = io.Fonts->AddFontFromFileTTF(resource_path("fonts/Inter_24pt-Regular.ttf").string().c_str(), 24.0f);
    if (font != nullptr) {
        io.FontDefault = font;
    }

    
    SDL_Window* window = SDL_CreateWindow("DeckController", 1200, 800, SDL_WINDOW_RESIZABLE);
    if (!window) {
        ImGui::DestroyContext();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        SDL_DestroyWindow(window);
        ImGui::DestroyContext();
        SDL_Quit();
        return 1;
    }
    
    SDL_SetRenderVSync(renderer, 1);
    
    if (!ImGui_ImplSDL3_InitForSDLRenderer(window, renderer)) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        ImGui::DestroyContext();
        SDL_Quit();
        return 1;
    }
    
    if (!ImGui_ImplSDLRenderer3_Init(renderer)) {
        ImGui_ImplSDL3_Shutdown();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        ImGui::DestroyContext();
        SDL_Quit();
        return 1;
    }
    
    ImGui::StyleColorsDark();
    //io.IniFilename = resource_path("./resources/layout.ini").string().c_str();

    SDL_Gamepad* gamepad = nullptr;
    bool quit = false;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT) {
                quit = true;
            }
        }

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(ImGui::GetID("MainDockSpace"), ImGui::GetMainViewport());

        ImGui::Begin("Active Controllers");
        ImGui::End();

        ImGui::Begin("Controller Inputs");
        ImGui::End();

        ImGui::Render();

        SDL_SetRenderDrawColor(renderer, 24, 24, 28, 255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }
    
    if (gamepad) {
        SDL_CloseGamepad(gamepad);
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}