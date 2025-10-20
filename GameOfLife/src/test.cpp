#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include "Logging.h"
#include "ShaderManager.h"
#include "vendor/imgui.h"
#include "vendor/imgui_impl_glfw.h"
#include "vendor/imgui_impl_opengl3.h"
#include <print>

int main()
{
    if (!glfwInit())
        return -1;

    float windowWidth = 1920;
    float windowHeight = 1080;
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Conway's Game of Life", NULL, NULL);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewInit();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    //gol::ShaderManager m_Shader(std::filesystem::path("resources") / "shader" / "default.shader");
    //GL_DEBUG(glUseProgram(m_Shader.Program()));

    uint32_t m_FrameBuffer, m_Texture, m_renderBuffer;
    GL_DEBUG(glGenFramebuffers(1, &m_FrameBuffer));
    GL_DEBUG(glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer));

    GL_DEBUG(glGenTextures(1, &m_Texture));
    GL_DEBUG(glBindTexture(GL_TEXTURE_2D, m_Texture));
    GL_DEBUG(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));
    GL_DEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_DEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_DEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture, 0));

    GL_DEBUG(glGenRenderbuffers(1, &m_renderBuffer));
    GL_DEBUG(glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer));
    GL_DEBUG(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight));
    GL_DEBUG(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderBuffer));

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return -1;

    GL_DEBUG(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    GL_DEBUG(glBindTexture(GL_TEXTURE_2D, 0));
    GL_DEBUG(glBindRenderbuffer(GL_RENDERBUFFER, 0));

    float positions[] =
    {
        windowWidth / 4, windowHeight / 4,
        windowWidth * 3 / 4, windowHeight / 4,
        windowWidth * 3 / 4, windowHeight * 3 / 4,
        windowWidth / 4, windowHeight * 3 / 4
    };

    uint8_t indices[] =
    {
        0, 1,
        1, 2,
        2, 3,
        3, 0
    };

    uint32_t buffer, indexBuffer;

    GL_DEBUG(glGenBuffers(1, &buffer));
    GL_DEBUG(glGenBuffers(1, &indexBuffer));

    GL_DEBUG(glOrtho(0, windowWidth, windowHeight, 0, 1, -1));
    while (!glfwWindowShouldClose(window))
    {
        GL_DEBUG(glfwPollEvents());
        GL_DEBUG(glClear(GL_COLOR_BUFFER_BIT));
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        GL_DEBUG(glClearColor(0, 0, 0, 1));
        GL_DEBUG(glClear(GL_COLOR_BUFFER_BIT));
        GL_DEBUG(glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer));

        GL_DEBUG(glBindBuffer(GL_ARRAY_BUFFER, buffer));
        GL_DEBUG(glBufferData(GL_ARRAY_BUFFER, 2 * 4 * sizeof(float), positions, GL_DYNAMIC_DRAW));
        GL_DEBUG(glEnableVertexAttribArray(0));
        GL_DEBUG(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));

        GL_DEBUG(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer));
        GL_DEBUG(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * 4 * sizeof(uint8_t), indices, GL_DYNAMIC_DRAW));

        GL_DEBUG(glDrawElements(GL_LINES, 8, GL_UNSIGNED_BYTE, nullptr));
        GL_DEBUG(glBindFramebuffer(GL_FRAMEBUFFER, 0));

        std::println("Test");
        ImGui::Begin("Simulation");
        {
            ImGui::BeginChild("GameRender");

            ImGui::Image(
                (ImTextureID)m_Texture,
                ImGui::GetContentRegionAvail(),
                ImVec2(0, 1),
                ImVec2(1, 0)
            );
        }
        ImGui::EndChild();
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        GL_DEBUG(glfwSwapBuffers(window));
    }
}