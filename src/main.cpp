;//ZEBRA ENGINE
#include <array>
#include <vector>
#include <cstddef>
#include <iostream>
#include <algorithm>
#include <utility>

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

//ZEBRA ENGINE
namespace
{
    constexpr int WINDOW_WIDTH = 1920;
    constexpr int WINDOW_HEIGHT = 1080;

    constexpr float CAMERA_SPEED = 18.0f;
    constexpr float CAMERA_SPEED_BOOST = 2.0f;
    constexpr float CAMERA_ROTATION_SPEED = 150.0f;

    constexpr float CAMERA_FOV = 65.0f;
    constexpr float CAMERA_NEAR_PLANE = 0.1f;
    constexpr float CAMERA_FAR_PLANE = 300.0f;

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
        // UV
        float u;
        float v;
    };
    struct Texture
    {
        unsigned int id = 0;

        int width = 0;
        int height = 0;

        Texture() = default;

        ~Texture()
        {
            if (id != 0)
            {
                glDeleteTextures( 1, &id );
            }
        }

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        Texture(Texture&& other) noexcept
        {
            id = other.id;
            width = other.width;
            height = other.height;

            other.id = 0;
            other.width = 0;
            other.height = 0;
        }

        Texture& operator=(Texture&& other) noexcept
        {
            if (this != &other)
            {
                if (id != 0)
                {
                    glDeleteTextures( 1, &id );
                }

                id = other.id;
                width = other.width;
                height = other.height;

                other.id = 0;
                other.width = 0;
                other.height = 0;
            }
            return *this;
        }
    };
    struct Material
    {
        const Texture* texture;

        glm::vec3 color;
        float opacity;
        
        float specularStrength;
        float shininess;

        glm::vec2 uvOffset;
        glm::vec2 uvScale;
        glm::vec2 uvTiling;
    };
    struct Mesh
    {
        unsigned int vao = 0;
        unsigned int vbo = 0;
        unsigned int ebo = 0;

        GLsizei indicesCount = 0;

        Mesh() = default;

        ~Mesh()
        {
            if (vao != 0) { glDeleteVertexArrays(1, &vao); }
            if (vbo != 0) { glDeleteBuffers(1, &vbo); }
            if (ebo != 0) { glDeleteBuffers(1, &ebo); }
        }        
        Mesh(const Mesh&) = delete;

        Mesh& operator=(const Mesh&) = delete;

        Mesh(Mesh&& other) noexcept
        {
            vao = other.vao;
            vbo = other.vbo;
            ebo = other.ebo;
            indicesCount = other.indicesCount;

            other.vao = 0;
            other.vbo = 0;
            other.ebo = 0;
            other.indicesCount = 0;
        }

        Mesh& operator=(Mesh&& other) noexcept
        {
            if (this != &other)
            {
                if (vao != 0)
                {
                    glDeleteVertexArrays(1, &vao);
                }

                if (vbo != 0)
                {
                    glDeleteBuffers(1, &vbo);
                }

                if (ebo != 0)
                {
                    glDeleteBuffers(1, &ebo);
                }

                vao = other.vao;
                vbo = other.vbo;
                ebo = other.ebo;
                indicesCount = other.indicesCount;

                other.vao = 0;
                other.vbo = 0;
                other.ebo = 0;
                other.indicesCount = 0;
            }

            return *this;
        }
    };
    struct AtlasRegion
    {
        glm::vec2 offset;
        glm::vec2 scale;
    };
    struct Transform
    {
        glm::vec3 position;
        glm::vec3 scale;

        float rotationDegrees;
        glm::vec3 rotationAxis;
    };
    struct Renderable
    {
        const Mesh* mesh;
        const Material* material;

        Transform transform;
    };
    struct Camera
    {
        glm::vec3 position;

        float yaw;
        float pitch;
    };
    struct ObjVertexIndex
    {
        int positionIndex;
        int texCoordIndex;
        int normalIndex;
    };
    struct ObjSubmesh
    {
        std::string materialName;

        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
    };
    struct ObjMaterial
    {
        std::string name;
        std::string diffuseTexturePath;
    };
    struct ObjData
    {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> texCoords;
        std::vector<glm::vec3> normals;

        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        std::vector<std::string> triangleMaterials;
        std::vector<ObjSubmesh> submeshes;
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
        {-0.5f, -0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f},
        { 0.5f, -0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f},
        { 0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f},
        {-0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f},

        // Back: 4-7
        {-0.5f, -0.5f, -0.5f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f, -1.0f ,   0.0f, 0.0f},
        { 0.5f, -0.5f, -0.5f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f, -1.0f ,   1.0f, 0.0f},
        { 0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f, -1.0f ,   1.0f, 1.0f},
        {-0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 1.0f,  0.0f, 0.0f, -1.0f ,   0.0f, 1.0f},

        // Right: 8-11
        { 0.5f, -0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f},
        { 0.5f, -0.5f, -0.5f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f},
        { 0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f},
        { 0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f},

        // left: 12-15
        {-0.5f, -0.5f,  0.5f,   1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f},
        {-0.5f, -0.5f, -0.5f,   1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,   1.0f, 0.0f},
        {-0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f},
        {-0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,   0.0f, 1.0f},

        // Top: 16-19
        {-0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f},
        { 0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f},
        { 0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f},
        {-0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f},

        // Bottom: 20-23
        {-0.5f, -0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f},
        { 0.5f, -0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f},
        { 0.5f, -0.5f, -0.5f,   1.0f, 1.0f, 1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f},
        {-0.5f, -0.5f, -0.5f,   1.0f, 1.0f, 1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f}
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
        12, 14, 13,
        12, 15, 14,
        // Top  
        16, 17, 18,
        16, 18, 19,
        // Bottom
        20, 22, 21,
        20, 23, 22
    }};
    constexpr std::array<Vertex, 4> PLANE_VERTICES =
    {{
        // Position                Color             Normal             UV
        {-0.5f, 0.0f,  0.5f,     1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f},
        { 0.5f, 0.0f,  0.5f,     1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f},
        { 0.5f, 0.0f, -0.5f,     1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f},
        {-0.5f, 0.0f, -0.5f,     1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f}
    }};
    constexpr std::array<unsigned int, 6> PLANE_INDICES =
    {{
        0, 1, 2,
        0, 2, 3
    }};
    constexpr std::array<Vertex, 4> CARD_VERTICES =
    {{
        // Position                Color             Normal             UV
        {-0.5f,  -0.5f, 0.0f,     1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f},
        { 0.5f,  -0.5f, 0.0f,     1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f},
        { 0.5f,   0.5f, 0.0f,     1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f},
        {-0.5f,   0.5f, 0.0f,     1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f}
    }};
    constexpr std::array<unsigned int, 6> CARD_INDICES =
    {{
        0, 1, 2,
        0, 2, 3
    }};


