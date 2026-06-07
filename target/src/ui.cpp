#include "ui.h"

#include <SDL3/SDL.h>

#include <algorithm> // for clamp

namespace {
constexpr uint64_t kSignalTimeoutMs = 500;

void DrawStickOverlay(ImDrawList* drawList, const ImVec2& topLeft, const ImVec2& imageSize,
					  float centerX, float centerY, float radius,
					  int16_t rawX, int16_t rawY, bool pressed, ImVec2 offset = {0, 0}) {
    const float normX = std::clamp(static_cast<float>(rawX) / 32768.0f, -1.0f, 1.0f);
    const float normY = std::clamp(static_cast<float>(rawY) / 32768.0f, -1.0f, 1.0f);

    const ImVec2 center{
        topLeft.x + imageSize.x * centerX + offset.x,
        topLeft.y + imageSize.y * centerY + offset.y,
    };
    const ImVec2 knob{
        center.x + normX * radius,
        center.y + normY * radius,
    };

    const ImU32 fillColor = pressed ? ImGui::GetColorU32(ImVec4(0.7f, 0.7f, 0.7f, 0.9f)) : ImGui::GetColorU32(ImVec4(0.45f, 0.45f, 0.95f, 0.9f));
    drawList->AddCircleFilled(knob, radius * 0.28f, fillColor, 16);
}

void DrawButtonOverlay(ImDrawList* drawList, const ImVec2& topLeft, const ImVec2& imageSize,
                       float centerX, float centerY, float radius, bool pressed, ImVec2 offset = {0, 0}) {
    const ImVec2 center{
        topLeft.x + imageSize.x * centerX + offset.x,
        topLeft.y + imageSize.y * centerY + offset.y,
    };

    const ImU32 fillColor = pressed
        ? ImGui::GetColorU32(ImVec4(0.45f, 0.45f, 0.95f, 0.9f))
        : ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    drawList->AddCircleFilled(center, radius * 0.28f, fillColor, 16);
}

void DrawTriggerOverlay(ImDrawList* drawList, const ImVec2& topLeft, const ImVec2& imageSize,
					   float centerX, float centerY, float widthRatio, float heightRatio,
					   uint8_t value, ImVec2 offset = {0, 0}) {
    const ImVec2 trackMin{
        topLeft.x + imageSize.x * centerX - imageSize.x * widthRatio * 0.5f + offset.x,
        topLeft.y + imageSize.y * centerY - imageSize.y * heightRatio * 0.5f + offset.y,
    };
    const ImVec2 trackMax{
        topLeft.x + imageSize.x * centerX + imageSize.x * widthRatio * 0.5f + offset.x,
        topLeft.y + imageSize.y * centerY + imageSize.y * heightRatio * 0.5f + offset.y,
    };

    const float fillRatio = static_cast<float>(value) / 255.0f;
    const ImVec2 fillMax{
        trackMin.x + (trackMax.x - trackMin.x) * fillRatio,
        trackMax.y,
    };
    const ImVec2 knobCenter{
        fillMax.x,
        (trackMin.y + trackMax.y) * 0.5f,
    };

    const ImU32 trackColor = ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.40f));
    const ImU32 fillColor = ImGui::GetColorU32(ImVec4(0.45f, 0.45f, 0.95f, 0.85f));

    drawList->AddRectFilled(trackMin, trackMax, trackColor, 999.0f);
    drawList->AddRectFilled(trackMin, fillMax, fillColor, 999.0f);
}
}

bool IsSignalFresh(const ControllerSnapshot& snapshot) {
    return snapshot.connected && (SDL_GetTicks() - snapshot.lastPacketTime < kSignalTimeoutMs);
}

void DrawCenteredMessage(ImGuiIO& io, const char* message, ImVec2 offset = {0, 0}, ImFont* font = nullptr) {
    const ImVec2 textSize = ImGui::CalcTextSize(message);
    const ImVec2 windowSize = io.DisplaySize;
    ImGui::SetCursorPos({(windowSize.x - textSize.x) * 0.5f + offset.x,
                         (windowSize.y - textSize.y) * 0.5f + offset.y});
    if (font) {
        ImGui::PushFont(font);
    }
    ImGui::TextUnformatted(message);
    if (font) {
        ImGui::PopFont();
    }
}

