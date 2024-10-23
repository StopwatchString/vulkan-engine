#ifndef VULKAN_UTILS_H
#define VULKAN_UTILS_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <optional>

std::vector<VkExtensionProperties> getVkInstanceExtensionProperties();
std::vector<const char*> getRequiredInstanceExtensions();
std::vector<VkPhysicalDevice> getPhysicalDevices(const VkInstance& instance);

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value()
            && presentFamily.has_value();
    }
};
QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface);

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};
SwapchainSupportDetails querySwapchainSupport(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface);

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

bool checkValidationLayerSupport();
bool checkRequiredInstanceExtensionsSupport(std::vector<const char*> requiredExtensions);
bool isPhysicalDeviceSuitable(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface);
bool checkDeviceExtensionSupport(const VkPhysicalDevice& physicalDevice);

VkShaderModule createShaderModule(const VkDevice& device, const std::vector<char>& code);

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);


VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
VkDebugUtilsMessageTypeFlagsEXT  messageType,
const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
void* pUserData);

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, 
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
    const VkAllocationCallbacks* pAllocator, 
    VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator);

#endif