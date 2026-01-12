message(STATUS "Configuring ImGUI...")

add_library(imgui_lib STATIC
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_impl_glfw.cpp
    imgui/imgui_impl_opengl3.cpp
)

target_include_directories(imgui_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui
)

target_link_libraries(imgui_lib
    PUBLIC
        glfw
        OpenGL::GL
        glm
        glew
        GOLLoggingLib
)

target_compile_definitions(imgui_lib PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)

# Add /FS flag for MSVC to avoid PDB file access issues during parallel builds
if(MSVC)
    target_compile_options(imgui_lib PRIVATE /FS)
endif()

message(STATUS "ImGUI configured successfully")