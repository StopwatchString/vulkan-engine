#include "VulkanApplication.h"

#include "constants.h"
#include "vulkan_utils.h"

#include <stdexcept>
#include <vector>
#include <iostream>
#include <set>

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
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
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

    if (CreateDebugUtilsMessengerEXT(m_VkInstance, &createInfo, nullptr, &m_VkDebugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("ERROR VulkanApplication::setupDebugMessenger() CreateDebugUtilsMessengerEXT() failed");
    }
}

//---------------------------------
// createSurface()
//---------------------------------
void VulkanApplication::createSurface()
{
    if (glfwCreateWindowSurface(m_VkInstance, m_GLFWwindow, nullptr, &m_VkSurface) != VK_SUCCESS) {
        throw std::runtime_error("ERROR VulkanApplication::createSurface() Failed to create window surface!");
    }
}

//---------------------------------
// pickPhysicalDevice()
//---------------------------------
void VulkanApplication::pickPhysicalDevice()
{
    std::vector<VkPhysicalDevice> physicalDevices = getPhysicalDevices(m_VkInstance);
    if (physicalDevices.size() == 0) {
        throw std::runtime_error("ERROR VulkanApplication::pickPhysicalDevice() No physical devices present!");
    }

    for (const VkPhysicalDevice& physicalDevice : physicalDevices) {
        if (isPhysicalDeviceSuitable(physicalDevice, m_VkSurface)) {
            m_VkPhysicalDevice = physicalDevice;
            break;
        }
    }

    if (m_VkPhysicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("ERROR VulkanApplication::pickPhysicalDevice() No suitable physical device is present!");
    }
}

//---------------------------------
// createLogicalDevice()
//---------------------------------
void VulkanApplication::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(m_VkPhysicalDevice, m_VkSurface);

    float queuePriority = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphicsFamily.value(),
        indices.presentFamily.value()
    };
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.pNext = nullptr;
        queueCreateInfo.flags = 0;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.push_back(queueCreateInfo);
    }

    // All VK_FALSE for now
    VkPhysicalDeviceFeatures deviceFeatures;
    deviceFeatures.robustBufferAccess = VK_FALSE;
    deviceFeatures.fullDrawIndexUint32 = VK_FALSE;
    deviceFeatures.imageCubeArray = VK_FALSE;
    deviceFeatures.independentBlend = VK_FALSE;
    deviceFeatures.geometryShader = VK_FALSE;
    deviceFeatures.tessellationShader = VK_FALSE;
    deviceFeatures.sampleRateShading = VK_FALSE;
    deviceFeatures.dualSrcBlend = VK_FALSE;
    deviceFeatures.logicOp = VK_FALSE;
    deviceFeatures.multiDrawIndirect = VK_FALSE;
    deviceFeatures.drawIndirectFirstInstance = VK_FALSE;
    deviceFeatures.depthClamp = VK_FALSE;
    deviceFeatures.depthBiasClamp = VK_FALSE;
    deviceFeatures.fillModeNonSolid = VK_FALSE;
    deviceFeatures.depthBounds = VK_FALSE;
    deviceFeatures.wideLines = VK_FALSE;
    deviceFeatures.largePoints = VK_FALSE;
    deviceFeatures.alphaToOne = VK_FALSE;
    deviceFeatures.multiViewport = VK_FALSE;
    deviceFeatures.samplerAnisotropy = VK_FALSE;
    deviceFeatures.textureCompressionETC2 = VK_FALSE;
    deviceFeatures.textureCompressionASTC_LDR = VK_FALSE;
    deviceFeatures.textureCompressionBC = VK_FALSE;
    deviceFeatures.occlusionQueryPrecise = VK_FALSE;
    deviceFeatures.pipelineStatisticsQuery = VK_FALSE;
    deviceFeatures.vertexPipelineStoresAndAtomics = VK_FALSE;
    deviceFeatures.fragmentStoresAndAtomics = VK_FALSE;
    deviceFeatures.shaderTessellationAndGeometryPointSize = VK_FALSE;
    deviceFeatures.shaderImageGatherExtended = VK_FALSE;
    deviceFeatures.shaderStorageImageExtendedFormats = VK_FALSE;
    deviceFeatures.shaderStorageImageMultisample = VK_FALSE;
    deviceFeatures.shaderStorageImageReadWithoutFormat = VK_FALSE;
    deviceFeatures.shaderStorageImageWriteWithoutFormat = VK_FALSE;
    deviceFeatures.shaderUniformBufferArrayDynamicIndexing = VK_FALSE;
    deviceFeatures.shaderSampledImageArrayDynamicIndexing = VK_FALSE;
    deviceFeatures.shaderStorageBufferArrayDynamicIndexing = VK_FALSE;
    deviceFeatures.shaderStorageImageArrayDynamicIndexing = VK_FALSE;
    deviceFeatures.shaderClipDistance = VK_FALSE;
    deviceFeatures.shaderCullDistance = VK_FALSE;
    deviceFeatures.shaderFloat64 = VK_FALSE;
    deviceFeatures.shaderInt64 = VK_FALSE;
    deviceFeatures.shaderInt16 = VK_FALSE;
    deviceFeatures.shaderResourceResidency = VK_FALSE;
    deviceFeatures.shaderResourceMinLod = VK_FALSE;
    deviceFeatures.sparseBinding = VK_FALSE;
    deviceFeatures.sparseResidencyBuffer = VK_FALSE;
    deviceFeatures.sparseResidencyImage2D = VK_FALSE;
    deviceFeatures.sparseResidencyImage3D = VK_FALSE;
    deviceFeatures.sparseResidency2Samples = VK_FALSE;
    deviceFeatures.sparseResidency4Samples = VK_FALSE;
    deviceFeatures.sparseResidency8Samples = VK_FALSE;
    deviceFeatures.sparseResidency16Samples = VK_FALSE;
    deviceFeatures.sparseResidencyAliased = VK_FALSE;
    deviceFeatures.variableMultisampleRate = VK_FALSE;
    deviceFeatures.inheritedQueries = VK_FALSE;

    VkDeviceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.enabledLayerCount = 0; // enabledLayerCount is deprecated and should not be used
    createInfo.ppEnabledLayerNames = nullptr; // ppEnabledLayerNames is deprecated and should not be used
    createInfo.enabledExtensionCount = 0;
    createInfo.ppEnabledExtensionNames = nullptr;
    createInfo.pEnabledFeatures = &deviceFeatures;

    if (vkCreateDevice(m_VkPhysicalDevice, &createInfo, nullptr, &m_VkDevice) != VK_SUCCESS) {
        throw std::runtime_error("ERROR VulkanApplication::createLogicalDevice() Failed to create logical device!");
    }

    vkGetDeviceQueue(m_VkDevice, indices.graphicsFamily.value(), 0, &m_VkGraphicsQueue);
    vkGetDeviceQueue(m_VkDevice, indices.presentFamily.value(), 0, &m_VkPresentQueue);
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
    vkDestroyDevice(m_VkDevice, nullptr);

    DestroyDebugUtilsMessengerEXT(m_VkInstance, m_VkDebugMessenger, nullptr);

    vkDestroySurfaceKHR(m_VkInstance, m_VkSurface, nullptr);
    vkDestroyInstance(m_VkInstance, nullptr);

    glfwDestroyWindow(m_GLFWwindow);
    glfwTerminate();
}