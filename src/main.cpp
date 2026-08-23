#include <array>
#include <cstddef>
#include <iostream>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//duck hunt clone
//dungeon crawler 2d-ish game
//something with wave function collapse
//particle life sim / conways game of life
//3d model uploader 
//sdfs
//top down grid like vertex defined that samples noise maps/height map to create "landmass/island" and height map for colors and also for shadows eventually
//ZEBRA ENGINE
namespace
{
    constexpr int WINDOW_WIDTH = 900;
    constexpr int WINDOW_HEIGHT = 900;

    constexpr float CAMERA_SPEED = 3.5f;
    constexpr float CAMERA_SPEED_BOOST = 3.0f;
    constexpr float CAMERA_ROTATION_SPEED = 140.0f;

    constexpr float CAMERA_FOV = 60.0f;
    constexpr float CAMERA_NEAR_PLANE = 0.1f;
    constexpr float CAMERA_FAR_PLANE = 100.0f;

    struct Vertex
    {
        // Position
        float x;
        float y;
        float z;
        // Color
        float r;
        float g;
        float b;
        // Normals
        float nx;
        float ny;
        float nz;
    };
    
    const glm::vec3 worldUp
    {
        0.0f,
        1.0f,
        0.0f
    };  
   
    glm::vec3 lightDirection
    {
        0.0f,
        1.0f,
        0.0f
    };
    glm::vec3 lightColor
    {
        1.0f,
        .95f,
        .85f
    };

    glm::vec3 CalculateCameraForward(float yawDegrees, float pitchDegrees)
    {
        const float yaw = glm::radians(yawDegrees);
        const float pitch = glm::radians(pitchDegrees);

        glm::vec3 forward
        {
            glm::cos(yaw) * glm::cos(pitch),
            glm::sin(pitch),
            glm::sin(yaw) * glm::cos(pitch)
        };

        return glm::normalize(forward);
    }
    
    constexpr std::array<Vertex, 24> CUBE_INDEXED_VERTICES =
    {{
        // Front: 0-3  normal = +Z
        {-0.5f, -0.5f,  0.5f,   1.0f, 0.2f, 0.2f,   0.0f, 0.0f, 1.0f},
        { 0.5f, -0.5f,  0.5f,   1.0f, 0.2f, 0.2f,   0.0f, 0.0f, 1.0f},
        { 0.5f,  0.5f,  0.5f,   1.0f, 0.2f, 0.2f,   0.0f, 0.0f, 1.0f},
        {-0.5f,  0.5f,  0.5f,   1.0f, 0.2f, 0.2f,   0.0f, 0.0f, 1.0f},
        // Back: 4-7
        {-0.5f, -0.5f, -0.5f,   0.2f, 0.3f, 1.0f,  0.0f, 0.0f, -1.0f},
        { 0.5f, -0.5f, -0.5f,   0.2f, 0.3f, 1.0f,  0.0f, 0.0f, -1.0f},
        { 0.5f,  0.5f, -0.5f,   0.2f, 0.3f, 1.0f,  0.0f, 0.0f, -1.0f},
        {-0.5f,  0.5f, -0.5f,   0.2f, 0.3f, 1.0f,  0.0f, 0.0f, -1.0f},
        // Right: 8-11
        { 0.5f, -0.5f,  0.5f,   0.2f, 1.0f, 0.2f,   1.0f, 0.0f, 0.0f},
        { 0.5f, -0.5f, -0.5f,   0.2f, 1.0f, 0.2f,   1.0f, 0.0f, 0.0f},
        { 0.5f,  0.5f, -0.5f,   0.2f, 1.0f, 0.2f,   1.0f, 0.0f, 0.0f},
        { 0.5f,  0.5f,  0.5f,   0.2f, 1.0f, 0.2f,   1.0f, 0.0f, 0.0f},
        // left: 12-15
        {-0.5f, -0.5f,  0.5f,   0.8f, 1.0f, 0.2f,  -1.0f, 0.0f, 0.0f},
        {-0.5f, -0.5f, -0.5f,   0.8f, 1.0f, 0.2f,  -1.0f, 0.0f, 0.0f},
        {-0.5f,  0.5f, -0.5f,   0.8f, 1.0f, 0.2f,  -1.0f, 0.0f, 0.0f},
        {-0.5f,  0.5f,  0.5f,   0.8f, 1.0f, 0.2f,  -1.0f, 0.0f, 0.0f},
        // Top: 16-19
        {-0.5f,  0.5f,  0.5f,   1.0f, 0.5f, 0.1f,   0.0f, 1.0f, 0.0f},
        { 0.5f,  0.5f,  0.5f,   1.0f, 0.5f, 0.1f,   0.0f, 1.0f, 0.0f},
        { 0.5f,  0.5f, -0.5f,   1.0f, 0.5f, 0.1f,   0.0f, 1.0f, 0.0f},
        {-0.5f,  0.5f, -0.5f,   1.0f, 0.5f, 0.1f,   0.0f, 1.0f, 0.0f},
        // Bottom: 20-23
        {-0.5f, -0.5f,  0.5f,   1.0f, 0.3f, 0.7f,   0.0f, -1.0f, 0.0f},
        { 0.5f, -0.5f,  0.5f,   1.0f, 0.3f, 0.7f,   0.0f, -1.0f, 0.0f},
        { 0.5f, -0.5f, -0.5f,   1.0f, 0.3f, 0.7f,   0.0f, -1.0f, 0.0f},
        {-0.5f, -0.5f, -0.5f,   1.0f, 0.3f, 0.7f,   0.0f, -1.0f, 0.0f}
    }};

