#include "VulkanApplication.h"

#include "constants.h"
#include "vulkan_utils.h"
#include "utils.h"

#include <stdexcept>
#include <vector>
#include <iostream>
#include <set>

namespace VulkanApp {
    void run()
    {
        VulkanState state;
        initWindow(state);
        initVulkan(state);
        mainLoop(state);
        cleanup(state);
    }

    void initWindow(VulkanState& state)
    {
        if (glfwInit() == GLFW_FALSE) {
            throw std::runtime_error("ERROR VulkanApplication::initWindow() glfwInit() failed");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        state.GLFWwindow = glfwCreateWindow(
            DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, DEFAULT_WINDOW_NAME.c_str(), nullptr, nullptr);

        if (state.GLFWwindow == nullptr) {
            throw std::runtime_error("ERROR VulkanApplication::initWindow() glfwCreateWindow() failed");
        }
    }

    void initVulkan(VulkanState& state){
        createInstance(state);
        setupDebugMessenger(state);
        createSurface(state);
        pickPhysicalDevice(state);
        createLogicalDevice(state);
        createSwapchain(state);
        createImageViews(state);
        createRenderPass(state);
        createGraphicsPipeline(state);
        createFramebuffers(state);
        createCommandPool(state);
        createCommandBuffer(state);
        createSyncObjects(state);
    }

    void createInstance(VulkanState& state){
        if (VALIDATION_LAYERS_ENABLE && !checkValidationLayerSupport()) {
            throw std::runtime_error(
                "ERROR VulkanApplication::createInstance() Validation layers are enabled but are not available.");
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
            throw std::runtime_error(
                "ERROR VulkanApplication::createInstance() Not all required instance extensions are supported.");
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

        if (vkCreateInstance(&createInfo, nullptr, &state.VkInstance) != VK_SUCCESS) {
            throw std::runtime_error("ERROR VulkanApplication::createInstance() vkCreateInstance() failed");
        }
    }
    void setupDebugMessenger(VulkanState& state){
        if (!VALIDATION_LAYERS_ENABLE)
            return;
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(state.VkInstance, &createInfo, nullptr, &state.VkDebugMessenger) != VK_SUCCESS) {
            throw std::runtime_error(
                "ERROR VulkanApplication::setupDebugMessenger() CreateDebugUtilsMessengerEXT() failed");
        }
    }
    void createSurface(VulkanState& state){
        if (glfwCreateWindowSurface(state.VkInstance, state.GLFWwindow, nullptr, &state.VkSurface) != VK_SUCCESS) {
            throw std::runtime_error("ERROR VulkanApplication::createSurface() Failed to create window surface!");
        }
    }
    void pickPhysicalDevice(VulkanState& state){
        std::vector<VkPhysicalDevice> physicalDevices = getPhysicalDevices(state.VkInstance);
        if (physicalDevices.size() == 0) {
            throw std::runtime_error("ERROR VulkanApplication::pickPhysicalDevice() No physical devices present!");
        }

        for (const VkPhysicalDevice& physicalDevice : physicalDevices) {
            if (isPhysicalDeviceSuitable(physicalDevice, state.VkSurface)) {
                state.VkPhysicalDevice = physicalDevice;
                break;
            }
        }

        if (state.VkPhysicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error(
                "ERROR VulkanApplication::pickPhysicalDevice() No suitable physical device is present!");
        }
    }
    void createLogicalDevice(VulkanState& state){
        QueueFamilyIndices indices = findQueueFamilies(state.VkPhysicalDevice, state.VkSurface);

        float queuePriority = 1.0f;

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
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
        createInfo.enabledLayerCount = 0;         // enabledLayerCount is deprecated and should not be used
        createInfo.ppEnabledLayerNames = nullptr; // ppEnabledLayerNames is deprecated and should not be used
        createInfo.enabledExtensionCount = static_cast<uint32_t>(REQUIRED_DEVICE_EXTENSIONS.size());
        createInfo.ppEnabledExtensionNames = REQUIRED_DEVICE_EXTENSIONS.data();
        createInfo.pEnabledFeatures = &deviceFeatures;

        if (vkCreateDevice(state.VkPhysicalDevice, &createInfo, nullptr, &state.VkDevice) != VK_SUCCESS) {
            throw std::runtime_error("ERROR VulkanApplication::createLogicalDevice() Failed to create logical device!");
        }

        vkGetDeviceQueue(state.VkDevice, indices.graphicsFamily.value(), 0, &state.VkGraphicsQueue);
        vkGetDeviceQueue(state.VkDevice, indices.presentFamily.value(), 0, &state.VkPresentQueue);
    }
    void createSwapchain(VulkanState& state){
        SwapchainSupportDetails swapchainSupportDetails = querySwapchainSupport(state.VkPhysicalDevice, state.VkSurface);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupportDetails.formats);
        state.Format = surfaceFormat.format;
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupportDetails.presentModes);
        state.Extent = chooseSwapExtent(swapchainSupportDetails.capabilities, state.GLFWwindow);

