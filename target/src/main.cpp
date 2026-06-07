#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <string>
#include <cstring>
#include <cmath>

#include "data.h"
#include "paths.h"

struct SharedState {
    std::mutex          mtx;
    ControllerState     state   = {};
    bool                connected = false;
    std::string         deckIP;
    std::string         deckID;
    uint64_t            lastPacketTime = 0; // SDL_GetTicks()
};

static SharedState g_shared;
static std::atomic<bool> g_running{true};

void NetworkThread() {
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Non-blocking
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    sockaddr_in serverAddr{};
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_port        = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (sockaddr*)&serverAddr, sizeof(serverAddr));

    sockaddr_in clientAddr{};
    socklen_t   addrLen = sizeof(clientAddr);
    Packet      packet{};

    while (g_running) {
        int bytes = recvfrom(sock, &packet, sizeof(packet), 0,
                             (sockaddr*)&clientAddr, &addrLen);
        if (bytes > 0) {
            if (packet.type == PACKET_BROADCAST) {
                Packet ack = {PACKET_BROADCAST, "PC"};
                sendto(sock, &ack, sizeof(ack), 0,
                       (sockaddr*)&clientAddr, addrLen);

                std::lock_guard<std::mutex> lock(g_shared.mtx);
                g_shared.connected = true;
                g_shared.deckIP    = inet_ntoa(clientAddr.sin_addr);
                g_shared.deckID    = packet.id;
            }
            else if (packet.type == PACKET_INPUT_STATE) {
                std::lock_guard<std::mutex> lock(g_shared.mtx);
                g_shared.state         = packet.state;
                g_shared.lastPacketTime = SDL_GetTicks();
            }
        }
        else {
            SDL_Delay(1);
        }
    }

    close(sock);
}


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

    ImFont* fontDefault = io.Fonts->AddFontFromFileTTF(regularFontPath.c_str(), 16);
    ImFont* fontBig     = io.Fonts->AddFontFromFileTTF(boldFontPath.c_str(), 24);

    std::thread netThread(NetworkThread);

    while (true) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            ImGui_ImplSDL3_ProcessEvent(&ev);
            if (ev.type == SDL_EVENT_QUIT) { g_running = false; goto done; }
        }

        // Snapshot shared state
        ControllerState state;
        bool   connected;
        std::string deckIP, deckID;
        uint64_t lastPkt;
        {
            std::lock_guard<std::mutex> lock(g_shared.mtx);
            state     = g_shared.state;
            connected = g_shared.connected;
            deckIP    = g_shared.deckIP;
            deckID    = g_shared.deckID;
            lastPkt   = g_shared.lastPacketTime;
        }

        bool signalFresh = connected && (SDL_GetTicks() - lastPkt < 500);

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("##root", nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoResize     |
            ImGuiWindowFlags_NoBringToFrontOnFocus);

        ImGui::PushFont(fontBig);
        ImGui::Text("DECK MONITOR");
        ImGui::PopFont();
        ImGui::SameLine(0, 16);

        // Connection status LED + text
        //DrawLED(connected, {0.0f, 0.9f, 0.4f, 1.0f});
        ImGui::SameLine(0, 6);
        if (connected) {
            ImGui::TextColored({0.0f, 0.9f, 0.4f, 1.0f},
                "CONNECTED  —  %s  (%s)", deckID.c_str(), deckIP.c_str());
        } else {
            ImGui::TextColored({0.6f, 0.6f, 0.6f, 1.0f},
                "Waiting for Deck on :8080 ...");
        }

        if (connected) {
            ImGui::SameLine();
            //DrawLED(signalFresh, {0.0f, 0.6f, 1.0f, 1.0f});
            ImGui::SameLine(0, 4);
            ImGui::TextDisabled(signalFresh ? "LIVE" : "STALE");
        }

        ImGui::Separator();
        ImGui::Spacing();

        ImGui::BeginChild("##sticks", {0, 120}, false);
        //DrawStick("LEFT",  NormAxis(state.leftX),  NormAxis(state.leftY));
        ImGui::SameLine(0, 24);
        //DrawStick("RIGHT", NormAxis(state.rightX), NormAxis(state.rightY));

        ImGui::SameLine(0, 24);
        ImGui::BeginGroup();
        ImGui::TextDisabled("AXES (raw)");
        ImGui::Spacing();
        ImGui::Text("LX  %+6d", state.leftX);
        ImGui::Text("LY  %+6d", state.leftY);
        ImGui::Text("RX  %+6d", state.rightX);
        ImGui::Text("RY  %+6d", state.rightY);
        ImGui::EndGroup();
        ImGui::EndChild();

        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextDisabled("TRIGGERS");
        ImGui::Spacing();
        //DrawTrigger("LT", state.leftTrigger);
        ImGui::SameLine(0, 24);
        //DrawTrigger("RT", state.rightTrigger);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextDisabled("BUTTONS  (bitmask: %04d)", state.buttons);
        ImGui::Spacing();

        for (int i = 0; i < 4; i++) {
            bool pressed = (state.buttons >> i) & 1;
            ImVec4 col = pressed ? ImVec4(0.54f, 0.43f, 0.54f, 1.0f) : ImVec4(0.18f, 0.18f, 0.22f, 1.0f);
            ImVec4 text = pressed ? ImVec4(0,0,0,1) : ImVec4(0.4f,0.4f,0.5f,1.0f);

            ImGui::PushStyleColor(ImGuiCol_Button,        col);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, col);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  col);
            ImGui::PushStyleColor(ImGuiCol_Text,          text);
            ImGui::PopStyleColor(4);

            if (i < 3) ImGui::SameLine(0, 8);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextDisabled("UDP :8080  |  16ms poll  |  %.0f fps",
                            io.Framerate);

        ImGui::End();

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

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}