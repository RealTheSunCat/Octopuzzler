#pragma once

#include <unordered_map>

#include <GLAD/glad.h>
#include <glm/glm.hpp>

class Camera;

class Shader
{
public:
    // the shader program's ID
    GLuint ID = 0;

    Shader() = default;

    // constructor reads and builds the shader
    Shader(const GLchar* vertexName, const GLchar* fragmentName);

    void doProjView(Camera& _camera, int _width, int _height, bool doPos) const;

    // activate the shader
    void use() const;

    // utility uniform functions
    void setBool(const std::string& name, bool value) const;
    // ------------------------------------------------------------------------
    void setInt(const std::string& name, int value) const;
    // ------------------------------------------------------------------------
    void setFloat(const std::string& name, float value) const;
    // ------------------------------------------------------------------------
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec2(const std::string& name, float x, float y) const;
    // ------------------------------------------------------------------------
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    // ------------------------------------------------------------------------
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setVec4(const std::string& name, float x, float y, float z, float w) const;
    // ------------------------------------------------------------------------
    void setMat2(const std::string& name, const glm::mat2& mat) const;
    // ------------------------------------------------------------------------
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    // ------------------------------------------------------------------------
    void setMat4(const std::string& name, const glm::mat4& mat) const;

private:
    // utility function for checking shader compilation/linking errors.
    static void checkCompileErrors(GLuint shader, const std::string& type);

    GLint getUniformLocation(const char* uniformName) const;

    mutable std::unordered_map<size_t, GLint> uniformCache;
};
