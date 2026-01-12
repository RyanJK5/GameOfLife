file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

message(STATUS "GOLGuiLib sources found: ${SOURCES}")

add_library(GOLGuiLib STATIC ${SOURCES} ${HEADERS})

target_include_directories(GOLGuiLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs
        ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets
        ${CMAKE_CURRENT_SOURCE_DIR}/src/misc
)

target_link_libraries(GOLGuiLib
    PUBLIC
        imgui_lib
        nfd
        fontawesome
        GOLAlgoLib
        GOLGraphicsLib
        GOLLoggingLib
)

# Add /FS flag for MSVC to avoid PDB file access issues during parallel builds
if(MSVC)
    target_compile_options(GOLGuiLib PRIVATE /FS)
endif()

message(STATUS "GOLGuiLib configured successfully")