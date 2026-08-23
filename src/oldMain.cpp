#include <iostream>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cstddef>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

//duck hunt clone
//dungeon crawler 2d-ish game
//something with wave function collapse
//particle life sim / conways game of life
//3d model uploader 
//sdfs
//top down grid like vertex defined that samples noise maps/height map to create "landmass/island" and height map for colors and also for shadows eventually
//ZEBRA ENGINE

struct Vertex
{
    //position
    float x;
    float y;
    float z;

    //color
    float r;
    float g;
    float b;
};
struct Matrix2
{
    float a;
    float b;
    float c;
    float d;
};
constexpr int WINDOW_WIDTH = 900;
constexpr int WINDOW_HEIGHT = 900;

//
Vertex TransformVertex(Vertex vertex, const Matrix2& matrix)
{
    const float oldX = vertex.x;
    const float oldY = vertex.y;

    vertex.x = matrix.a * oldX + matrix.b * oldY;
    vertex.y = matrix.c * oldX + matrix.d * oldY;

    return vertex;
}
//
Matrix2 MultiplyMatrices(const Matrix2& left, const Matrix2& right)
{
    return
    {
        left.a * right.a + left.b * right.c,
        left.a * right.b + left.b * right.d,

        left.c * right.a + left.d * right.c,
        left.c * right.b + left.d * right.d
    };
}
//
Vertex ScaleVertex(Vertex vertex, float scaleX, float scaleY)
{
    vertex.x *= scaleX;
    vertex.y *= scaleY;

    return vertex;
}
//
Vertex TranslateVertex(Vertex vertex, float offsetX, float offsetY)
{
    vertex.x += offsetX;
    vertex.y += offsetY;
    return vertex;
}
//
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // 1. Update the OpenGL rendering viewport size (pixels)
    glViewport(0, 0, width, height);

    // 2. Recalculate your projection matrix aspect ratio
    float aspectRatio = (float)width / (float)height;
    
    // Example for 3D Perspective Projection (using GLM)
    // projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
    
    // Example for 2D Orthographic Projection (using GLM)
    // projectionMatrix = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
}
//
void MatrixExperiments(const Vertex& original)
{
    Vertex transformed = ScaleVertex(original, 2.0f, 2.0f);
    transformed = TranslateVertex(transformed, 5.0f, 0.0f);

    std::cout
        << "Original: (" << original.x << ", " << original.y << ")\n"
        << "Transformed: (" << transformed.x << ", "
        << transformed.y << ")\n";

    const Matrix2 rotate90 =
    {
         0.0f, -1.0f,
         1.0f,  0.0f
    };

    Vertex rotated = TransformVertex(original, rotate90);

    std::cout
        << "Rotated 90 degrees: ("
        << rotated.x << ", "
        << rotated.y << ")\n";

    const Matrix2 scaleMatrix =
    {
        2.0f, 0.0f,
        0.0f, 3.0f
    };

    Vertex matrixScaled = TransformVertex(original, scaleMatrix);

    std::cout
        << "Matrix scaled: ("
        << matrixScaled.x << ", "
        << matrixScaled.y << ")\n";

    Vertex sequential = TransformVertex(original, scaleMatrix);
    sequential = TransformVertex(sequential, rotate90);

    Matrix2 combined = MultiplyMatrices(rotate90, scaleMatrix);
    Vertex combinedResult = TransformVertex(original, combined);

    std::cout
        << "Sequential: (" << sequential.x
        << ", " << sequential.y << ")\n"
        << "Combined:   (" << combinedResult.x
        << ", " << combinedResult.y << ")\n";
}

