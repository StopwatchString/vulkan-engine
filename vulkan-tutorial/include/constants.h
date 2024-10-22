#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>
#include <string>
#include <vector>

const static uint32_t DEFAULT_WINDOW_WIDTH = 800;
const static uint32_t DEFAULT_WINDOW_HEIGHT = 600;
const static std::string DEFAULT_WINDOW_NAME = "Vulkan Window";
const static std::string APPLICATION_NAME = "Vulkan Application";
const static std::string ENGINE_NAME = "No Engine";

static std::vector<const char*> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const static bool VALIDATION_LAYERS_ENABLE = false;
#else
const static bool VALIDATION_LAYERS_ENABLE = true;
#endif

#endif