// -------------------------------------------------
// SHADERS
// -------------------------------------------------
    constexpr const char* VERTEX_SHADER_SOURCE = {R"(
        #version 330 core

        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 color;
        layout(location = 2) in vec3 normal;
        layout(location = 3) in vec2 texCoord;

        out vec3 vertexColor;
        out vec3 vertexNormal;
        out vec3 vertexWorldPosition;
        out vec2 vertexTexCoord;

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
            vertexTexCoord = texCoord;
        }
    )"};
    constexpr const char* FRAGMENT_SHADER_SOURCE = {R"(
        #version 330 core

        in vec3 vertexColor;
        in vec3 vertexNormal;
        in vec3 vertexWorldPosition;
        in vec2 vertexTexCoord;
        
        out vec4 fragmentColor;

        uniform vec3 lightDirection;
        uniform vec3 lightColor;
        uniform vec3 cameraPosition;

        uniform sampler2D textureSampler;

        uniform float specularStrength;
        uniform float shininess;
        uniform float lightIntensity;
        uniform vec3 materialColor;
        uniform float materialOpacity;

        uniform vec2 uvOffset;
        uniform vec2 uvScale;
        uniform vec2 uvTiling;

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

            if (diffuse > 0.0) { 
                specular = pow( max(dot(R, V), 0.0), shininess ); 
            }
                        
            vec3 diffuseLight = lightColor * diffuse * lightIntensity;
            vec3 ambientLight = vec3(0.75) * lightColor;
            vec3 specularLight = lightColor * lightIntensity * specularStrength * specular;

            vec3 baseColor = vertexColor * materialColor;
            vec3 litColor = baseColor * (ambientLight + diffuseLight ) + specularLight;
            
            vec2 tiledUV = fract(vertexTexCoord * uvTiling);
            vec2 atlasUV = uvOffset + tiledUV * uvScale;
            vec4 textureColor = texture( textureSampler, atlasUV );
            if (textureColor.a < 0.5)
            {
                discard;
            }

            //fragmentColor = vec4(litColor,1.0);
            fragmentColor =    textureColor * vec4(litColor, materialOpacity);
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
        const unsigned int vertexShader = CompileShader(GL_VERTEX_SHADER, VERTEX_SHADER_SOURCE);

        if (vertexShader == 0)
            return 0;

        const unsigned int fragmentShader = CompileShader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_SOURCE);

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

    void ProcessCameraInput(GLFWwindow* window, float deltaTime, Camera& camera)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        {
            camera.yaw -= CAMERA_ROTATION_SPEED * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            camera.yaw += CAMERA_ROTATION_SPEED * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            camera.pitch += CAMERA_ROTATION_SPEED * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            camera.pitch -= CAMERA_ROTATION_SPEED * deltaTime;
        }

        camera.pitch = glm::clamp( camera.pitch, -89.0f, 89.0f );
        glm::vec3 cameraForward = CalculateCameraForward(camera.yaw, camera.pitch);
        glm::vec3 cameraRight = glm::normalize( glm::cross(cameraForward, worldUp) );
        glm::vec3 cameraUp = glm::normalize( glm::cross(cameraRight, cameraForward) );

        float cameraMovement = CAMERA_SPEED * deltaTime;

        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            cameraMovement *= CAMERA_SPEED_BOOST;
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            camera.position += cameraForward * cameraMovement;
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            camera.position -= cameraForward * cameraMovement;
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            camera.position  += cameraRight * cameraMovement;
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            camera.position  -= cameraRight * cameraMovement;
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        {
            camera.position  += cameraUp * cameraMovement;
        }

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        {
            camera.position  -= cameraUp * cameraMovement;
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
    glm::mat4 BuildModelMatrix( const Transform& transform)
    {
        glm::mat4 model{1.0f};

        model = glm::translate(
            model,
            transform.position
        );

        model = glm::rotate(
            model,
            glm::radians(transform.rotationDegrees),
            transform.rotationAxis
        );

        model = glm::scale(
            model,
            transform.scale
        );

        return model;
    }
   
    AtlasRegion MakeAtlasRegion( int x, int y, int width, int height, int atlasWidth, int atlasHeight )
    {
        AtlasRegion region;

        region.scale =
        {
            static_cast<float>(width) / atlasWidth,
            static_cast<float>(height) / atlasHeight
        };

        region.offset =
        {
            static_cast<float>(x) / atlasWidth,
            1.0f - static_cast<float>(y + height) / atlasHeight
        };
        return region;
    }
    
    Texture LoadTexture(const char* filePath)
    {
        Texture texture;

        int channels = 0;

        unsigned char* pixels = stbi_load( filePath, &texture.width, &texture.height, &channels, STBI_rgb_alpha );

        if (pixels == nullptr)
        {
            std::cerr
                << "Failed to load texture: "
                << filePath
                << '\n'
                << stbi_failure_reason()
                << '\n';
            return texture;
        }

        glGenTextures( 1, &texture.id );
        glBindTexture( GL_TEXTURE_2D, texture.id );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexImage2D( 
            GL_TEXTURE_2D, 
            0, GL_RGBA, texture.width, texture.height, 
            0, GL_RGBA, GL_UNSIGNED_BYTE, pixels 
        );
        // glGenerateMipmap( GL_TEXTURE_2D );
        stbi_image_free(pixels);
        
        return texture;
    }

    void DrawRenderable( const Renderable& object, int modelLocation, int materialColorLocation,int materialOpacityLocation, int specularStrengthLocation, int shininessLocation, int uvOffsetLocation, int uvScaleLocation, int uvTilingLocation)
    {
        const glm::mat4 model = BuildModelMatrix(object.transform);

        glUniform3fv( materialColorLocation, 1, glm::value_ptr(object.material->color) );
        glUniform1f( materialOpacityLocation, object.material->opacity );
        glUniform1f( specularStrengthLocation, object.material->specularStrength );
        glUniform1f( shininessLocation, object.material->shininess );
        glUniformMatrix4fv( modelLocation, 1, GL_FALSE, glm::value_ptr(model) );
        glUniform2fv( uvOffsetLocation, 1, glm::value_ptr(object.material->uvOffset) );
        glUniform2fv( uvScaleLocation, 1, glm::value_ptr(object.material->uvScale) );
        glUniform2fv( uvTilingLocation, 1, glm::value_ptr(object.material->uvTiling) );
        
        glBindTexture( GL_TEXTURE_2D, object.material->texture->id );
        glBindVertexArray( object.mesh->vao );
        glDrawElements( GL_TRIANGLES, object.mesh->indicesCount, GL_UNSIGNED_INT, nullptr );
    }

    Mesh CreateMesh( const Vertex* vertices, std::size_t vertexCount, const unsigned int* indices, std::size_t indicesCount )
    {
        Mesh mesh;
        mesh.indicesCount = static_cast<GLsizei>(indicesCount);
        // {
        //     0, 0, 0, static_cast<GLsizei>(indicesCount)
        // };

        glGenVertexArrays( 1, &mesh.vao );
        glGenBuffers( 1, &mesh.vbo );
        glGenBuffers( 1, &mesh.ebo );
        
        glBindVertexArray(mesh.vao);

        glBindBuffer( GL_ARRAY_BUFFER, mesh.vbo );
        glBufferData(
            GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(vertexCount * sizeof(Vertex)),
            vertices,
            GL_STATIC_DRAW
        );
        
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mesh.ebo );
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            static_cast<GLsizeiptr>( indicesCount * sizeof(unsigned int) ),
            indices,
            GL_STATIC_DRAW
        );

        //ATTRIBUTE POINTERS
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, x)));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, r)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, nx)));
        glEnableVertexAttribArray(2);

        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, u)));
        glEnableVertexAttribArray(3);

        return mesh;
    }

