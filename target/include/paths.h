#pragma once

#include <SDL3/SDL.h>
#include <filesystem>

inline std::filesystem::path executable_directory() {
    static std::filesystem::path cached = []() {
        const char* base_path = SDL_GetBasePath();
        std::filesystem::path result = base_path != nullptr
            ? std::filesystem::path(base_path)
            : std::filesystem::current_path();
        if (base_path != nullptr)
            SDL_free((void*)base_path);
        return result;
    }();
    return cached;
}

inline std::filesystem::path resource_path(const std::filesystem::path& relative_path) {
    return executable_directory() / "resources" / relative_path;
}