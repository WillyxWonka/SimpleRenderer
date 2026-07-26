#include <iostream>
#include <GLFW/glfw3.h>

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW.\n";
        return 1;
    }

    GLFWwindow* window = glfwCreateWindow(
        800,
        600,
        "Simple Renderer",
        nullptr,
        nullptr
    );

    if (window == nullptr)
    {
        std::cerr << "Failed to create window.\n";

        glfwTerminate();

        return 1;
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    std::cout << "Window Closed | Loop end\n";
    return 0;
}