    constexpr std::array<unsigned int, 36> CUBE_INDICES =
    {{
        // Front
        0,  1,  2,
        0,  2,  3,
        // Back
        4,  6,  5,
        4,  7,  6,
        // Right
        8,  9, 10,
        8, 10, 11,
        // Left
        12, 13, 14,
        12, 14, 15,
        // Top  
        16, 17, 18,
        16, 18, 19,
        // Bottom
        20, 22, 21,
        20, 23, 22
    }};

  
    constexpr const char* VERTEX_SHADER_SOURCE = {R"(
        #version 330 core

        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 color;
        layout(location = 2) in vec3 normal;

        out vec3 vertexColor;
        out vec3 vertexNormal;
        out vec3 vertexWorldPosition;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        void main()
        {
            vec4 worldPosition =  model * vec4(position, 1.0);
            vertexWorldPosition = worldPosition.xyz;
            gl_Position = projection * view * worldPosition;
            vertexColor = color;
            mat3 normalMatrix = transpose(inverse(mat3(model)));
            vertexNormal = normalMatrix * normal;
        }
    )"};

    constexpr const char* FRAGMENT_SHADER_SOURCE = {R"(
        #version 330 core

        in vec3 vertexColor;
        in vec3 vertexNormal;
        in vec3 vertexWorldPosition;

        out vec4 fragmentColor;

        uniform vec3 lightDirection;
        uniform vec3 lightColor;
        uniform vec3 cameraPosition;

        uniform float specularStrength;
        uniform float shininess;
        uniform float lightIntensity;
        uniform vec3 materialColor;
        
        void main()
        {
            // N = surface normal
            // L = fragment → light
            // V = fragment → camera

            vec3 N = normalize(vertexNormal);
            vec3 L = normalize(lightDirection);
            vec3 V = normalize(cameraPosition - vertexWorldPosition);
            vec3 R = reflect(-L, N);

            float diffuse = max(dot(N, L), 0.0);
            float specular = 0.0;

            if (diffuse > 0.0)
            {
                specular =
                    pow(
                        max(dot(R, V), 0.0),
                        shininess
                    );
            }
                        
            vec3 diffuseLight = lightColor * diffuse * lightIntensity;
            vec3 ambientLight = vec3(0.075) * lightColor;
            vec3 specularLight = lightColor * lightIntensity * specularStrength * specular;

            vec3 baseColor = vertexColor * materialColor;
            vec3 litColor = baseColor * (ambientLight + diffuseLight ) + specularLight;

            fragmentColor = vec4(litColor, 1.0);
        }
    )"};

    void FramebufferSizeCallback(GLFWwindow*, int width, int height)
    {
        glViewport(0, 0, width, height);
    }