void RenderControllerWindow(const ControllerSnapshot& snapshot, ImGuiIO& io,
							const UiFonts& fonts, SDL_Texture* controllerTexture,
							ImVec2 controllerTextureSize) {
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("##root", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoBringToFrontOnFocus);

    if (!snapshot.connected) {
        DrawCenteredMessage(io, "Deck Controller", {-20, -30}, fonts.bold);
        DrawCenteredMessage(io, "Waiting for Steam Deck on port 8080...");
        ImGui::End();
        return;
    }

    const bool signalFresh = IsSignalFresh(snapshot);

    ImGui::PushFont(fonts.bold);
    ImGui::TextUnformatted("DECK MONITOR");
    ImGui::PopFont();

    ImGui::TextColored(ImVec4(0.0f, 0.9f, 0.4f, 1.0f), "CONNECTED:  %s  (%s)", snapshot.deckID.c_str(), snapshot.deckIP.c_str());
    ImGui::SameLine(0, 4);
    ImGui::TextDisabled(signalFresh ? "LIVE" : "DISCONNECTED");

    if (controllerTexture != nullptr && controllerTextureSize.x > 0.0f && controllerTextureSize.y > 0.0f) {
        ImVec2 imageSize = controllerTextureSize;
        const float maxWidth = io.DisplaySize.x * 0.78f;
        if (imageSize.x > maxWidth) {
            const float scale = maxWidth / imageSize.x;
            imageSize.x *= scale;
            imageSize.y *= scale;

        }

        ImGui::Spacing();
        ImGui::SetCursorPosX((io.DisplaySize.x - imageSize.x) * 0.5f);
        ImGui::Image(controllerTexture, imageSize);

        // Overlays

        const ImVec2 imageTopLeft = ImGui::GetItemRectMin();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        DrawStickOverlay(drawList, imageTopLeft, imageSize, 0.368f, 0.399f, imageSize.x * 0.055f,
                 snapshot.state.leftX, snapshot.state.leftY, (snapshot.state.buttons >> 6) & 1, {5, -7});
        DrawStickOverlay(drawList, imageTopLeft, imageSize, 0.601f, 0.399f, imageSize.x * 0.055f,
                 snapshot.state.rightX, snapshot.state.rightY, (snapshot.state.buttons >> 7) & 1, {5, -7});

        DrawTriggerOverlay(drawList, imageTopLeft, imageSize, 0.250f, 0.1f, 0.1f, 0.03f,
                 snapshot.state.leftTrigger, {0, -4});
        DrawTriggerOverlay(drawList, imageTopLeft, imageSize, 0.725f, 0.1f, 0.1f, 0.03f,
                 snapshot.state.rightTrigger, {0, -4});

        DrawButtonOverlay(drawList, imageTopLeft, imageSize, 0.697f, 0.305f, imageSize.x * 0.075f,
                          (snapshot.state.buttons >> 0) & 1, {20, 20}); // A
        DrawButtonOverlay(drawList, imageTopLeft, imageSize, 0.697f, 0.305f, imageSize.x * 0.075f,
                          (snapshot.state.buttons >> 1) & 1, {40, 0}); // B
        DrawButtonOverlay(drawList, imageTopLeft, imageSize, 0.697f, 0.305f, imageSize.x * 0.075f,
                          (snapshot.state.buttons >> 2) & 1, {0, 0}); // X
        DrawButtonOverlay(drawList, imageTopLeft, imageSize, 0.697f, 0.305f, imageSize.x * 0.075f,
                          (snapshot.state.buttons >> 3) & 1, {20, -20}); // Y

        // D-pad
        DrawButtonOverlay(drawList, imageTopLeft, imageSize, 0.295f, 0.305f, imageSize.x * 0.05f,
                          (snapshot.state.buttons >> 8) & 1, {-20, -18}); // UP
        DrawButtonOverlay(drawList, imageTopLeft, imageSize, 0.295f, 0.305f, imageSize.x * 0.05f,
                          (snapshot.state.buttons >> 9) & 1, {-20, 18}); // DOWN
        DrawButtonOverlay(drawList, imageTopLeft, imageSize, 0.295f, 0.305f, imageSize.x * 0.05f,
                          (snapshot.state.buttons >> 10) & 1, {-36, 0}); // LEFT
        DrawButtonOverlay(drawList, imageTopLeft, imageSize, 0.295f, 0.305f, imageSize.x * 0.05f,
                          (snapshot.state.buttons >> 11) & 1, {0, 0}); // RIGHT

        // Shoulder buttons
        DrawButtonOverlay(drawList, imageTopLeft, imageSize, 0.25f, 0.05f, imageSize.x * 0.06f,
                          (snapshot.state.buttons >> 4) & 1, {0, 32}); // LB
        DrawButtonOverlay(drawList, imageTopLeft, imageSize, 0.75f, 0.05f, imageSize.x * 0.06f,
                          (snapshot.state.buttons >> 5) & 1, {0, 32}); // RB

        // Menu + Start
        DrawButtonOverlay(drawList, imageTopLeft, imageSize, 0.5f, 0.1f, imageSize.x * 0.06f,
                          (snapshot.state.buttons >> 12) & 1, {-64, 42}); // MENU
        DrawButtonOverlay(drawList, imageTopLeft, imageSize, 0.5f, 0.1f, imageSize.x * 0.06f,
                          (snapshot.state.buttons >> 14) & 1, {60, 42}); // START
    }

    ImGui::Separator();
    ImGui::Spacing();

    ImGui::BeginChild("##sticks", {0, 120}, false);
    ImGui::BeginGroup();
    ImGui::TextDisabled("AXES (raw)");
    ImGui::Spacing();
    ImGui::Text("LX  %+6d", snapshot.state.leftX);
    ImGui::Text("LY  %+6d", snapshot.state.leftY);
    ImGui::Text("RX  %+6d", snapshot.state.rightX);
    ImGui::Text("RY  %+6d", snapshot.state.rightY);
    ImGui::EndGroup();
    ImGui::EndChild();

    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextDisabled("TRIGGERS");
    ImGui::Spacing();
    ImGui::Text("LT  %3u", snapshot.state.leftTrigger);
    ImGui::Text("RT  %3u", snapshot.state.rightTrigger);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Buttons grid (bits 0..14)
    ImGui::TextDisabled("BUTTONS  (bitmask: %04u)", snapshot.state.buttons);
    ImGui::Spacing();
    const char* btnLabels[14] = {
        "A", "B", "X", "Y",    // 0-3
        "LB", "RB", "LSTICK", "RSTICK", // 4-7
        "DPAD_UP", "DPAD_DOWN", "DPAD_LEFT", "DPAD_RIGHT", // 8-11
        "MENU", "START" // 12-14
    };
    const int cols = 4;
    const ImVec2 btnSize = {72, 36};
    for (int i = 0; i < 14; ++i) {
        const bool pressed = (snapshot.state.buttons >> i) & 1;
        const ImVec4 col = pressed ? ImVec4(0.54f, 0.43f, 0.54f, 1.0f)
                                   : ImVec4(0.18f, 0.18f, 0.22f, 1.0f);
        const ImVec4 text = pressed ? ImVec4(0, 0, 0, 1) : ImVec4(0.4f, 0.4f, 0.5f, 1.0f);

        ImGui::PushStyleColor(ImGuiCol_Button, col);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, col);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, col);
        ImGui::PushStyleColor(ImGuiCol_Text, text);
        ImGui::Button(btnLabels[i], btnSize);
        ImGui::PopStyleColor(4);

        if ((i + 1) % cols != 0 && i != 14) {
            ImGui::SameLine(0, 8);
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextDisabled("D-PAD");

    const char* dpadStates[4] = {
        (snapshot.state.buttons >> 8) & 1 ? "UP" : "up",
        (snapshot.state.buttons >> 9) & 1 ? "DOWN" : "down",
        (snapshot.state.buttons >> 10) & 1 ? "LEFT" : "left",
        (snapshot.state.buttons >> 11) & 1 ? "RIGHT" : "right",
    };

    ImGui::Text("%s  %s  %s  %s", dpadStates[0], dpadStates[1], dpadStates[2], dpadStates[3]);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextDisabled("UDP: 8080  |  %.0f FPS", io.Framerate);

    ImGui::End();
}
