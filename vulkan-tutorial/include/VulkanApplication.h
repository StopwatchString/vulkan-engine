#ifndef APPLICATION_H
#define APPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

//---------------------------------
// class VulkanApplication
//---------------------------------
class VulkanApplication
{
public:
    VulkanApplication() {};
    ~VulkanApplication() {};

    VulkanApplication(const VulkanApplication& other) = delete;
    VulkanApplication(VulkanApplication&& other) noexcept = delete;

    VulkanApplication& operator=(const VulkanApplication& other) = delete;
    VulkanApplication& operator=(VulkanApplication&& other) noexcept = delete;

    void run();
    
private:
    void initWindow();

    void initVulkan();
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapchain();
    void createImageViews();
    void createRenderPass();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffer();
    void createSyncObjects();

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void mainLoop();
    void drawFrame();
    
    void cleanup();

    GLFWwindow*                m_GLFWwindow              { nullptr };
    VkInstance                 m_VkInstance              { nullptr };
    VkDebugUtilsMessengerEXT   m_VkDebugMessenger        { nullptr };
    VkSurfaceKHR               m_VkSurface               { nullptr };
    VkPhysicalDevice           m_VkPhysicalDevice        { nullptr };
    VkDevice                   m_VkDevice                { nullptr };
    VkQueue                    m_VkGraphicsQueue         { nullptr };
    VkQueue                    m_VkPresentQueue          { nullptr };
    VkSwapchainKHR             m_VkSwapchain             { nullptr };
    std::vector<VkImage>       m_SwapchainImages;
    VkFormat                   m_Format                  {};
    VkExtent2D                 m_Extent                  {};
    std::vector<VkImageView>   m_SwapchainImageViews;
    VkRenderPass               m_RenderPass              { nullptr };
    VkPipelineLayout           m_PipelineLayout          { nullptr };
    VkPipeline                 m_GraphicsPipeline        { nullptr };
    std::vector<VkFramebuffer> m_SwapchainFramebuffers;
    VkCommandPool              m_CommandPool             { nullptr };
    VkCommandBuffer            m_CommandBuffer           { nullptr };
    VkSemaphore                m_ImageAvailableSemaphore { nullptr };
    VkSemaphore                m_RenderFinishedSemaphore { nullptr };
    VkFence                    m_InFlightFence           { nullptr };
};

#endif