    unsigned int CompileShader(unsigned int shaderType, const char* source)
    {
        const unsigned int shader = glCreateShader(shaderType);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        int compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

        if (!compiled)
        {
            char errorMessage[1024]{};
            glGetShaderInfoLog(shader, sizeof(errorMessage), nullptr, errorMessage);

            std::cerr << "Shader compilation failed:\n"
                      << errorMessage << '\n';

            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    unsigned int CreateShaderProgram()
    {
        const unsigned int vertexShader =
            CompileShader(GL_VERTEX_SHADER, VERTEX_SHADER_SOURCE);

        if (vertexShader == 0)
            return 0;

        const unsigned int fragmentShader =
            CompileShader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_SOURCE);

        if (fragmentShader == 0)
        {
            glDeleteShader(vertexShader);
            return 0;
        }

        const unsigned int shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        int linked = 0;
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        if (!linked)
        {
            char errorMessage[1024]{};
            glGetProgramInfoLog(shaderProgram, sizeof(errorMessage), nullptr, errorMessage);

            std::cerr << "Shader program linking failed:\n"
                      << errorMessage << '\n';

            glDeleteProgram(shaderProgram);
            return 0;
        }

        return shaderProgram;
    }

    void ProcessCameraInput(GLFWwindow* window, float deltaTime, glm::vec3& cameraPosition, float& cameraYaw, float& cameraPitch)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        {
            cameraYaw -= CAMERA_ROTATION_SPEED * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            cameraYaw += CAMERA_ROTATION_SPEED * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            cameraPitch += CAMERA_ROTATION_SPEED * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            cameraPitch -= CAMERA_ROTATION_SPEED * deltaTime;
        }
        cameraPitch = glm::clamp(
            cameraPitch,
            -89.0f,
            89.0f
        );

        glm::vec3 cameraForward =
            CalculateCameraForward(cameraYaw, cameraPitch);

        glm::vec3 cameraRight =
            glm::normalize(
                glm::cross(cameraForward, worldUp)
            );

        glm::vec3 cameraUp =
            glm::normalize(
                glm::cross(cameraRight, cameraForward)
            );

        float cameraMovement = CAMERA_SPEED * deltaTime;

        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            cameraMovement *= CAMERA_SPEED_BOOST;
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            cameraPosition += cameraForward * cameraMovement;
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            cameraPosition -= cameraForward * cameraMovement;
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            cameraPosition += cameraRight * cameraMovement;
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            cameraPosition -= cameraRight * cameraMovement;
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        {
            cameraPosition += cameraUp * cameraMovement;
        }

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        {
            cameraPosition -= cameraUp * cameraMovement;
        }
    }

    glm::mat4 BuildViewMatrix( const glm::vec3& cameraPosition, const glm::vec3&    cameraForward, const glm::vec3& cameraUp)
    {
        return glm::lookAt(
            cameraPosition,
            cameraPosition + cameraForward,
            cameraUp
        );
    }
    glm::mat4 BuildProjectionMatrix(int framebufferWidth, int framebufferHeight)
    {
        const float aspectRatio =
            static_cast<float>(framebufferWidth) /
            static_cast<float>(framebufferHeight);

        return glm::perspective(
            glm::radians(CAMERA_FOV),
            aspectRatio,
            CAMERA_NEAR_PLANE,
            CAMERA_FAR_PLANE
        );
    }
}

