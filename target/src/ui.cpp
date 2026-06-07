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

    const float panelPadX = 12.0f;
    const float panelPadY = 8.0f;
    const ImVec4 kHeaderCol = ImVec4(0.55f, 0.55f, 0.65f, 1.0f);

    auto SectionHeader = [&](const char* icon, const char* title, const char* subtitle = nullptr) {
        ImGui::Spacing();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + panelPadX);
        ImGui::TextColored(kHeaderCol, "%s  %s", icon, title);
        if (subtitle) {
            ImGui::SameLine();
            ImGui::TextDisabled("-  %s", subtitle);
        }
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + panelPadX);
        ImGui::Spacing();
    };

    auto Padded = [&](auto fn) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + panelPadX);
        fn();
    };

    ImGui::Separator();

    SectionHeader("", "AXES", "normalised ·  range  -32768 → +32767");

    auto DrawAxisBar = [&](const char* label, const char* desc, float norm) {
        constexpr float barWidth  = 160.0f;
        constexpr float barHeight = 10.0f;

        const float fill = (norm + 1.0f) * 0.5f;
        ImDrawList* dl   = ImGui::GetWindowDrawList();

        const ImU32 trackCol  = ImGui::GetColorU32(ImVec4(0.12f, 0.12f, 0.16f, 1.0f));
        const ImU32 fillCol   = ImGui::GetColorU32(ImVec4(0.45f, 0.45f, 0.95f, 0.9f));
        const ImU32 centerCol = ImGui::GetColorU32(ImVec4(0.35f, 0.35f, 0.45f, 1.0f));
        const ImU32 knobCol   = ImGui::GetColorU32(ImVec4(0.75f, 0.75f, 1.0f,  1.0f));

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + panelPadX);
        const ImVec2 cursorPos = ImGui::GetCursorScreenPos();

        const ImVec2 trackMin = cursorPos;
        const ImVec2 trackMax = { cursorPos.x + barWidth, cursorPos.y + barHeight };
        const float  midX     = cursorPos.x + barWidth * 0.5f;
        const float  fillX    = cursorPos.x + barWidth * fill;

        dl->AddRectFilled(trackMin, trackMax, trackCol, barHeight);
        if (fill >= 0.5f)
            dl->AddRectFilled({ midX, trackMin.y }, { fillX, trackMax.y }, fillCol, barHeight);
        else
            dl->AddRectFilled({ fillX, trackMin.y }, { midX, trackMax.y }, fillCol, barHeight);
        dl->AddLine({ midX, trackMin.y }, { midX, trackMax.y }, centerCol, 1.5f);
        dl->AddCircleFilled({ fillX, cursorPos.y + barHeight * 0.5f }, barHeight * 0.65f, knobCol, 12);

        ImGui::Dummy({ barWidth + 4.0f, barHeight });
        ImGui::SameLine(0, 10);
        ImGui::Text("%-4s", label);
        ImGui::SameLine(0, 6);
        ImGui::TextColored(ImVec4(0.45f, 0.45f, 0.95f, 1.0f), "%+6.0f", norm * 32768.0f);
        ImGui::SameLine(0, 10);
        ImGui::TextDisabled("%s", desc);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + panelPadX);
        ImGui::Spacing();
    };

    DrawAxisBar("LX", "left stick  ·  horizontal", std::clamp(snapshot.state.leftX  / 32768.0f, -1.0f, 1.0f));
    DrawAxisBar("LY", "left stick  ·  vertical",   std::clamp(snapshot.state.leftY  / 32768.0f, -1.0f, 1.0f));
    DrawAxisBar("RX", "right stick  ·  horizontal", std::clamp(snapshot.state.rightX / 32768.0f, -1.0f, 1.0f));
    DrawAxisBar("RY", "right stick  ·  vertical",   std::clamp(snapshot.state.rightY / 32768.0f, -1.0f, 1.0f));

    // ── Triggers ─────────────────────────────────────────────────────────────
    ImGui::Spacing();
    ImGui::Separator();

    SectionHeader("", "TRIGGERS", "analog  ·  range  0 → 255");

    auto DrawTriggerBar = [&](const char* label, const char* desc, uint8_t value) {
        constexpr float barWidth  = 160.0f;
        constexpr float barHeight = 10.0f;

        const float  fill = value / 255.0f;
        ImDrawList*  dl   = ImGui::GetWindowDrawList();

        const ImU32 trackCol = ImGui::GetColorU32(ImVec4(0.12f, 0.12f, 0.16f, 1.0f));
        const ImU32 fillCol  = ImGui::GetColorU32(ImVec4(0.45f, 0.85f, 0.65f, 0.9f));
        const ImU32 knobCol  = ImGui::GetColorU32(ImVec4(0.65f, 1.0f,  0.8f,  1.0f));

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + panelPadX);
        const ImVec2 cursorPos = ImGui::GetCursorScreenPos();

        const ImVec2 trackMin = cursorPos;
        const ImVec2 trackMax = { cursorPos.x + barWidth, cursorPos.y + barHeight };
        const float  fillX    = cursorPos.x + barWidth * fill;

        dl->AddRectFilled(trackMin, trackMax, trackCol, barHeight);
        dl->AddRectFilled(trackMin, { fillX, trackMax.y }, fillCol, barHeight);
        dl->AddCircleFilled({ fillX, cursorPos.y + barHeight * 0.5f }, barHeight * 0.65f, knobCol, 12);

        ImGui::Dummy({ barWidth + 4.0f, barHeight });
        ImGui::SameLine(0, 10);
        ImGui::Text("%-4s", label);
        ImGui::SameLine(0, 6);
        ImGui::TextColored(ImVec4(0.45f, 0.85f, 0.65f, 1.0f), "%3u", value);
        ImGui::SameLine(0, 10);
        ImGui::TextDisabled("%s", desc);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + panelPadX);
        ImGui::Spacing();
    };

    DrawTriggerBar("LT", "left trigger",  snapshot.state.leftTrigger);
    DrawTriggerBar("RT", "right trigger", snapshot.state.rightTrigger);

    ImGui::Spacing();
    ImGui::Separator();

    char buttonSubtitle[48];
    snprintf(buttonSubtitle, sizeof(buttonSubtitle), "digital  ·  bitmask  0x%04X", snapshot.state.buttons);
    SectionHeader("", "BUTTONS", buttonSubtitle);

    struct ButtonDef { const char* label; int bit; const char* tooltip; };
    constexpr ButtonDef kButtons[] = {
        {"A",    0,  "Face — confirm"},
        {"B",    1,  "Face — cancel"},
        {"X",    2,  "Face — action"},
        {"Y",    3,  "Face — action"},
        {"LB",   4,  "Left shoulder"},
        {"RB",   5,  "Right shoulder"},
        {"L3",   6,  "Left stick click"},
        {"R3",   7,  "Right stick click"},
        {"↑",    8,  "D-pad up"},
        {"↓",    9,  "D-pad down"},
        {"←",   10,  "D-pad left"},
        {"→",   11,  "D-pad right"},
        {"MENU",   12,  "Menu"},
        {"START",   14,  "Start"},
    };

    constexpr int    kCols   = 4;
    const     ImVec2 btnSize = { 68.0f, 30.0f };

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(6.0f, 6.0f));

    for (int i = 0; i < IM_ARRAYSIZE(kButtons); ++i) {
        const bool   pressed = (snapshot.state.buttons >> kButtons[i].bit) & 1;
        const ImVec4 bgCol   = pressed ? ImVec4(0.45f, 0.45f, 0.95f, 1.0f) : ImVec4(0.16f, 0.16f, 0.20f, 1.0f);
        const ImVec4 textCol = pressed ? ImVec4(1.0f,  1.0f,  1.0f,  1.0f) : ImVec4(0.45f, 0.45f, 0.55f, 1.0f);

        if (i % kCols == 0)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + panelPadX);

        ImGui::PushStyleColor(ImGuiCol_Button,        bgCol);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bgCol);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  bgCol);
        ImGui::PushStyleColor(ImGuiCol_Text,          textCol);
        ImGui::Button(kButtons[i].label, btnSize);
        ImGui::PopStyleColor(4);

        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::TextDisabled("bit %d", kButtons[i].bit);
            ImGui::SameLine(0, 6);
            ImGui::TextUnformatted(kButtons[i].tooltip);
            ImGui::EndTooltip();
        }

        if ((i + 1) % kCols != 0 && i != IM_ARRAYSIZE(kButtons) - 1)
            ImGui::SameLine();
    }

    ImGui::PopStyleVar(2);

    ImGui::Spacing();
    ImGui::Separator();

    SectionHeader("", "D-PAD", "directional  ·  bits 8 – 11");

    auto DpadBtn = [&](const char* label, int bit) {
        const bool   pressed = (snapshot.state.buttons >> bit) & 1;
        const ImVec4 col     = pressed ? ImVec4(0.45f, 0.45f, 0.95f, 1.0f) : ImVec4(0.16f, 0.16f, 0.20f, 1.0f);
        const ImVec4 text    = pressed ? ImVec4(1.0f,  1.0f,  1.0f,  1.0f) : ImVec4(0.45f, 0.45f, 0.55f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Button,        col);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, col);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  col);
        ImGui::PushStyleColor(ImGuiCol_Text,          text);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
        ImGui::Button(label, { 38.0f, 30.0f });
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);
    };

    constexpr float kBtnW    = 38.0f;
    constexpr float kBtnH    = 30.0f;
    constexpr float kBtnGap  = 4.0f;
    const     float kStride  = kBtnW + kBtnGap;
    const     float kOriginX = ImGui::GetCursorPosX() + panelPadX;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { kBtnGap, kBtnGap });

    ImGui::SetCursorPosX(kOriginX);
    ImGui::Dummy({ kBtnW, kBtnH }); ImGui::SameLine();
    DpadBtn("↑", 8);                ImGui::SameLine();
    ImGui::Dummy({ kBtnW, kBtnH });

    ImGui::SetCursorPosX(kOriginX);
    DpadBtn("←", 10); ImGui::SameLine();
    {
        // Centre pip
        const ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::Dummy({ kBtnW, kBtnH });
        ImGui::GetWindowDrawList()->AddCircleFilled(
            { p.x + kBtnW * 0.5f, p.y + kBtnH * 0.5f }, 4.0f,
            ImGui::GetColorU32(ImVec4(0.25f, 0.25f, 0.30f, 1.0f)), 12);
    }
    ImGui::SameLine();
    DpadBtn("→", 11); ImGui::SameLine(0, 16);

    {
        const bool up    = (snapshot.state.buttons >> 8)  & 1;
        const bool down  = (snapshot.state.buttons >> 9)  & 1;
        const bool left  = (snapshot.state.buttons >> 10) & 1;
        const bool right = (snapshot.state.buttons >> 11) & 1;

        const char* dirLabel = "NEUTRAL";
        if      (up   && left)  dirLabel = "UP-LEFT";
        else if (up   && right) dirLabel = "UP-RIGHT";
        else if (down && left)  dirLabel = "DOWN-LEFT";
        else if (down && right) dirLabel = "DOWN-RIGHT";
        else if (up)            dirLabel = "UP";
        else if (down)          dirLabel = "DOWN";
        else if (left)          dirLabel = "LEFT";
        else if (right)         dirLabel = "RIGHT";

        const bool   anyPressed = up || down || left || right;
        const ImVec4 labelCol   = anyPressed
            ? ImVec4(0.45f, 0.45f, 0.95f, 1.0f)
            : ImVec4(0.35f, 0.35f, 0.40f, 1.0f);

        // Vertically center the text against the 3-row cross (3 * stride high)
        const float crossHeight = kBtnH * 3 + kBtnGap * 2;
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - kBtnH - kBtnGap + (crossHeight - ImGui::GetTextLineHeight()) * 0.5f);
        ImGui::TextColored(labelCol, "%s", dirLabel);
    }

    // Row 3: [ ] [↓] [ ]
    ImGui::SetCursorPosX(kOriginX);
    ImGui::Dummy({ kBtnW, kBtnH }); ImGui::SameLine();
    DpadBtn("↓", 9);                ImGui::SameLine();
    ImGui::Dummy({ kBtnW, kBtnH });

    ImGui::PopStyleVar(); // ItemSpacing

    // ── Status bar ────────────────────────────────────────────────────────────
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    const bool fresh = IsSignalFresh(snapshot);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + panelPadX);
    ImGui::TextDisabled("UDP 8080");
    ImGui::SameLine(0, 8);
    ImGui::TextDisabled("|");
    ImGui::SameLine(0, 8);
    ImGui::TextColored(
        fresh ? ImVec4(0.0f, 0.9f, 0.4f, 1.0f) : ImVec4(0.9f, 0.3f, 0.3f, 1.0f),
        fresh ? "● LIVE" : "● STALE");
    ImGui::SameLine(0, 8);
    ImGui::TextDisabled("|");
    ImGui::SameLine(0, 8);
    ImGui::TextDisabled("%.0f FPS", io.Framerate);
    ImGui::Spacing();

    ImGui::End();
}
