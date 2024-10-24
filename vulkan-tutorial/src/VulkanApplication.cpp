#include "VulkanApplication.h"

#include "constants.h"
#include "vulkan_utils.h"
#include "utils.h"

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
    createSwapchain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
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

    std::vector<const char*> requiredExtensions = getRequiredInstanceExtensions();
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
    createInfo.enabledExtensionCount = static_cast<uint32_t>(REQUIRED_DEVICE_EXTENSIONS.size());
    createInfo.ppEnabledExtensionNames = REQUIRED_DEVICE_EXTENSIONS.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    if (vkCreateDevice(m_VkPhysicalDevice, &createInfo, nullptr, &m_VkDevice) != VK_SUCCESS) {
        throw std::runtime_error("ERROR VulkanApplication::createLogicalDevice() Failed to create logical device!");
    }

    vkGetDeviceQueue(m_VkDevice, indices.graphicsFamily.value(), 0, &m_VkGraphicsQueue);
    vkGetDeviceQueue(m_VkDevice, indices.presentFamily.value(), 0, &m_VkPresentQueue);
}

//---------------------------------
// createSwapchain()
//---------------------------------
void VulkanApplication::createSwapchain()
{
    SwapchainSupportDetails swapchainSupportDetails = querySwapchainSupport(m_VkPhysicalDevice, m_VkSurface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupportDetails.formats);
    m_Format = surfaceFormat.format;
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupportDetails.presentModes);
    m_Extent = chooseSwapExtent(swapchainSupportDetails.capabilities, m_GLFWwindow);

    uint32_t imageCount = swapchainSupportDetails.capabilities.minImageCount + 1;
    if (swapchainSupportDetails.capabilities.maxImageCount > 0) {
        imageCount = std::min(imageCount, swapchainSupportDetails.capabilities.maxImageCount);
    }

    VkSwapchainCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.surface = m_VkSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = m_Extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
    createInfo.preTransform = swapchainSupportDetails.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_VkDevice, &createInfo, nullptr, &m_VkSwapchain) != VK_SUCCESS) {
        throw std::runtime_error("ERROR VulkanApplication::createSwapchain() Failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(m_VkDevice, m_VkSwapchain, &imageCount, nullptr);
    m_SwapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_VkDevice, m_VkSwapchain, &imageCount, m_SwapchainImages.data());
}

//---------------------------------
// createImageViews()
//---------------------------------
void VulkanApplication::createImageViews()
{
    m_SwapchainImageViews.resize(m_SwapchainImages.size());

    for (size_t i = 0; i < m_SwapchainImages.size(); i++) {
        VkImageViewCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.image = m_SwapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_Format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_VkDevice, &createInfo, nullptr, &m_SwapchainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("ERROR VulkanApplication::createImageViews() Failed to create swapchain image views!");
        }
    }
}