// -------------------------------------------------
// OBJECT LOADING
// -------------------------------------------------
    void BuildObjSubmeshes(ObjData& data)
    {
        for ( std::size_t triangleIndex = 0; triangleIndex < data.triangleMaterials.size(); triangleIndex++ )
        {
            const std::string& materialName = data.triangleMaterials[triangleIndex];
            std::size_t submeshIndex = data.submeshes.size();

            // Look for an existing submesh with this material.
            for ( std::size_t i = 0; i < data.submeshes.size(); ++i )
            {
                if ( data.submeshes[i].materialName == materialName )
                {
                    submeshIndex = i;
                    break;
                }
            }

            // If one didn't exist, create it/default one. because data.submeshes.size() represents an out of bounds. 
            //Then the code below starts filling it.
            if (submeshIndex == data.submeshes.size())
            {
                data.submeshes.push_back(
                    ObjSubmesh
                    {
                        materialName, {}, {}
                    }
                );
                submeshIndex = data.submeshes.size() - 1;
            }

            ObjSubmesh& submesh = data.submeshes[submeshIndex];

            //EX: Triangle 2: vertices[6] vertices[7] vertices[8]
            //    firstVertex = 2 * 3; = 6;
            const std::size_t firstVertex = triangleIndex * 3;

            for ( std::size_t corner = 0; corner < 3; ++corner )
            {
                submesh.vertices.push_back(
                     data.vertices[ firstVertex + corner ]
                );
                submesh.indices.push_back(
                    static_cast<unsigned int>( submesh.vertices.size() - 1 )
                );
            }
        }
    }
    ObjVertexIndex ParseObjVertexIndex( const std::string& token )
        {
            ObjVertexIndex result{};

            std::istringstream stream(token);

            std::string position;
            std::string texCoord;
            std::string normal;

            std::getline(stream, position, '/');
            std::getline(stream, texCoord, '/');
            std::getline(stream, normal, '/');

            result.positionIndex = std::stoi(position) - 1;
            result.texCoordIndex = std::stoi(texCoord) - 1;
            result.normalIndex = std::stoi(normal) - 1;

            return result;
        }
    Vertex BuildVertexFromObj( const ObjData& data, const ObjVertexIndex& index )
    {
        const glm::vec3& position = data.positions[index.positionIndex];
        const glm::vec2& texCoord = data.texCoords[index.texCoordIndex];
        const glm::vec3& normal = data.normals[index.normalIndex];

        return Vertex
        {
            position.x, position.y, position.z,
            1.0f, 1.0f, 1.0f,
            normal.x, normal.y, normal.z,
            texCoord.x, texCoord.y
        };
    }
    ObjData LoadObjData(const char* filePath)
    {
        ObjData data;
        std::ifstream file(filePath);
        std::string currentMaterial;

        if (!file)
        {
            std::cerr << "Failed to open OBJ: " << filePath << '\n';
            return data;
        }

        std::string line;

        while (std::getline(file, line))
        {
            std::istringstream stream(line);
            std::string prefix;
            stream >> prefix;

            if (prefix == "v")
            {
                glm::vec3 position;

                stream >> position.x >> position.y >> position.z;

                data.positions.push_back(position);
            }
            else if (prefix == "vt")
            {
                glm::vec2 texCoord;

                stream >> texCoord.x >> texCoord.y;

                data.texCoords.push_back(texCoord);
            }
            else if (prefix == "vn")
            {
                glm::vec3 normal;

                stream >> normal.x >> normal.y >> normal.z;

                data.normals.push_back(normal);
            }
            else if (prefix == "f")
            {
                std::vector<std::string> faceTokens;

                std::string token;

                //TRIANGULATE POLYGONAL FACES INTO TRIANGLES
                while (stream >> token)
                {
                    faceTokens.push_back(token);
                }     
                if (faceTokens.size() < 3)
                {
                    continue;
                }     
                for (std::size_t i = 1; i + 1 < faceTokens.size(); i++)
                {
                    const ObjVertexIndex indexA = ParseObjVertexIndex(faceTokens[0]);
                    const ObjVertexIndex indexB = ParseObjVertexIndex(faceTokens[i]);
                    const ObjVertexIndex indexC = ParseObjVertexIndex(faceTokens[i + 1]);

                    data.vertices.push_back( BuildVertexFromObj(data, indexA) );
                    data.indices.push_back( static_cast<unsigned int>( data.vertices.size() - 1 ) );

                    data.vertices.push_back( BuildVertexFromObj(data, indexB) );
                    data.indices.push_back( static_cast<unsigned int>( data.vertices.size() - 1 ) );

                    data.vertices.push_back( BuildVertexFromObj(data, indexC) );
                    data.indices.push_back( static_cast<unsigned int>( data.vertices.size() - 1 ) );
                    
                    data.triangleMaterials.push_back( currentMaterial );
                }
            }
            else if (prefix == "usemtl")
            {
                stream >> currentMaterial;
            }
        }
        BuildObjSubmeshes(data);
        return data;
    }
    std::vector<ObjMaterial> LoadMtlData( const char* filePath )
    {
        std::vector<ObjMaterial> materials;

        std::ifstream file(filePath);

        if (!file)
        {
            std::cerr << "Failed to open MTL: " << filePath << '\n';

            return materials;
        }

        ObjMaterial* currentMaterial = nullptr;

        std::string line;

        while (std::getline(file, line))
        {
            std::istringstream stream(line);

            std::string prefix;
            stream >> prefix;

            if (prefix == "newmtl")
            {
                std::string materialName;
                stream >> materialName;

                materials.push_back(
                    ObjMaterial
                    {
                        materialName,
                        ""
                    }
                );
                currentMaterial = &materials.back();
            }
            else if ( prefix == "map_Kd" && currentMaterial != nullptr )
            {
                std::getline(
                    stream >> std::ws,
                    currentMaterial->diffuseTexturePath
                );
            }
        }

        return materials;
    }
    std::string ResolveImportedTexturePath( const std::string& importedPath )
    {
        const std::filesystem::path sourcePath(importedPath);

        const std::filesystem::path zebraPath =
            std::filesystem::path("assets/textures") / sourcePath.filename();

        return zebraPath.string();
    }
    std::size_t FindObjMaterialIndex( const std::vector<ObjMaterial>& materials, const std::string& materialName )
    {
        for (std::size_t i = 0; i < materials.size(); ++i)
        {
            if (materials[i].name == materialName)
            {
                return i;
            }
        }

        return materials.size();
    };

    const Texture* GetOrLoadTexture( std::unordered_map<std::string, Texture>& cache, const std::string& path )
    {
        const std::filesystem::path normalizedPath = std::filesystem::path(path).lexically_normal();

        const std::string cacheKey = normalizedPath.generic_string();

        auto existing = cache.find(cacheKey);

        if (existing != cache.end())
        {
            std::cout << "Texture cache HIT: " << cacheKey << '\n';

            return &existing->second;
        }

        Texture texture = LoadTexture(normalizedPath.string().c_str());

        if (texture.id == 0)
        {
            return nullptr;
        }

        auto inserted = cache.emplace(cacheKey, std::move(texture));

        std::cout << "Texture cache MISS - loaded: " << cacheKey << '\n';

        return &inserted.first->second;
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

    GLFWwindow* window = glfwCreateWindow( WINDOW_WIDTH, WINDOW_HEIGHT, "ZEBRA Engine", nullptr, nullptr );

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

    std::cout << "Loaded OpenGL " << GLAD_VERSION_MAJOR(version) << '.' << GLAD_VERSION_MINOR(version) << '\n';
              
// ---------------------------------------------
// OPENGL RESOURCE LIFETIME
// ---------------------------------------------
    {
        glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

        int framebufferWidth = 0;
        int framebufferHeight = 0;
        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
        glViewport(0, 0, framebufferWidth, framebufferHeight);

        glfwSwapInterval(1); // 1 = VSync on, 0 = off.
        
        bool gWasDown = false; // for spawning "things" on key down
        bool rWasDown = false; // for de-spawning "things" on key down

// -------------------------------------------------
// LOAD OBJ FILE TEST WITH ORC IMPORT
// -------------------------------------------------
        ObjData orc_Obj = LoadObjData("assets/models/TD_EnemyModels.obj");
        std::vector<ObjMaterial> objMaterials = LoadMtlData( "assets/models/TD_EnemyModels.mtl" );

        // std::vector<Texture> orcTextures;
        // orcTextures.reserve( objMaterials.size() );

        std::unordered_map<std::string, Texture> textureCache;
        stbi_set_flip_vertically_on_load(true);

        std::vector<Material> orcMaterials;
        orcMaterials.reserve( objMaterials.size() );

        for (const ObjMaterial& objMaterial : objMaterials)
        {
            const std::string texturePath = ResolveImportedTexturePath( objMaterial.diffuseTexturePath );
            const Texture* importedTexture = GetOrLoadTexture( textureCache, texturePath );

            if (importedTexture == nullptr)
            {
                std::cerr << "Failed to load imported texture: " << texturePath << '\n';
                return 1;
            }

            orcMaterials.push_back(
                Material
                {
                    importedTexture,

                    glm::vec3(1.0f),
                    1.0f,

                    0.2f,
                    16.0f,

                    glm::vec2(0.0f),
                    glm::vec2(1.0f),
                    glm::vec2(1.0f)
                }
            );
        }

        std::vector<Mesh> orcMeshes;
        orcMeshes.reserve( orc_Obj.submeshes.size() );

        for (const ObjSubmesh& submesh : orc_Obj.submeshes)
        {
            orcMeshes.push_back(
                CreateMesh(
                    submesh.vertices.data(),
                    submesh.vertices.size(),
                    submesh.indices.data(),
                    submesh.indices.size()
                )
            );
        }

        Transform orcTransform
        {
            glm::vec3(0.0f, 0.0f, -10.0f),
            glm::vec3(1.0f),

            0.0f,
            glm::vec3(0.0f, 1.0f, 0.0f)
        };

        std::vector<Renderable> orcRenderables;
        orcRenderables.reserve( orc_Obj.submeshes.size() );

        for (std::size_t i = 0; i < orc_Obj.submeshes.size(); ++i)
        {
            const ObjSubmesh& submesh = orc_Obj.submeshes[i];

            const std::size_t materialIndex = FindObjMaterialIndex( objMaterials, submesh.materialName );
            if (materialIndex == objMaterials.size())
            {
                std::cerr << "No runtime material for submesh: " << submesh.materialName << '\n';
                continue;
            }

            orcRenderables.push_back(
                Renderable
                {
                    &orcMeshes[i], &orcMaterials[materialIndex], orcTransform
                }
            );
        }

// -------------------------------------------------
// LOAD TEXTURE AND COOROSPONDING UNIFORM DATA
// -------------------------------------------------
        stbi_set_flip_vertically_on_load(true);
        const Texture* texture = GetOrLoadTexture(textureCache,"assets/textures/TexturePallete_512x512_MedievalPack.png");
        const Texture* secondTexture = GetOrLoadTexture( textureCache,"assets/textures/TexturePallete_LowPoly_512x512_MedievalPack.png");

        if (texture == nullptr || secondTexture == nullptr)
        {
            return 1;
        }

        const AtlasRegion brickRegion = MakeAtlasRegion( 128, 160, 32, 32, texture->width, texture->height );
        const AtlasRegion woodRegion = MakeAtlasRegion( 160, 256, 32, 32, texture->width, texture->height );
        const AtlasRegion grassRegion = MakeAtlasRegion( 32, 160, 32, 32,  texture->width, texture->height );
        const AtlasRegion fireRegion = MakeAtlasRegion( 0, 0, 512, 512,  secondTexture->width, secondTexture->height );
        const AtlasRegion grassCardRegion = MakeAtlasRegion( 3, 472, 32, 32,  texture->width, texture->height );

        std::cout << "Unique GPU textures: " << textureCache.size() << '\n';
// -------------------------------------------------
// LIGHT AND CAMERA STATE
// -------------------------------------------------
        float lightAngle = 0.0f;
        float lightIntensity = 3.0f;

        //camera
        Camera camera
        {
            glm::vec3(0.0f, 6.5f, 5.0f),
            -90.0f,
            0.0f
        };
        
// -------------------------------------------------
// Mesh GPU resources
// -------------------------------------------------
        
        //CUBE MESH
        Mesh cubeMesh =
            CreateMesh(
                CUBE_INDEXED_VERTICES.data(),
                CUBE_INDEXED_VERTICES.size(),
                CUBE_INDICES.data(),
                CUBE_INDICES.size()
            );
        //PLANE MESH
        Mesh planeMesh =
            CreateMesh(
                PLANE_VERTICES.data(),
                PLANE_VERTICES.size(),
                PLANE_INDICES.data(),
                PLANE_INDICES.size()
            );
        Mesh cardMesh  = 
            CreateMesh(
                CARD_VERTICES.data(),
                CARD_VERTICES.size(),
                CARD_INDICES.data(),
                CARD_INDICES.size()
            );
// -------------------------------------------------
// Shader program
// -------------------------------------------------
        const unsigned int shaderProgram = CreateShaderProgram();
        if (shaderProgram == 0)
        {
            std::cerr << "Failed to create shader program.\n";
            return 1;
        }
        
// -------------------------------------------------
//UNIFORM LOCATIONS
// -------------------------------------------------
        const int modelLocation =          glGetUniformLocation(shaderProgram, "model");
        const int viewLocation =           glGetUniformLocation(shaderProgram, "view");
        const int projectionLocation =     glGetUniformLocation(shaderProgram, "projection");
        const int lightDirectionLocation = glGetUniformLocation(shaderProgram, "lightDirection");
        const int lightColorLocation =     glGetUniformLocation(shaderProgram, "lightColor");
        const int cameraPositionLocation = glGetUniformLocation(shaderProgram, "cameraPosition");
        const int shininessLocation =       glGetUniformLocation(shaderProgram, "shininess");
        const int specularStrengthLocation = glGetUniformLocation(shaderProgram, "specularStrength");
        const int lightIntensityLocation = glGetUniformLocation(shaderProgram, "lightIntensity");
        const int materialColorLocation = glGetUniformLocation( shaderProgram, "materialColor" );
        const int textureSamplerLocation = glGetUniformLocation(shaderProgram,"textureSampler" );
        const int uvOffsetLocation = glGetUniformLocation( shaderProgram, "uvOffset" );
        const int uvScaleLocation = glGetUniformLocation( shaderProgram, "uvScale" );
        const int uvTilingLocation = glGetUniformLocation( shaderProgram, "uvTiling" );
        const int materialOpacityLocation = glGetUniformLocation( shaderProgram, "materialOpacity" );
        
// -------------------------------------------------
// Scene state
// -------------------------------------------------
        
        Material material_brick
        {
            texture,
            
            glm::vec3(1.0f),
            1.0f,          // opacity

            .50f,
            16.0f,

            brickRegion.offset,
            brickRegion.scale,
            glm::vec2(1.5f,2.0f)
        };
        Material material_wood
        {
            texture,
            
            glm::vec3(1.0f),
            1.0f,          // opacity

            0.1f,
            2.0f,

            woodRegion.offset,
            woodRegion.scale,
            glm::vec2(1.0f)
        };
        Material material_grass
        {
            texture,
            
            glm::vec3(1.0f),
            1.0f,          // opacity

            0.05f,
            1.0f,

            grassRegion.offset,
            grassRegion.scale,
            glm::vec2(5.25f)
        };
        Material material_fire
        {
            secondTexture,

            glm::vec3(1.0f),
            1.0f,          // opacity

            1.0f,
            64.0f,

            fireRegion.offset,
            fireRegion.scale,
            glm::vec2(1.0f)
        };
        Material material_grass_card
        {
            texture,

            glm::vec3(1.0f),
            0.75f,          // opacity

            1.0f,
            64.0f,

            grassCardRegion.offset,
            grassCardRegion.scale,
            glm::vec2(1.0f)
        };

        Renderable objectA
        {
            &cubeMesh,
            &material_brick,
            Transform
            {
                glm::vec3(1.5f, 5.0f, -5.0f),
                glm::vec3(1.25f),

                45.0f,
                glm::vec3(1.0f, 1.0f, 0.0f)
            }
        };
        Renderable objectB
        {
            &cubeMesh,
            &material_wood,
            Transform
            {
                glm::vec3(-1.5f, 5.0f, -5.0f),
                glm::vec3(1.0f),

                0.0f,
                glm::vec3(-1.0f, -1.0f, 0.0f)
            }
        };
        Renderable objectC
        {
            &planeMesh,
            &material_grass,
            Transform
            {
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(50.0f, 1.0f, 50.0f),

                0.0f,
                glm::vec3(0.0f, 1.0f, 0.0f)
            }
        };
        Renderable objectD
        {
            &cubeMesh,
            &material_fire,
            // &cardMesh,
            // &material_grass_card,
            Transform
            {
                glm::vec3(0.0f, 10.0f, 0.0f),
                glm::vec3(2.5f),

                0.0f,
                glm::vec3(0.0f, 1.0f, 0.0f)
            }
        };
        Renderable TowerLeft
        {
            &cubeMesh,
            &material_brick,
            Transform
            {
                glm::vec3(-10.0f, 2.5f, -15.0f),
                glm::vec3(5.0f),

                0.0f,
                glm::vec3(0.0f, 1.0f, 0.0f)
            }
        };
        Renderable TowerRight
        {
            &cubeMesh,
            &material_brick,
            Transform
            {
                glm::vec3(5.0f, 2.5f, -15.0f),
                glm::vec3(5.0f),

                0.0f,
                glm::vec3(0.0f, 1.0f, 0.0f)
            }
        };
        Renderable Wall_1
        {
            &cubeMesh,
            &material_brick,
            Transform
            {
                glm::vec3(-5.0f, 1.5f, -15.0f),
                glm::vec3(5.0f, 3.0f, 2.0f),

                0.0f,
                glm::vec3(0.0f, 1.0f, 0.0f)
            }
        };
        Renderable Wall_2
        {
            &cubeMesh,
            &material_brick,
            Transform
            {
                glm::vec3(0.0f, 1.5f, -15.0f),
                glm::vec3(5.0f, 3.0f, 2.0f),

                0.0f,
                glm::vec3(0.0f, 1.0f, 0.0f)
            }
        };
        
        std::vector<Renderable*> opaqueScene
        {
            &objectA,
            &objectB,
            &objectC,
            &objectD,
            &TowerLeft,
            &TowerRight,
            &Wall_1,
            &Wall_2,
        };

        std::vector<Renderable> transparentScene;
        transparentScene.reserve(50);
        for (int i = 0; i < 10; ++i)
        {
            const float x =
                -10.0f + static_cast<float>(i) * 2.0f;

            transparentScene.push_back(
                Renderable
                {
                    &cardMesh,
                    &material_grass_card,

                    Transform
                    {
                        glm::vec3(x, .5f, -8.0f),
                        glm::vec3(1.0f),

                        0.0f,
                        glm::vec3(0.0f, 1.0f, 0.0f)
                    }
                }
            );
        }

        for (Renderable& part : orcRenderables)
        {
            opaqueScene.push_back(
                &part
            );
        }

        double previousTime = glfwGetTime();
        
        glClearColor(0.4f, 0.7f, 0.9f, 1.0f);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        glEnable(GL_BLEND);
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

// -------------------------------------------------
// APPLICATION LOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOP
// -------------------------------------------------
        while (!glfwWindowShouldClose(window))
        {

// -------------------------------------------------
// Events + Time
// -------------------------------------------------
            glfwPollEvents();
            
            const double currentTime = glfwGetTime();
            const float deltaTime = static_cast<float>( currentTime - previousTime );
            previousTime = currentTime;

// -------------------------------------------------
// UPDATE
// -------------------------------------------------

            // Camera state
            ProcessCameraInput( window, deltaTime, camera);

            // Light state
            lightAngle += deltaTime * 0.45f;
            
            lightDirection.x = glm::cos(lightAngle);
            lightDirection.y = 0.85f;
            lightDirection.z = glm::sin(lightAngle);

            // Object state
            objectA.transform.rotationDegrees += 50.0f * deltaTime;
            objectB.transform.rotationDegrees += 50.0f * deltaTime;
            objectD.transform.position.x = 28.0f * glm::cos(lightAngle);
            objectD.transform.position.z = 28.0f * glm::sin(lightAngle);

// -------------------------------------------------
// DERIVE FRAME DATA
// -------------------------------------------------
            const glm::vec3 cameraForward = CalculateCameraForward( camera.yaw, camera.pitch );
            const glm::vec3 cameraRight = glm::normalize( glm::cross( cameraForward, worldUp ) );
            const glm::vec3 cameraUp = glm::normalize( glm::cross( cameraRight, cameraForward ) );

            // Runtime grass spawning
            const bool gIsDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

            if (gIsDown && !gWasDown)
            {
                glm::vec3 spawnPosition = camera.position + cameraForward * 10.0f;
                float size = 1.5f;
                spawnPosition.y = size * .5f;

                transparentScene.push_back(
                    Renderable
                    {
                        &cardMesh,
                        &material_grass_card,

                        Transform
                        {
                            spawnPosition,
                            glm::vec3(size),

                            0.0f,
                            glm::vec3(0.0f, 1.0f, 0.0f)
                        }
                    }
                );
            }
            gWasDown = gIsDown; // if button is still down gWasDown stays true prevent repeated spawns per the if check

            const bool rIsDown = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
            if (rIsDown && !rWasDown)
            {
                if (!transparentScene.empty())
                {
                    auto nearest = std::min_element( transparentScene.begin(), transparentScene.end(),
                            [&camera]
                            ( const Renderable& a, const Renderable& b )
                            {
                                const glm::vec3 aToCamera = a.transform.position - camera.position;
                                const glm::vec3 bToCamera = b.transform.position - camera.position;

                                const float aDistanceSquared = glm::dot(aToCamera, aToCamera);
                                const float bDistanceSquared = glm::dot(bToCamera, bToCamera);
                                return aDistanceSquared < bDistanceSquared;
                            }
                        );

                    transparentScene.erase(nearest);
                }
            }
            rWasDown = rIsDown;

            glfwGetFramebufferSize( window, &framebufferWidth, &framebufferHeight );

            if( framebufferWidth <= 0 || framebufferHeight <= 0 ) 
            {
                continue; 
            }

            const glm::mat4 view = BuildViewMatrix( camera.position, cameraForward, cameraUp );
            const glm::mat4 projection = BuildProjectionMatrix( framebufferWidth, framebufferHeight );
// -------------------------------------------------
// BEGIN RENDER
// -------------------------------------------------

            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            glUseProgram(shaderProgram);

// -------------------------------------------------
// FRAME-WIDE UNIFORMS
// -------------------------------------------------
            
            glUniformMatrix4fv( viewLocation, 1, GL_FALSE, glm::value_ptr(view) );
            glUniformMatrix4fv( projectionLocation, 1, GL_FALSE, glm::value_ptr(projection) );
            glUniform3fv( cameraPositionLocation, 1, glm::value_ptr(camera.position) );
            glUniform3fv( lightDirectionLocation, 1, glm::value_ptr(lightDirection) );
            glUniform3fv( lightColorLocation, 1, glm::value_ptr(lightColor) );
            glUniform1f( lightIntensityLocation, lightIntensity );

// -------------------------------------------------
// TEXTURE STUFF
// -------------------------------------------------
            //The texture unit currently configuring is unit 0.
            glActiveTexture(GL_TEXTURE0); 
            //textureSampler should sample from Texture Unit 0.
            glUniform1i( textureSamplerLocation, 0 );

// -------------------------------------------------
// DRAW CUBE MESH
// -------------------------------------------------
            glDepthMask(GL_TRUE);
            for (const Renderable* object : opaqueScene)
            {
                DrawRenderable( 
                    *object, 
                    modelLocation,
                    materialColorLocation, 
                    materialOpacityLocation,
                    specularStrengthLocation, 
                    shininessLocation,
                    uvOffsetLocation, 
                    uvScaleLocation, 
                    uvTilingLocation 
                );
            }
            glDepthMask(GL_FALSE);

            std::sort(
                transparentScene.begin(),
                transparentScene.end(),

                [&camera]
                ( const Renderable& a, const Renderable& b )
                {
                    const glm::vec3 aToCamera = camera.position - a.transform.position;
                    const glm::vec3 bToCamera = camera.position - b.transform.position;
                    const float aDistanceSquared = glm::dot( aToCamera, aToCamera );
                    const float bDistanceSquared = glm::dot( bToCamera, bToCamera );

                    return aDistanceSquared > bDistanceSquared;
                }
            );

            for (const Renderable& object : transparentScene)
            {
                DrawRenderable( 
                    object, 
                    modelLocation,
                    materialColorLocation, 
                    materialOpacityLocation,
                    specularStrengthLocation, 
                    shininessLocation,
                    uvOffsetLocation, 
                    uvScaleLocation, 
                    uvTilingLocation 
                );
            }
            glDepthMask(GL_TRUE);

// -------------------------------------------------
// Present
// -------------------------------------------------
            glfwSwapBuffers(window);
        }

// -------------------------------------------------
// Cleanup
// -------------------------------------------------
        glDeleteProgram(shaderProgram);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}




//duck hunt clone
//dungeon crawler 2d-ish game
//something with wave function collapse
//particle life sim / conways game of life
//3d model uploader 
//sdfs
//top down grid like vertex defined that samples noise maps/height map to create "landmass/island" and height map for colors and also for shadows eventually


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

  //my experiment
    // std::vector<Renderable*> transparentScene;
    // transparentScene.reserve( grassObjects.size() );
    // for (Renderable& grass : grassObjects)
    // {
    //     transparentScene.push_back(&grass);
    // }

    // srand(time(NULL));
    // std::vector<Renderable> transparentScene;
    // transparentScene.reserve(100);

    // for (int i = 0; i < 50; i++)
    // {
    //     int rNumX = static_cast<float>(rand() % 50) +1;
    //     int rNumY = static_cast<float>(rand() % 50) +1;

    //     transparentScene.push_back(createRenderable(&cardMesh, &material_grass_card, rNumX, rNumY));
    // }