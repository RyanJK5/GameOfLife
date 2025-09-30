#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>
#include <optional>

using IDPair = typename std::pair<unsigned int, unsigned int>;

namespace gol
{
    class ShaderManager
    {

    public:
        explicit ShaderManager(const std::string& shaderFilePath);

        ShaderManager(const ShaderManager& other) = delete;

        ShaderManager(ShaderManager&& other) noexcept;

        ShaderManager& operator=(const ShaderManager& other) = delete;

        ShaderManager& operator=(ShaderManager&& other) noexcept;

        ~ShaderManager();

        uint32_t Program() const;
    private:
        uint32_t CompileShader(uint32_t type, const std::string& source) const;

        std::optional<IDPair> ParseShader(const std::string& filePath) const;

        void CreateShader(uint32_t program, uint32_t shaderId);
    private:
        unsigned int m_programID;
    };
}