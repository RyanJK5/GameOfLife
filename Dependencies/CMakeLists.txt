# OpenGL is now found in root CMakeLists.txt

# Header-only libraries
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glm)

add_library(unordered_dense INTERFACE)
target_include_directories(unordered_dense INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/unordered_dense)

add_library(fontawesome INTERFACE)
target_include_directories(fontawesome INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/FontAwesome)

message(STATUS "Header-only dependencies configured")

# GLEW (static) - Support all configurations
add_library(glew STATIC IMPORTED GLOBAL)
set_target_properties(glew PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/GLEW/glew-2.1.0/lib/Release/x64/glew32s.lib"
    IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/GLEW/glew-2.1.0/lib/Release/x64/glew32s.lib"
    IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/GLEW/glew-2.1.0/lib/Release/x64/glew32s.lib"
    IMPORTED_LOCATION_MINSIZEREL "${CMAKE_CURRENT_SOURCE_DIR}/GLEW/glew-2.1.0/lib/Release/x64/glew32s.lib"
    IMPORTED_LOCATION_RELWITHDEBINFO "${CMAKE_CURRENT_SOURCE_DIR}/GLEW/glew-2.1.0/lib/Release/x64/glew32s.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/GLEW/glew-2.1.0/include"
    INTERFACE_COMPILE_DEFINITIONS "GLEW_STATIC"
)
message(STATUS "GLEW configured: ${CMAKE_CURRENT_SOURCE_DIR}/GLEW/glew-2.1.0/include")

# GLFW - Support all configurations (rename to glfw3_lib to avoid conflicts)
# Detect platform architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(GLFW_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/GLFW/lib-vc2022/glfw3.lib")
    message(STATUS "GLFW: Using x64 library")
else()
    set(GLFW_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/GLFW/x86-lib-vc2022/glfw3.lib")
    message(STATUS "GLFW: Using x86 library")
endif()

add_library(glfw3_lib STATIC IMPORTED GLOBAL)
set_target_properties(glfw3_lib PROPERTIES
    IMPORTED_LOCATION "${GLFW_LIB_PATH}"
    IMPORTED_LOCATION_DEBUG "${GLFW_LIB_PATH}"
    IMPORTED_LOCATION_RELEASE "${GLFW_LIB_PATH}"
    IMPORTED_LOCATION_MINSIZEREL "${GLFW_LIB_PATH}"
    IMPORTED_LOCATION_RELWITHDEBINFO "${GLFW_LIB_PATH}"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/GLFW/include"
    INTERFACE_LINK_LIBRARIES "opengl32;ole32;gdi32;user32;kernel32;shell32"
)
add_library(glfw ALIAS glfw3_lib)
message(STATUS "GLFW configured: ${CMAKE_CURRENT_SOURCE_DIR}/GLFW/include")

# NFD - Support all configurations
# Detect platform architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(NFD_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/NFD/lib/nfd64.lib")
    message(STATUS "NFD: Using x64 library")
else()
    set(NFD_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/NFD/lib/nfd.lib")
    message(STATUS "NFD: Using x86 library")
endif()

add_library(nfd STATIC IMPORTED GLOBAL)
set_target_properties(nfd PROPERTIES
    IMPORTED_LOCATION "${NFD_LIB_PATH}"
    IMPORTED_LOCATION_DEBUG "${NFD_LIB_PATH}"
    IMPORTED_LOCATION_RELEASE "${NFD_LIB_PATH}"
    IMPORTED_LOCATION_MINSIZEREL "${NFD_LIB_PATH}"
    IMPORTED_LOCATION_RELWITHDEBINFO "${NFD_LIB_PATH}"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/NFD/include"
    INTERFACE_LINK_LIBRARIES "comctl32"
)
message(STATUS "NFD configured: ${CMAKE_CURRENT_SOURCE_DIR}/NFD/include")

add_subdirectory(ImGUI)