// -------------------------------------------------
// MAIN
// -------------------------------------------------
int main()
{
    // -------------------------------------------------
    // Initialize GLFW
    // -------------------------------------------------
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW.\n";
        return 1;
    }
    // Request an OpenGL 3.3 Core Profile context.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    // -------------------------------------------------
    // Create the window
    // -------------------------------------------------
    GLFWwindow* window = glfwCreateWindow(  WINDOW_WIDTH, WINDOW_HEIGHT, "ZEBRA Engine", nullptr, nullptr );

    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window.\n";
        glfwTerminate();
        return 1;
    }

    // Tell OpenGL that subsequent commands belong to this window.
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // -------------------------------------------------
    // Initialize GLAD
    // -------------------------------------------------
    const int version = gladLoadGL(glfwGetProcAddress);

    if (version == 0)
    {
        std::cerr << "Failed to initialize GLAD.\n";

        glfwDestroyWindow(window);
        glfwTerminate();

        return 1;
    }
    std::cout
        << "Loaded OpenGL "
        << GLAD_VERSION_MAJOR(version)
        << "."
        << GLAD_VERSION_MINOR(version)
        << '\n';

    // Define the area of the window OpenGL renders into.
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    // -------------------------------------------------
    // Triangle data in CPU memory
    // -------------------------------------------------
    const Vertex vertices[] =
    {
        // // Leaves
        // {-0.4f, -0.1f, 0.0f,  0.35f, 0.5f, 0.1f},
        // { 0.4f, -0.1f, 0.0f,  0.0f, 0.35f, 0.1f},
        // { 0.0f,  0.45f, 0.0f,  0.35f, 0.5f, 0.1f},
        // // Leaves
        // {-0.4f, 0.2f, 0.0f,  0.2f, 0.5f, 0.1f},
        // { 0.4f, 0.2f, 0.0f,  0.00f, 0.35f, 0.1f},
        // { 0.0f,  0.75f, 0.0f,  0.2f, 0.5f, 0.1f},
            // Trunk: first triangle
        
        // WALLS: stone
        {-0.15f, -0.8f, 0.0f,  0.55f, 0.53f, 0.48f},
        { 0.15f, -0.8f, 0.0f,  0.48f, 0.47f, 0.43f},
        { 0.15f, -0.1f, 0.0f,  0.62f, 0.60f, 0.54f},

        {-0.15f, -0.8f, 0.0f,  0.55f, 0.53f, 0.48f},
        { 0.15f, -0.1f, 0.0f,  0.62f, 0.60f, 0.54f},
        {-0.15f, -0.1f, 0.0f,  0.45f, 0.44f, 0.40f},

        // ROOF: reddish-brown shingles
        {-0.12f, -0.75f, 0.0f,  0.32f, 0.16f, 0.10f},
        { 0.12f, -0.75f, 0.0f,  0.40f, 0.20f, 0.12f},
        { 0.12f, -0.15f, 0.0f,  0.27f, 0.13f, 0.09f},

        {-0.12f, -0.75f, 0.0f,  0.32f, 0.16f, 0.10f},
        { 0.12f, -0.15f, 0.0f,  0.27f, 0.13f, 0.09f},
        {-0.12f, -0.15f, 0.0f,  0.36f, 0.18f, 0.11f},
        
        // CHIMNEY / OPENING: black rectangle made of 2 triangles
        {-0.08f, -0.55f, 0.0f,  0.02f, 0.02f, 0.02f},
        {-0.02f, -0.55f, 0.0f,  0.02f, 0.02f, 0.02f},
        {-0.02f, -0.38f, 0.0f,  0.06f, 0.06f, 0.06f},

        {-0.08f, -0.55f, 0.0f,  0.02f, 0.02f, 0.02f},
        {-0.02f, -0.38f, 0.0f,  0.06f, 0.06f, 0.06f},
        {-0.08f, -0.38f, 0.0f,  0.03f, 0.03f, 0.03f},
    };
    
    MatrixExperiments(vertices[2]);


    // -------------------------------------------------
    // Create a Vertex Buffer Object
    // -------------------------------------------------

    //VAO stuff
    unsigned int vertexArrayObject = 0;
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    // Create an OpenGL buffer object and upload vertex data into storage managed by the graphics driver. The VBO stores our vertex data in GPU-managed memory.
    unsigned int vertexBufferObject = 0;                                            // ID (handle) used to refer to that buffer.
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);                              // Make this the currently selected array buffer.
    glBufferData( GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW );    // Copy the triangle data into the selected buffer.

    // Describe how each vertex is laid out in the VBO.
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, x)) );// Attribute 0:  3 floats (x, y, z) for pos
    glEnableVertexAttribArray(0);                                                                                   // Enable attribute 0 so the vertex shader can read it.
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, r)) );// Attribute 1:  3 floats (r,g,b) for color
    glEnableVertexAttribArray(1);                                                                                   // Enable attribute 1 so the vertex shader can read it.
    
    // GPU program that processes each vertex position.
    const char* vertexShaderSource = R"(
        #version 330 core

        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 color;

        out vec3 vertexColor;
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        void main()
        {
            gl_Position =   projection *
                            view *
                            model *
                            vec4(position, 1.0);
            vertexColor = color;
        }
    )";

    // Create a vertex shader object and store its OpenGL handle.
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);

    // Give OpenGL the GLSL source code, then compile it for the GPU.
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    //logic to check if vertex shader compiled
    int vertexShaderCompiled = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexShaderCompiled );

    //shader did not compile error 
    if (!vertexShaderCompiled)
    {
        char errorMessage[512];
        glGetShaderInfoLog( vertexShader, sizeof(errorMessage), nullptr, errorMessage );
        std::cerr
            << "Vertex shader compilation failed:\n"
            << errorMessage
            << '\n';
    }
    else if(vertexShaderCompiled){
        std::cout << "Vertex shader compiled successfully.\n";
    }

    // GPU program that chooses the color of each generated fragment.
    const char* fragmentShaderSource = R"(
        #version 330 core

        in vec3 vertexColor;
        out vec4 fragmentColor;

        void main()
        {
            fragmentColor = vec4(vertexColor, 1.0);
        }
    )";
    // Create a fragment shader object and store its OpenGL handle.
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Give OpenGL the GLSL source code, then compile it for the GPU.
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    int fragmentShaderCompiled = 0;
    glGetShaderiv( fragmentShader, GL_COMPILE_STATUS, &fragmentShaderCompiled );
    if (!fragmentShaderCompiled)
    {
        char errorMessage[512];

        glGetShaderInfoLog( fragmentShader,sizeof(errorMessage), nullptr, errorMessage );
        std::cerr
            << "Fragment shader compilation failed:\n"
            << errorMessage
            << '\n';
    }
    else{
        std::cout << "Fragment shader compiled successfully.\n";
    }

    // Create a complete GPU program from the compiled shader stages.
    unsigned int shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);

    int shaderProgramLinked = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &shaderProgramLinked );

    if (!shaderProgramLinked)
    {
        char errorMessage[512];
        glGetProgramInfoLog( shaderProgram, sizeof(errorMessage), nullptr, errorMessage );
        std::cerr
            << "Shader program linking failed:\n"
            << errorMessage
            << '\n';
    }
    else
    {
        std::cout << "Shader program linked successfully.\n";
    }

    // The linked program now owns the compiled GPU instructions,
    // so the separate shader objects can be deleted.
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // -------------------------------------------------
    // VERTEX TRANSFORM STUFF
    // -------------------------------------------------
    const float aspectRatio =
    static_cast<float>(WINDOW_WIDTH) /
    static_cast<float>(WINDOW_HEIGHT);

    glm::mat4 projection = glm::perspective(
        glm::radians(60.0f),
        aspectRatio,
        0.1f,
        100.0f
    );

    glm::vec3 cameraPosition{0.0f, 0.0f, 0.0f};
    float cameraRotation = 0.0f;

    const int modelLocation =
        glGetUniformLocation(shaderProgram, "model");

    const int viewLocation =
        glGetUniformLocation(shaderProgram, "view");

    const int projectionLocation =
        glGetUniformLocation(shaderProgram, "projection");
    // glm::mat4 view{1.0f};
    // view = glm::translate(
    //     view,
    //     -cameraPosition
    // );

    glm::mat4 model{1.0f};
    model = glm::translate(
        model,
        glm::vec3(0.0f, 0.0f, -5.0f)
    );
    model = glm::rotate(
        model,
        glm::radians(360.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
    model = glm::scale(
        model,
        glm::vec3(1.0f, 1.0f, 1.0f)
    );
    std::cout
        << "Model uniform location: "
        << modelLocation
        << '\n';

    // -------------------------------------------------
    // SHADER PROGRAM
    // -------------------------------------------------
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(
        modelLocation,
        1,
        GL_FALSE,
        glm::value_ptr(model)
    );
    glUniformMatrix4fv(
        projectionLocation,
        1,
        GL_FALSE,
        glm::value_ptr(projection)
    );
    // glUniformMatrix4fv(
    //     viewLocation,
    //     1,
    //     GL_FALSE,
    //     glm::value_ptr(view)
    // );
    int framebufferWidth = WINDOW_WIDTH;
    int framebufferHeight = WINDOW_HEIGHT;

    glfwSwapInterval(0);    // Enable vertical synchronization.
    double previousTime = glfwGetTime();
    // -------------------------------------------------
    // Application LOOOOOOOOOOOOP
    // -------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        const double currentTime = glfwGetTime();
        const double deltaTime = currentTime - previousTime;
        previousTime = currentTime;
        
        const float cameraSpeed = 1.0f;
        const float cameraSpeedBoost = 2.0f;
        float cameraMovement = cameraSpeed * static_cast<float>(deltaTime); // per frame movement reletive to framerate
        const float cameraRotationSpeed = 90.0f;
        
        glfwGetFramebufferSize(
            window,
            &framebufferWidth,
            &framebufferHeight
        );
        const float aspectRatio =
            static_cast<float>(framebufferWidth) /
            static_cast<float>(framebufferHeight);

        glm::mat4 projection = glm::perspective(
            glm::radians(60.0f),
            aspectRatio,
            0.1f,
            100.0f
        );
        //CAMERA ROTATE
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        {
            cameraRotation += cameraRotationSpeed * static_cast<float>(deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        {
            cameraRotation -= cameraRotationSpeed * static_cast<float>(deltaTime);
        }
        
        const float cameraAngle = glm::radians(cameraRotation);

        const glm::vec3 cameraForward
        {
            -glm::sin(cameraAngle),
            glm::cos(cameraAngle),
            0.0f
        };
        const glm::vec3 cameraRight
        {
            glm::cos(cameraAngle),
            glm::sin(cameraAngle),
            0.0f
        };
        //CAMERA MOVEMENT
        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
            cameraMovement *= cameraSpeedBoost;
        }
        if (glfwGetKey(window, GLFW_KEY_W) || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            cameraPosition += cameraForward * cameraMovement;
        }
        if (glfwGetKey(window, GLFW_KEY_S)  || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            cameraPosition -= cameraForward * cameraMovement;
        }
        if (glfwGetKey(window, GLFW_KEY_D) || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            cameraPosition += cameraRight * cameraMovement;
        }
        if (glfwGetKey(window, GLFW_KEY_A) || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        {
            cameraPosition -= cameraRight * cameraMovement;
        }

        glm::mat4 view{1.0f};
        view = glm::rotate(
            view,
            glm::radians(-cameraRotation),
            glm::vec3(0.0f, 0.0f, 1.0f)
        );

        view = glm::translate(
            view,
            -cameraPosition
        );

        glUniformMatrix4fv(
            viewLocation,
            1,
            GL_FALSE,
            glm::value_ptr(view)
        );
        glUniformMatrix4fv(
            projectionLocation,
            1,
            GL_FALSE,
            glm::value_ptr(projection)
        );
        glClearColor( 0.1f, 0.15f, 0.2f, 1.0f );
      
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(vertexArrayObject);
        glDrawArrays(GL_TRIANGLES, 0, std::size(vertices));

        glfwSwapBuffers(window);
        glfwPollEvents();  
    }

    // -------------------------------------------------
    // Cleanup
    // -------------------------------------------------
    glDeleteVertexArrays(1, &vertexArrayObject);
    glDeleteBuffers(1, &vertexBufferObject);
    glDeleteProgram(shaderProgram);
    
    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "Window closed | Loop ended\n";
    return 0;
}

    //square logic 
    // float vertices[] = {
    //  0.5f,  0.5f, 0.0f,  // top right
    //  0.5f, -0.5f, 0.0f,  // bottom right
    // -0.5f, -0.5f, 0.0f,  // bottom left
    // -0.5f,  0.5f, 0.0f   // top left 
    // };
    // unsigned int indices[] = {  // note that we start from 0!
    //     0, 1, 3,   // first triangle
    //     1, 2, 3    // second triangle
    // };  

    // -------------------------------------------------
    // VERTEX TRANSFORM STUFF (OLD LOGIC)
    // -------------------------------------------------
    // int offsetLocation = glGetUniformLocation(shaderProgram, "offset");
    // std::cout << "Offset uniform location: "
    //         << offsetLocation
    //         << '\n';
    // glUseProgram(shaderProgram);
    // glUniform2f(offsetLocation, 0.2f, 0.0f);

    // if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    // {
    //     cameraPosition.x -= cameraMovement;
    // }
    // if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    // {
    //     cameraPosition.x += cameraMovement;
    // }
    // if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    // {
    //     cameraPosition.y -= cameraMovement;
    // }
    // if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    // {
    //     cameraPosition.y += cameraMovement;
    // }








    // //////////////////////////////// FUTURE
    // struct Transform
    // {
    //     glm::vec3 position;
    //     glm::quat rotation;
    //     glm::vec3 scale;
    // };
    // struct LocalToWorld
    // {
    //     glm::mat4 value;
    // };
//     Entity

// ├── Transform
// │      Position
// │      Rotation
// │      Scale
// │
// ├── LocalToWorld
// │      glm::mat4
// │
// ├── Mesh
// │
// └── Material
// ✔ Window 
// ✔ GPU Pipeline 
// ✔ Meshes 
// ✔ Colors 
// ✔ Model Matrix 
// ✔ View Matrix 
// ✔ Time-independent movement 
// ✔ Camera rotation 
// ⬜ Camera direction vectors ← Next 
// ⬜ Perspective projection 
// ⬜ Clip Space
// ⬜ NDC 
// ⬜ Depth buffer 
// ⬜ Multiple meshes 
// ⬜ Indexed geometry (EBOs) 
// ⬜ Textures 
// ⬜ Scene rendering 
// ⬜ Renderer abstraction 
// ⬜ Camera abstraction 
// ⬜ Material abstraction