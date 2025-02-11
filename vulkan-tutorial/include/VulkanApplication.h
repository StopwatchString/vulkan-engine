#ifndef APPLICATION_H
#define APPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace VulkanApp {

struct VulkanState
{
    GLFWwindow* GLFWwindow{nullptr};
    VkInstance VkInstance{nullptr};
    VkDebugUtilsMessengerEXT VkDebugMessenger{nullptr};
    VkSurfaceKHR VkSurface{nullptr};
    VkPhysicalDevice VkPhysicalDevice{nullptr};
    VkDevice VkDevice{nullptr};
    VkQueue VkGraphicsQueue{nullptr};
    VkQueue VkPresentQueue{nullptr};
    VkSwapchainKHR VkSwapchain{nullptr};
    std::vector<VkImage> SwapchainImages;
    VkFormat Format{};
    VkExtent2D Extent{};
    std::vector<VkImageView> SwapchainImageViews;
    VkRenderPass RenderPass{nullptr};
    VkPipelineLayout PipelineLayout{nullptr};
    VkPipeline GraphicsPipeline{nullptr};
    std::vector<VkFramebuffer> SwapchainFramebuffers;
    VkCommandPool CommandPool{nullptr};
    VkCommandBuffer CommandBuffer{nullptr};
    VkSemaphore ImageAvailableSemaphore{nullptr};
    VkSemaphore RenderFinishedSemaphore{nullptr};
    VkFence InFlightFence{nullptr};
};

void run();

void initWindow(VulkanState& state);

void initVulkan(VulkanState& state);
void createInstance(VulkanState& state);
void setupDebugMessenger(VulkanState& state);
void createSurface(VulkanState& state);
void pickPhysicalDevice(VulkanState& state);
void createLogicalDevice(VulkanState& state);
void createSwapchain(VulkanState& state);
void createImageViews(VulkanState& state);
void createRenderPass(VulkanState& state);
void createGraphicsPipeline(VulkanState& state);
void createFramebuffers(VulkanState& state);
void createCommandPool(VulkanState& state);
void createCommandBuffer(VulkanState& state);
void createSyncObjects(VulkanState& state);

void recordCommandBuffer(VulkanState& state, VkCommandBuffer commandBuffer, uint32_t imageIndex);

void mainLoop(VulkanState& state);
void drawFrame(VulkanState& state);

void cleanup(VulkanState& state);

} // namespace VulkanApp

#endif