int main()
{
    // -------------------------------------------------
    // Window + OpenGL initialization
    // -------------------------------------------------
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW.\n";
        return 1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        "ZEBRA Engine",
        nullptr,
        nullptr
    );

    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window.\n";
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    const int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0)
    {
        std::cerr << "Failed to initialize GLAD.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    std::cout << "Loaded OpenGL "
              << GLAD_VERSION_MAJOR(version)
              << '.'
              << GLAD_VERSION_MINOR(version)
              << '\n';

    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    glfwSwapInterval(1); // 1 = VSync on, 0 = off.

    // Light state
    float lightAngle = 0.0f;
    float specularStrength = 0.5f;
    float shininess = 32.0f;
    float lightIntensity = 1.5f;

    // CAMERA POSITION AND ORIENTATION
    glm::vec3 cameraPosition{0.0f, 0.0f, 3.0f};
    float cameraYaw = -90.0f;
    float cameraPitch = 0.0f;

    // -------------------------------------------------
    // Mesh GPU resources
    // -------------------------------------------------

    unsigned int cubeVAO = 0;
    unsigned int cubeVBO = 0;
    unsigned int cubeEBO = 0;

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);

    //BIND CUBE MESH BUFFER
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(
            CUBE_INDEXED_VERTICES.size() * sizeof(Vertex)
        ),
        CUBE_INDEXED_VERTICES.data(),
        GL_STATIC_DRAW
    );
    glBindBuffer(
        GL_ELEMENT_ARRAY_BUFFER,
        cubeEBO
    );
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(
            CUBE_INDICES.size() * sizeof(unsigned int)
        ),
        CUBE_INDICES.data(),
        GL_STATIC_DRAW
    );
    //ATTRIBUTE POINTERS CUBE MESH
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, x)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, r)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, nx)));
    glEnableVertexAttribArray(2);


    // -------------------------------------------------
    // Shader program
    // -------------------------------------------------
    const unsigned int shaderProgram = CreateShaderProgram();
    if (shaderProgram == 0)
    {
        glDeleteVertexArrays(1, &cubeVAO);
        glDeleteBuffers(1, &cubeVBO);
        glDeleteBuffers(1, &cubeEBO);
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    const int modelLocation =          glGetUniformLocation(shaderProgram, "model");
    const int viewLocation =           glGetUniformLocation(shaderProgram, "view");
    const int projectionLocation =     glGetUniformLocation(shaderProgram, "projection");
    const int lightDirectionLocation = glGetUniformLocation(shaderProgram, "lightDirection");
    const int lightColorLocation =     glGetUniformLocation(shaderProgram, "lightColor");
    const int cameraPositionLocation = glGetUniformLocation(shaderProgram, "cameraPosition");
    const int shininessLocation = glGetUniformLocation(shaderProgram, "shininess");
    const int specularStrengthLocation = glGetUniformLocation(shaderProgram, "specularStrength");
    const int lightIntensityLocation = glGetUniformLocation(shaderProgram, "lightIntensity");
    const int materialColorLocation = glGetUniformLocation( shaderProgram, "materialColor" );


    // -------------------------------------------------
    // Scene state
    // -------------------------------------------------

    glm::vec3 cubeAMaterialColor
    {
        1.0f,
        1.0f,
        1.0f
    };
    glm::vec3 cubeBMaterialColor
    {
        2.0f,
        1.0f,
        1.0f
    };
    glm::vec3 cubeCMaterialColor
    {
        .7f,
        2.3f,
        1.0f
    };

    float cubeBSpecularStrength = 1.0f;
    float cubeBShininess = 128.0f;

    float cubeASpecularStrength = 0.05f;
    float cubeAShininess = 4.0f;

    glm::mat4 modelA{1.0f};
    modelA = glm::translate(
        modelA,
        glm::vec3(1.50f, 0.0f, -5.0f)
    );
    modelA = glm::scale(
        modelA,
        glm::vec3(1.25f, 1.25f, 1.25f)
    );
    modelA = glm::rotate(
        modelA,
        glm::radians(45.0f),
        glm::vec3(1.0f, 1.0f, 0.0f)
    );

    glm::mat4 modelB{1.0f};
    modelB = glm::translate(
        modelB,
        glm::vec3(-1.50f, 0.0f, -5.0f)
    );

    glm::mat4 modelC{1.0f};
    modelC = glm::translate(
        modelC,
    glm::vec3(0.0f, -5.0f, 0.0f)
    );
    modelC = glm::scale(
        modelC,
        glm::vec3(50.0f, 0.1f, 50.0f)
    );



    double previousTime = glfwGetTime();
    
    glClearColor(0.5f, 0.75f, 0.9f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);

    // -------------------------------------------------
    // Application loop
    // -------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Time
        const double currentTime =  glfwGetTime();
        const float deltaTime =     static_cast<float>(currentTime - previousTime);
        previousTime =              currentTime;

        // Input + camera state
        ProcessCameraInput( window, deltaTime, cameraPosition, cameraYaw, cameraPitch );

        lightAngle += deltaTime * 0.25f;

        lightDirection.x = glm::cos(lightAngle);
        lightDirection.y = 0.85f;
        lightDirection.z = glm::sin(lightAngle);

        const glm::vec3 cameraForward = CalculateCameraForward(cameraYaw, cameraPitch);
        const glm::vec3 cameraRight =   glm::normalize(glm::cross(cameraForward, worldUp));
        const glm::vec3 cameraUp =      glm::normalize(glm::cross(cameraRight, cameraForward));

        // Current framebuffer size drives the projection aspect ratio.
        glfwGetFramebufferSize( window, &framebufferWidth, &framebufferHeight );

        // prevents the projection calculation from dividing by 0 when window is minimized.
        if (framebufferWidth <= 0 || framebufferHeight <= 0)
        {
            continue;
        }

        // RE-BUILD VIEW AND PROJECTION MATRIXS 
        const glm::mat4 view =         BuildViewMatrix(cameraPosition, cameraForward, cameraUp);
        const glm::mat4 projection =   BuildProjectionMatrix(framebufferWidth, framebufferHeight);

        // RENDER
        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT
        );

        glUseProgram(shaderProgram);

        //SHADERS UPLOADS
        glUniform3fv(
            lightDirectionLocation,
            1,
            glm::value_ptr(lightDirection)
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
        glUniform3fv(
            lightColorLocation,
            1,
            glm::value_ptr(lightColor)
        );
        glUniform3fv(
            cameraPositionLocation,
            1,
            glm::value_ptr(cameraPosition)
        );
        // glUniform1f(
        //     shininessLocation,
        //     shininess
        // );
        // glUniform1f(
        //     specularStrengthLocation,
        //     specularStrength
        // );
        glUniform1f(
            lightIntensityLocation,
            lightIntensity
        );
        // CUBE MESH WITH EBO
        {        
        glBindVertexArray(cubeVAO);
        
        //MODEL A
        glUniform1f(
            specularStrengthLocation,
            cubeASpecularStrength
        );
        glUniform1f(
            shininessLocation,
            cubeAShininess
        );
        glUniform3fv(
            materialColorLocation,
            1,
            glm::value_ptr(cubeAMaterialColor)
        );
        modelA = glm::rotate(
            modelA,
            glm::radians(50.0f * deltaTime),
            glm::vec3(1.0f, 1.0f, 0.0f)
        );
        glUniformMatrix4fv(
            modelLocation,
            1,
            GL_FALSE,
            glm::value_ptr(modelA)
        );

        glDrawElements(
            GL_TRIANGLES,
            static_cast<GLsizei>(CUBE_INDICES.size()),
            GL_UNSIGNED_INT,
            nullptr
        );

        //MODEL B
        glUniform1f(
            specularStrengthLocation,
            cubeBSpecularStrength
        );
        glUniform1f(
            shininessLocation,
            cubeBShininess
        );
        glUniform3fv(
            materialColorLocation,
            1,
            glm::value_ptr(cubeBMaterialColor)
        );
        glUniformMatrix4fv(
            modelLocation,
            1,
            GL_FALSE,
            glm::value_ptr(modelB)
        );
        modelB = glm::rotate(
            modelB,
            glm::radians(50.0f * deltaTime),
            glm::vec3(-1.0f, -1.0f, 0.0f)
        );
        glDrawElements(
            GL_TRIANGLES,
            static_cast<GLsizei>(CUBE_INDICES.size()),
            GL_UNSIGNED_INT,
            nullptr
        );

        //MODEL C
        glUniform1f(
            specularStrengthLocation,
            cubeASpecularStrength
        );
        glUniform1f(
            shininessLocation,
            cubeAShininess
        );
        glUniform3fv(
            materialColorLocation,
            1,
            glm::value_ptr(cubeCMaterialColor)
        );
        glUniformMatrix4fv(
            modelLocation,
            1,
            GL_FALSE,
            glm::value_ptr(modelC)
        );
        glDrawElements(
            GL_TRIANGLES,
            static_cast<GLsizei>(CUBE_INDICES.size()),
            GL_UNSIGNED_INT,
            nullptr
        );
        
        //MODEL D
        glm::mat4 modelD{1.0f};

        modelD = glm::translate(
            modelD,
            glm::vec3(
                10.0f * glm::cos(lightAngle),
                2.0f,
                10.0f * glm::sin(lightAngle)
            )
        );
        modelD = glm::scale(
            modelD,
            glm::vec3(1.75f)
        );
        glUniformMatrix4fv(
            modelLocation,
            1,
            GL_FALSE,
            glm::value_ptr(modelD)
        );
        glDrawElements(
            GL_TRIANGLES,
            static_cast<GLsizei>(CUBE_INDICES.size()),
            GL_UNSIGNED_INT,
            nullptr
        );
        
    }

        glfwSwapBuffers(window);
    }

    // -------------------------------------------------
    // Cleanup
    // -------------------------------------------------

    glDeleteProgram(shaderProgram);

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &cubeEBO);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}





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