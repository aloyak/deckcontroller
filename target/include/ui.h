#pragma once

#include <imgui.h>
#include "state.h"

struct SDL_Texture;

struct UiFonts {
	ImFont* regular = nullptr;
	ImFont* bold = nullptr;
};

void RenderControllerWindow(const ControllerSnapshot& snapshot, ImGuiIO& io,
									const UiFonts& fonts, SDL_Texture* controllerTexture,
									ImVec2 controllerTextureSize);