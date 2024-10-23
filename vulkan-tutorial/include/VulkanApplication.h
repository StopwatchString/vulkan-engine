#ifndef APPLICATION_H
#define APPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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
    void pickPhysicalDevice();
    void createLogicalDevice();

    void mainLoop();
    
    void cleanup();

    GLFWwindow* m_GLFWwindow                  { nullptr };
    VkInstance m_VkInstance                   { nullptr };
    VkDebugUtilsMessengerEXT m_DebugMessenger { nullptr };
    VkPhysicalDevice m_PhysicalDevice         { nullptr };
    VkDevice m_Device                         { nullptr };
    VkQueue m_GraphicsQueue                   { nullptr };
};

#endif