//---------------------------------
// createRenderPass()
//---------------------------------
void VulkanApplication::createRenderPass()
{
    VkAttachmentDescription colorAttachement;
    colorAttachement.flags = 0;
    colorAttachement.format = m_Format;
    colorAttachement.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachement.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachement.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachement.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachement.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachement.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachement.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentReference;
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass;
    subpass.flags = 0;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;
    subpass.pResolveAttachments = nullptr;
    subpass.pDepthStencilAttachment = nullptr;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;

    VkRenderPassCreateInfo renderPassInfo;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.flags = 0;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachement;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 0;
    renderPassInfo.pDependencies = nullptr;

    if (vkCreateRenderPass(m_VkDevice, &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) {
        throw std::runtime_error("ERROR VulkanApplication::createRenderPass() Failed to create render pass!");
    }

}

//---------------------------------
// createGraphicsPipeline()
//---------------------------------
void VulkanApplication::createGraphicsPipeline()
{
    std::vector<char> vertShaderCode = readFile("shaders/vert.spv");
    std::vector<char> fragShaderCode = readFile("shaders/frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(m_VkDevice, vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(m_VkDevice, fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo;
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.pNext = nullptr;
    vertShaderStageInfo.flags = 0;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo fragShaderStageInfo;
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.pNext = nullptr;
    fragShaderStageInfo.flags = 0;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState;
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pNext = nullptr;
    dynamicState.flags = 0;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pNext = nullptr;
    vertexInputInfo.flags = 0;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.pNext = nullptr;
    inputAssemblyInfo.flags = 0;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) m_Extent.width;
    viewport.height = (float) m_Extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset = { 0, 0 };
    scissor.extent = m_Extent;

    VkPipelineViewportStateCreateInfo viewportStateInfo;
    viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateInfo.pNext = nullptr;
    viewportStateInfo.flags = 0;
    viewportStateInfo.viewportCount = 1;
    viewportStateInfo.pViewports = &viewport;
    viewportStateInfo.scissorCount = 1;
    viewportStateInfo.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizerInfo;
    rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerInfo.pNext = nullptr;
    rasterizerInfo.flags = 0;
    rasterizerInfo.depthClampEnable = VK_FALSE;
    rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizerInfo.depthBiasEnable = VK_FALSE;
    rasterizerInfo.depthBiasConstantFactor = 0.0f;
    rasterizerInfo.depthBiasClamp = 0.0f;
    rasterizerInfo.depthBiasSlopeFactor = 0.0f;
    rasterizerInfo.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisamplingInfo;
    multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingInfo.pNext = nullptr;
    multisamplingInfo.flags = 0;
    multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisamplingInfo.sampleShadingEnable = VK_FALSE;
    multisamplingInfo.minSampleShading = 1.0f;
    multisamplingInfo.pSampleMask = nullptr;
    multisamplingInfo.alphaToCoverageEnable = VK_FALSE;
    multisamplingInfo.alphaToOneEnable = VK_FALSE;

    // Create later
    //VkPipelineDepthStencilStateCreateInfo depthStencilInfo;

    VkPipelineColorBlendAttachmentState colorBlendAttachementState;
    colorBlendAttachementState.blendEnable = VK_FALSE;
    colorBlendAttachementState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachementState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachementState.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachementState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachementState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachementState.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachementState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.pNext = nullptr;
    colorBlending.flags = 0;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachementState;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pNext = nullptr;
    pipelineLayoutInfo.flags = 0;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(m_VkDevice, &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("ERROR VulkanApplication::createGraphicsPipeline() Failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    pipelineInfo.flags = 0;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pTessellationState = nullptr;
    pipelineInfo.pViewportState = &viewportStateInfo;
    pipelineInfo.pRasterizationState = &rasterizerInfo;
    pipelineInfo.pMultisampleState = &multisamplingInfo;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_PipelineLayout;
    pipelineInfo.renderPass = m_RenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(m_VkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("ERROR VulkanApplication::createGraphicsPipeline() Failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(m_VkDevice, vertShaderModule, nullptr);
    vkDestroyShaderModule(m_VkDevice, fragShaderModule, nullptr);
}

//---------------------------------
// createFramebuffers()
//---------------------------------
void VulkanApplication::createFramebuffers()
{
    m_SwapchainFramebuffers.resize(m_SwapchainImages.size());

    for (size_t i = 0; i < m_SwapchainImageViews.size(); i++) {
        VkImageView attachments[] = {
            m_SwapchainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo;
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.pNext = nullptr;
        framebufferInfo.flags = 0;
        framebufferInfo.renderPass = m_RenderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_Extent.width;
        framebufferInfo.height = m_Extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_VkDevice, &framebufferInfo, nullptr, &m_SwapchainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("ERROR VulkanApplication::createFramebuffers() Failed to create framebuffer!");
        }
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
    for (VkFramebuffer framebuffer : m_SwapchainFramebuffers) {
        vkDestroyFramebuffer(m_VkDevice, framebuffer, nullptr);
    }

    vkDestroyPipeline(m_VkDevice, m_GraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_VkDevice, m_PipelineLayout, nullptr);
    vkDestroyRenderPass(m_VkDevice, m_RenderPass, nullptr);

    for (VkImageView imageView : m_SwapchainImageViews) {
        vkDestroyImageView(m_VkDevice, imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_VkDevice, m_VkSwapchain, nullptr);

    vkDestroyDevice(m_VkDevice, nullptr);

    DestroyDebugUtilsMessengerEXT(m_VkInstance, m_VkDebugMessenger, nullptr);

    vkDestroySurfaceKHR(m_VkInstance, m_VkSurface, nullptr);
    vkDestroyInstance(m_VkInstance, nullptr);

    glfwDestroyWindow(m_GLFWwindow);
    glfwTerminate();
}