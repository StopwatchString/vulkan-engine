#include "vulkan_utils.h"

#include "constants.h"

#include <iostream>
#include <set>
#include <algorithm>

//---------------------------------
// getVkInstanceExtensionProperties()
//---------------------------------
std::vector<VkExtensionProperties> getVkInstanceExtensionProperties()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    return extensions;
}

//---------------------------------
// getRequiredInstanceExtensions()
//---------------------------------
std::vector<const char*> getRequiredInstanceExtensions()
{
    // Query GLFW for required platform extensions
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = nullptr;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (VALIDATION_LAYERS_ENABLE) {
        requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return requiredExtensions;
}

//---------------------------------
// getPhysicalDevices()
//---------------------------------
std::vector<VkPhysicalDevice> getPhysicalDevices(const VkInstance& instance)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    return devices;
}

//---------------------------------
// findQueueFamilies()
//---------------------------------
QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    for (int i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) break;
    }

    return indices;
}

//---------------------------------
// querySwapchainSupport()
//---------------------------------
SwapchainSupportDetails querySwapchainSupport(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface)
{
    SwapchainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

//---------------------------------
// chooseSwapSurfaceFormat()
//---------------------------------
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const VkSurfaceFormatKHR& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

//---------------------------------
// chooseSwapPresentMode()
//---------------------------------
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const VkPresentModeKHR& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

//---------------------------------
// chooseSwapExtent()
//---------------------------------
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
    VkExtent2D extent{};
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        extent = capabilities.currentExtent;
    }
    else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        extent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };
        extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }
    return extent;
}

//---------------------------------
// checkValidationLayerSupport()
//---------------------------------
bool checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    std::vector<std::string> missingLayers;
    for (const std::string& layerName : VALIDATION_LAYERS) {
        auto it = std::find_if(std::begin(availableLayers), std::end(availableLayers), [&layerName](const VkLayerProperties& layerProperties) {
            return strcmp(layerProperties.layerName, layerName.c_str()) == 0;
            });
        if (it == std::end(availableLayers)) {
            missingLayers.push_back(layerName);
        }
    }

    // In the future we can return the list instead
    return missingLayers.size() == 0;
}

//---------------------------------
// checkRequiredInstanceExtensionsSupport()
//---------------------------------
bool checkRequiredInstanceExtensionsSupport(std::vector<const char*> requiredExtensions)
{
    std::vector<VkExtensionProperties> instanceExtensions = getVkInstanceExtensionProperties();

    std::vector<std::string> missingExtensions;
    for (const std::string& extension : requiredExtensions) {
        const char* extensionName = extension.c_str();
        auto it = std::find_if(std::begin(instanceExtensions), std::end(instanceExtensions),
            [extensionName](const VkExtensionProperties& exProp) {
                return strcmp(exProp.extensionName, extensionName) == 0;
            });
        if (it == std::end(instanceExtensions)) {
            missingExtensions.push_back(std::string(extensionName));
        }
    }

    return missingExtensions.size() == 0;
}

//---------------------------------
// isPhysicalDeviceSuitable()
//---------------------------------
bool isPhysicalDeviceSuitable(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface)
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

    bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapchainSupportDetails swapChainSupportDetails = querySwapchainSupport(physicalDevice, surface);
        swapChainAdequate = !swapChainSupportDetails.formats.empty() && !swapChainSupportDetails.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

//---------------------------------
// checkDeviceExtensionSupport()
//---------------------------------
bool checkDeviceExtensionSupport(const VkPhysicalDevice& physicalDevice)
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(REQUIRED_DEVICE_EXTENSIONS.begin(), REQUIRED_DEVICE_EXTENSIONS.end());
    std::erase_if(requiredExtensions, [&availableExtensions](const std::string& extensionName) {
        auto it = std::find_if(availableExtensions.begin(), availableExtensions.end(), [&extensionName](const VkExtensionProperties& extensionProperty) {
            return strcmp(extensionName.c_str(), extensionProperty.extensionName) == 0;
            });
        return it != availableExtensions.end();
    });

    return requiredExtensions.empty();
}

//---------------------------------
// populateDebugMessengerCreateInfo()
//---------------------------------
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;
}

//---------------------------------
// debugCallback()
//---------------------------------
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT  messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    // messageSeverity
    //VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
    //VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
    //VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
    //VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT

    // messageType
    // VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
    // VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
    // VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

//---------------------------------
// CreateDebugUtilsMessengerEXT()
//---------------------------------
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

//---------------------------------
// DestroyDebugUtilsMessengerEXT()
//---------------------------------
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}