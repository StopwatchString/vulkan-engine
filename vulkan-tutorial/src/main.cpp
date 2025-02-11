#include "VulkanApplication.h"

#include <iostream>

int main()
{
    try {
        VulkanApp::run();
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}