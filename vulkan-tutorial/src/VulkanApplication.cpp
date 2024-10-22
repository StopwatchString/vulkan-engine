#include "VulkanApplication.h"

#include "constants.h"
#include "vulkan_utils.h"

#include <stdexcept>
#include <vector>
#include <iostream>

//---------------------------------
// run()
//---------------------------------
void VulkanApplication::run()
{
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

//---------------------------------
// initWindow()
//---------------------------------
void VulkanApplication::initWindow()
{
    if (glfwInit() == GLFW_FALSE) {
        throw std::runtime_error("ERROR VulkanApplication::initWindow() glfwInit() failed");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_GLFWwindow = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT,
        DEFAULT_WINDOW_NAME.c_str(), nullptr, nullptr);

    if (m_GLFWwindow == nullptr) {
        throw std::runtime_error("ERROR VulkanApplication::initWindow() glfwCreateWindow() failed");
    }
}

//---------------------------------
// initVulkan()
//---------------------------------
void VulkanApplication::initVulkan()
{
    createInstance();
    setupDebugMessenger();
    pickPhysicalDevice();
}

//---------------------------------
// createInstance()
//---------------------------------
void VulkanApplication::createInstance()
{
    if (VALIDATION_LAYERS_ENABLE && !checkValidationLayerSupport()) {
        throw std::runtime_error("ERROR VulkanApplication::createInstance() Validation layers are enabled but are not available.");
    }

    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = APPLICATION_NAME.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = ENGINE_NAME.c_str();
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    std::vector<const char*> requiredExtensions = getRequiredExtensions();
    if (!checkRequiredInstanceExtensionsSupport(requiredExtensions)) {
        throw std::runtime_error("ERROR VulkanApplication::createInstance() Not all required instance extensions are supported.");
    }

    uint32_t layerCount = 0;
    const char** layerNames = nullptr;
    const void* pNext = nullptr;
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (VALIDATION_LAYERS_ENABLE) {
        layerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        layerNames = VALIDATION_LAYERS.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        pNext = &debugCreateInfo;
    }

    VkInstanceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = pNext;
    createInfo.flags = 0; // VkInstanceCreateFlags == uint32_t
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = layerCount;
    createInfo.ppEnabledLayerNames = layerNames;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    if (vkCreateInstance(&createInfo, nullptr, &m_VkInstance) != VK_SUCCESS) {
        throw std::runtime_error("ERROR VulkanApplication::createInstance() vkCreateInstance() failed");
    }
}

//---------------------------------
// setupDebugMessenger()
//---------------------------------
void VulkanApplication::setupDebugMessenger()
{
    if (!VALIDATION_LAYERS_ENABLE) return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(m_VkInstance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("ERROR VulkanApplication::setupDebugMessenger() CreateDebugUtilsMessengerEXT() failed");
    }
}

//---------------------------------
// pickPhysicalDevice()
//---------------------------------
void VulkanApplication::pickPhysicalDevice()
{
    std::vector<VkPhysicalDevice> physicalDevices = getPhysicalDevices(m_VkInstance);
    if (physicalDevices.size() == 0) {
        throw std::runtime_error("ERROR VulkanApplication::pickPhysicalDevice() No phyiscal devices present!");
    }

    for (const VkPhysicalDevice& physicalDevice : physicalDevices) {
        if (isPhysicalDeviceSuitable(physicalDevice)) {
            m_PhysicalDevice = physicalDevice;
            break;
        }
    }

    if (m_PhysicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("ERROR VulkanApplication::pickPhysicalDevice() No suitable physical device is present!");
    }
}

//---------------------------------
// mainLoop()
//---------------------------------
void VulkanApplication::mainLoop()
{
    while (!glfwWindowShouldClose(m_GLFWwindow)) {
        glfwPollEvents();
    }
}

//---------------------------------
// cleanup()
//---------------------------------
void VulkanApplication::cleanup()
{
    DestroyDebugUtilsMessengerEXT(m_VkInstance, m_DebugMessenger, nullptr);

    vkDestroyInstance(m_VkInstance, nullptr);

    glfwDestroyWindow(m_GLFWwindow);
    glfwTerminate();
}