        uint32_t imageCount = swapchainSupportDetails.capabilities.minImageCount + 1;
        if (swapchainSupportDetails.capabilities.maxImageCount > 0) {
            imageCount = std::min(imageCount, swapchainSupportDetails.capabilities.maxImageCount);
        }

        VkSwapchainCreateInfoKHR createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.surface = state.VkSurface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = state.Extent;
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

        if (vkCreateSwapchainKHR(state.VkDevice, &createInfo, nullptr, &state.VkSwapchain) != VK_SUCCESS) {
            throw std::runtime_error("ERROR VulkanApplication::createSwapchain() Failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(state.VkDevice, state.VkSwapchain, &imageCount, nullptr);
        state.SwapchainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(state.VkDevice, state.VkSwapchain, &imageCount, state.SwapchainImages.data());
    }
    void createImageViews(VulkanState& state){
        state.SwapchainImageViews.resize(state.SwapchainImages.size());

        for (size_t i = 0; i < state.SwapchainImages.size(); i++) {
            VkImageViewCreateInfo createInfo;
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.flags = 0;
            createInfo.image = state.SwapchainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = state.Format;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(state.VkDevice, &createInfo, nullptr, &state.SwapchainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error(
                    "ERROR VulkanApplication::createImageViews() Failed to create swapchain image views!");
            }
        }
    }
    void createRenderPass(VulkanState& state){
        VkAttachmentDescription colorAttachement;
        colorAttachement.flags = 0;
        colorAttachement.format = state.Format;
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

        VkSubpassDependency subpassDependency;
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;
        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.srcAccessMask = 0;
        subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDependency.dependencyFlags = 0;

        VkRenderPassCreateInfo renderPassInfo;
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.pNext = nullptr;
        renderPassInfo.flags = 0;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachement;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &subpassDependency;

        if (vkCreateRenderPass(state.VkDevice, &renderPassInfo, nullptr, &state.RenderPass) != VK_SUCCESS) {
            throw std::runtime_error("ERROR VulkanApplication::createRenderPass() Failed to create render pass!");
        }
    }
    void createGraphicsPipeline(VulkanState& state){
        std::vector<char> vertShaderCode = readFile("shaders/vert.spv");
        std::vector<char> fragShaderCode = readFile("shaders/frag.spv");

        VkShaderModule vertShaderModule = createShaderModule(state.VkDevice, vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(state.VkDevice, fragShaderCode);

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

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
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
        viewport.width = static_cast<float>(state.Extent.width);
        viewport.height = static_cast<float>(state.Extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor;
        scissor.offset = {0, 0};
        scissor.extent = state.Extent;

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
        // VkPipelineDepthStencilStateCreateInfo depthStencilInfo;

        VkPipelineColorBlendAttachmentState colorBlendAttachementState;
        colorBlendAttachementState.blendEnable = VK_FALSE;
        colorBlendAttachementState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachementState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachementState.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachementState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachementState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachementState.alphaBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachementState.colorWriteMask
            = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

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

        if (vkCreatePipelineLayout(state.VkDevice, &pipelineLayoutInfo, nullptr, &state.PipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error(
                "ERROR VulkanApplication::createGraphicsPipeline() Failed to create pipeline layout!");
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
        pipelineInfo.layout = state.PipelineLayout;
        pipelineInfo.renderPass = state.RenderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        if (vkCreateGraphicsPipelines(state.VkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &state.GraphicsPipeline)
            != VK_SUCCESS) {
            throw std::runtime_error(
                "ERROR VulkanApplication::createGraphicsPipeline() Failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(state.VkDevice, vertShaderModule, nullptr);
        vkDestroyShaderModule(state.VkDevice, fragShaderModule, nullptr);
    }
    void createFramebuffers(VulkanState& state){
        state.SwapchainFramebuffers.resize(state.SwapchainImages.size());

        for (size_t i = 0; i < state.SwapchainImageViews.size(); i++) {
            VkImageView attachments[] = {state.SwapchainImageViews[i]};

            VkFramebufferCreateInfo framebufferInfo;
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.pNext = nullptr;
            framebufferInfo.flags = 0;
            framebufferInfo.renderPass = state.RenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = state.Extent.width;
            framebufferInfo.height = state.Extent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(state.VkDevice, &framebufferInfo, nullptr, &state.SwapchainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("ERROR VulkanApplication::createFramebuffers() Failed to create framebuffer!");
            }
        }
    }
    void createCommandPool(VulkanState& state){
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(state.VkPhysicalDevice, state.VkSurface);

        VkCommandPoolCreateInfo commandPoolInfo;
        commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolInfo.pNext = nullptr;
        commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(state.VkDevice, &commandPoolInfo, nullptr, &state.CommandPool) != VK_SUCCESS) {
            throw std::runtime_error("ERROR VulkanApplication::createCommandPool() Failed to create command pool!");
        }
    }
    void createCommandBuffer(VulkanState& state){
        VkCommandBufferAllocateInfo commandBufferAllocateInfo;
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.pNext = nullptr;
        commandBufferAllocateInfo.commandPool = state.CommandPool;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(state.VkDevice, &commandBufferAllocateInfo, &state.CommandBuffer) != VK_SUCCESS) {
            throw std::runtime_error(
                "ERROR VulkanApplication::createCommandBuffer() Failed to allocate command buffers!");
        }
    }
    void createSyncObjects(VulkanState& state) {
        VkSemaphoreCreateInfo semaphoreCreateInfo;
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.pNext = nullptr;
        semaphoreCreateInfo.flags = 0;

        VkFenceCreateInfo fenceCreateInfo;
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.pNext = nullptr;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateSemaphore(state.VkDevice, &semaphoreCreateInfo, nullptr, &state.ImageAvailableSemaphore) != VK_SUCCESS) {
            throw std::runtime_error("ERROR VulkanApplication::createSyncObjects() Failed to create semaphore!");
        }

        if (vkCreateSemaphore(state.VkDevice, &semaphoreCreateInfo, nullptr, &state.RenderFinishedSemaphore) != VK_SUCCESS) {
            throw std::runtime_error("ERROR VulkanApplication::createSyncObjects() Failed to create semaphore!");
        }

        if (vkCreateFence(state.VkDevice, &fenceCreateInfo, nullptr, &state.InFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("ERROR VulkanApplication::createSyncObjects() Failed to create fence!");
        }
    }

    void recordCommandBuffer(VulkanState& state, VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo commandBufferBeginInfo;
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.pNext = nullptr;
        commandBufferBeginInfo.flags = 0;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS) {
            throw std::runtime_error(
                "ERROR VulkanApplication::recordCommandBuffer() Failed to begin recording command buffer!");
        }

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        // VkClearColorValue           color;
        // VkClearDepthStencilValue    depthStencil;

        VkRenderPassBeginInfo renderPassInfo;
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.pNext = nullptr;
        renderPassInfo.renderPass = state.RenderPass;
        renderPassInfo.framebuffer = state.SwapchainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = state.Extent;
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, state.GraphicsPipeline);

        VkViewport viewport;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(state.Extent.width);
        viewport.height = static_cast<float>(state.Extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor;
        scissor.offset = {0, 0};
        scissor.extent = state.Extent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("ERROR VulkanApplication::recordCommandBuffer() Failed to record command buffer!");
        }
    }

    void mainLoop(VulkanState& state) {
        while (!glfwWindowShouldClose(state.GLFWwindow)) {
            glfwPollEvents();
            drawFrame(state);
        }

        vkDeviceWaitIdle(state.VkDevice);
    }
    void drawFrame(VulkanState& state) {
        vkWaitForFences(state.VkDevice, 1, &state.InFlightFence, VK_TRUE, UINT64_MAX);
        vkResetFences(state.VkDevice, 1, &state.InFlightFence);

        uint32_t imageIndex;
        vkAcquireNextImageKHR(
            state.VkDevice, state.VkSwapchain, UINT64_MAX, state.ImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        vkResetCommandBuffer(state.CommandBuffer, 0);
        recordCommandBuffer(state, state.CommandBuffer, imageIndex);

        VkSemaphore waitSemaphores[] = {state.ImageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore signalSemaphores[] = {state.RenderFinishedSemaphore};

        VkSubmitInfo submitInfo;
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = nullptr;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &state.CommandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(state.VkGraphicsQueue, 1, &submitInfo, state.InFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("ERROR VulkanApplication::drawFrame() Failed to submit draw command buffer!");
        }

        VkSwapchainKHR swapChains[] = {state.VkSwapchain};

        VkPresentInfoKHR presentInfo;
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = nullptr;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        vkQueuePresentKHR(state.VkPresentQueue, &presentInfo);
    }

    void cleanup(VulkanState& state) {
        vkDestroySemaphore(state.VkDevice, state.ImageAvailableSemaphore, nullptr);
        vkDestroySemaphore(state.VkDevice, state.RenderFinishedSemaphore, nullptr);
        vkDestroyFence(state.VkDevice, state.InFlightFence, nullptr);

        vkDestroyCommandPool(state.VkDevice, state.CommandPool, nullptr);

        for (VkFramebuffer framebuffer : state.SwapchainFramebuffers) {
            vkDestroyFramebuffer(state.VkDevice, framebuffer, nullptr);
        }

        vkDestroyPipeline(state.VkDevice, state.GraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(state.VkDevice, state.PipelineLayout, nullptr);
        vkDestroyRenderPass(state.VkDevice, state.RenderPass, nullptr);

        for (VkImageView imageView : state.SwapchainImageViews) {
            vkDestroyImageView(state.VkDevice, imageView, nullptr);
        }

        vkDestroySwapchainKHR(state.VkDevice, state.VkSwapchain, nullptr);

        vkDestroyDevice(state.VkDevice, nullptr);

        DestroyDebugUtilsMessengerEXT(state.VkInstance, state.VkDebugMessenger, nullptr);

        vkDestroySurfaceKHR(state.VkInstance, state.VkSurface, nullptr);
        vkDestroyInstance(state.VkInstance, nullptr);

        glfwDestroyWindow(state.GLFWwindow);
        glfwTerminate();
    }
    }