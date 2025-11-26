cmake_minimum_required(VERSION 4.0)
project(VirtualGeometry)

set(CMAKE_CXX_STANDARD 23)

# Include FetchContent module
include(FetchContent)

# If Vulkan SDK not found, fetch Vulkan headers and loader
if(NOT Vulkan_FOUND)
    message(STATUS "Vulkan SDK not found. Fetching Vulkan headers and loader...")

    # Fetch Vulkan Headers
    FetchContent_Declare(
            VulkanHeaders
            GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
            GIT_TAG        v1.4.328
    )

    # Fetch Vulkan Loader
    FetchContent_Declare(
            VulkanLoader
            GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Loader.git
            GIT_TAG        v1.4.328
    )

    # Make Vulkan Headers available first
    FetchContent_MakeAvailable(VulkanHeaders)

    # Configure and build Vulkan Loader
    set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(BUILD_STATIC_LOADER OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(VulkanLoader)

    # Set Vulkan variables manually
    set(Vulkan_INCLUDE_DIRS ${vulkanheaders_SOURCE_DIR}/include)
    set(Vulkan_LIBRARIES vulkan)
    set(Vulkan_FOUND TRUE)
endif()

# Fetch GLFW
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Don't build GLFW docs")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Don't build GLFW tests")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Don't build GLFW examples")

# Fetch GLM
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        1.0.2
)

# VMA
FetchContent_Declare(
        VMA
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
        GIT_TAG        v3.3.0
)

# ImGui
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        v1.92.3
)

# fmt
FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG        12.0.0
)

# spdlog
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG        v1.15.3
)

# Tracy
FetchContent_Declare(
        tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG        v0.12.2
)

# Configure Tracy options before making it available
set(TRACY_ENABLE ON CACHE BOOL "" FORCE)

# Fetch Tracy
FetchContent_MakeAvailable(tracy)

# Make the rest available
FetchContent_MakeAvailable(glfw glm VMA imgui fmt spdlog)

# Create ImGui library target (since ImGui doesn't provide CMakeLists)
add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(imgui PRIVATE glfw ${Vulkan_LIBRARIES})

# Automatically collect all .cpp files under src/
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/src/*.cpp
)

add_executable(${CMAKE_PROJECT_NAME})
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${SOURCES})

# Set MSVC optimization flags
if(MSVC)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:/O2>  # Maximum optimization for Release builds
            $<$<CONFIG:Debug>:/Od>    # Disable optimization for Debug builds
            $<$<CONFIG:RelWithDebInfo>:/O2>       # Maximum optimization for RelWithDebInfo builds
            /permissive-              # Disable permissive mode for stricter conformance
    )
endif()

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        VULKAN_HPP_ENABLE_STRINGIZE
        GLM_ENABLE_EXPERIMENTAL
)

# Include directories
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${Vulkan_INCLUDE_DIRS}
        ${vma_SOURCE_DIR}/include
        ${glm_SOURCE_DIR}
        ${tracy_SOURCE_DIR}/public
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE glfw ${Vulkan_LIBRARIES} imgui fmt::fmt spdlog::spdlog TracyClient dwmapi)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE glfw ${Vulkan_LIBRARIES} imgui fmt::fmt spdlog::spdlog TracyClient)
elseif(APPLE)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE glfw ${Vulkan_LIBRARIES} imgui fmt::fmt spdlog::spdlog TracyClient)
endif()
