#ifndef VULKAN_UTILS_H
#define VULKAN_UTILS_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

std::vector<VkExtensionProperties> getVkInstanceExtensionProperties();
std::vector<const char*> getRequiredExtensions();
std::vector<VkPhysicalDevice> getPhysicalDevices(const VkInstance& instance);

bool checkValidationLayerSupport();
bool checkRequiredInstanceExtensionsSupport(std::vector<const char*> requiredExtensions);
bool isPhysicalDeviceSuitable(const VkPhysicalDevice& physicalDevice);

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