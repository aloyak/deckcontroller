#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <thread>
#include <atomic>

#include "state.h"
#include "network.h"
#include "paths.h"
#include "ui.h"

#ifndef _WIN32
    #include "linux/driver.h"
#else
    #include "windows/driver.h"
#endif

static SharedState g_shared;
static std::atomic<bool> g_running{true};


int main() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "Deck Controller", 560, 440, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    SDL_SetRenderVSync(renderer, 1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding    = 6.0f;
    style.FrameRounding     = 4.0f;
    style.GrabRounding      = 4.0f;
    style.ItemSpacing       = {10, 8};
    style.FramePadding      = {8,  5};
    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 0.0f;

    ImVec4* c = style.Colors;
    c[ImGuiCol_WindowBg]         = {0.08f, 0.08f, 0.10f, 1.00f};
    c[ImGuiCol_ChildBg]          = {0.11f, 0.11f, 0.14f, 1.00f};
    c[ImGuiCol_FrameBg]          = {0.13f, 0.13f, 0.17f, 1.00f};
    c[ImGuiCol_TitleBg]          = {0.06f, 0.06f, 0.08f, 1.00f};
    c[ImGuiCol_TitleBgActive]    = {0.08f, 0.08f, 0.11f, 1.00f};
    c[ImGuiCol_Header]           = {0.15f, 0.15f, 0.20f, 1.00f};
    c[ImGuiCol_HeaderHovered]    = {0.20f, 0.20f, 0.26f, 1.00f};
    c[ImGuiCol_Separator]        = {0.20f, 0.20f, 0.26f, 1.00f};
    c[ImGuiCol_Text]             = {0.85f, 0.85f, 0.90f, 1.00f};
    c[ImGuiCol_TextDisabled]     = {0.45f, 0.45f, 0.55f, 1.00f};
    c[ImGuiCol_Border]           = {0.22f, 0.22f, 0.28f, 1.00f};

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    auto regularFontPath = resource_path("fonts/Inter_24pt-Regular.ttf").string();
    auto boldFontPath    = resource_path("fonts/Inter_24pt-Bold.ttf").string();

    ImFont* fontRegular = io.Fonts->AddFontFromFileTTF(regularFontPath.c_str(), 18);
    ImFont* fontBold    = io.Fonts->AddFontFromFileTTF(boldFontPath.c_str(), 24);
    io.FontDefault = fontRegular;

    SDL_Texture* controllerTexture = IMG_LoadTexture(renderer, resource_path("controller.png").string().c_str());
    ImVec2 controllerTextureSize = {0.0f, 0.0f};
    if (controllerTexture != nullptr) {
        if (!SDL_GetTextureSize(controllerTexture, &controllerTextureSize.x, &controllerTextureSize.y)) {
            SDL_DestroyTexture(controllerTexture);
            controllerTexture = nullptr;
        }
    }

    std::thread netThread(RunNetworkLoop, std::ref(g_running), std::ref(g_shared), 8080);

    Driver controllerDriver = Driver();

    while (true) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            ImGui_ImplSDL3_ProcessEvent(&ev);
            if (ev.type == SDL_EVENT_QUIT) { g_running = false; goto done; }
        }

        ControllerSnapshot snapshot = SnapshotState(g_shared);
        controllerDriver.Run(snapshot);

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        RenderControllerWindow(snapshot, io, UiFonts{fontRegular, fontBold}, controllerTexture, controllerTextureSize);

        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, 18, 18, 22, 255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

done:
    netThread.join();
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    if (controllerTexture != nullptr) {
        SDL_DestroyTexture(controllerTexture);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}