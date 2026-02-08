cmake_minimum_required(VERSION 3.14)
project(VK_Android LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SDL_PATH "${CMAKE_SOURCE_DIR}/../../../../libraries/SDL3-3.4.0")

if(NOT EXISTS ${SDL_PATH})
    message(FATAL_ERROR "SDL3 not found: ${SDL_PATH}")
endif()

message(STATUS "SDL3 found: ${SDL_PATH}")

set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)

add_subdirectory(${SDL_PATH} SDL-build)

# Налаштування шейдерів
set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")

# ВИПРАВЛЕНО: Правильний шлях до assets в Android проекті
# Було: ${CMAKE_SOURCE_DIR}/../../assets
# Стало: ${CMAKE_SOURCE_DIR}/../../main/assets
set(ANDROID_ASSETS_DIR "${CMAKE_SOURCE_DIR}/../../main/assets")

message(STATUS "Looking for shaders in: ${SHADER_DIR}")
message(STATUS "Shaders will be output to: ${SHADER_OUTPUT_DIR}")
message(STATUS "Assets directory: ${ANDROID_ASSETS_DIR}")

file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

if(NOT EXISTS "${SHADER_DIR}/shader.vert")
    message(FATAL_ERROR "shader.vert not found at: ${SHADER_DIR}/shader.vert")
endif()

if(NOT EXISTS "${SHADER_DIR}/shader.frag")
    message(FATAL_ERROR "shader.frag not found at: ${SHADER_DIR}/shader.frag")
endif()

# Шукаємо glslc компілятор
find_program(GLSLC glslc HINTS
        $ENV{VULKAN_SDK}/bin
        $ENV{ANDROID_NDK}/shader-tools/linux-x86_64
        $ENV{ANDROID_NDK}/shader-tools/darwin-x86_64
        /usr/bin
        /usr/local/bin
)

if(NOT GLSLC)
    message(WARNING "glslc not found! Shaders will not be compiled.")
else()
    message(STATUS "glslc found: ${GLSLC}")

    # Компілюємо шейдери
    add_custom_command(
            OUTPUT ${SHADER_OUTPUT_DIR}/vert.spv
            COMMAND ${GLSLC} ${SHADER_DIR}/shader.vert -o ${SHADER_OUTPUT_DIR}/vert.spv
            DEPENDS ${SHADER_DIR}/shader.vert
            COMMENT "Compiling vertex shader"
    )

    add_custom_command(
            OUTPUT ${SHADER_OUTPUT_DIR}/frag.spv
            COMMAND ${GLSLC} ${SHADER_DIR}/shader.frag -o ${SHADER_OUTPUT_DIR}/frag.spv
            DEPENDS ${SHADER_DIR}/shader.frag
            COMMENT "Compiling fragment shader"
    )

    # Створюємо target для компіляції шейдерів
    add_custom_target(compile_shaders ALL
            DEPENDS ${SHADER_OUTPUT_DIR}/vert.spv ${SHADER_OUTPUT_DIR}/frag.spv
    )
endif()

# Створюємо бібліотеку
add_library(main SHARED main.cpp)

target_include_directories(main PRIVATE ${SDL_PATH}/include)

find_library(VULKAN_LIB vulkan REQUIRED)
find_library(LOG_LIB log REQUIRED)
find_library(ANDROID_LIB android REQUIRED)

target_link_libraries(main PRIVATE
        SDL3::SDL3
        ${VULKAN_LIB}
        ${LOG_LIB}
        ${ANDROID_LIB}
)

# Додаємо залежність від компіляції шейдерів
if(GLSLC)
    add_dependencies(main compile_shaders)

    # Копіюємо скомпільовані шейдери в ПРАВИЛЬНУ assets директорію
    add_custom_command(TARGET main POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${ANDROID_ASSETS_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SHADER_OUTPUT_DIR}/vert.spv ${ANDROID_ASSETS_DIR}/vert.spv
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SHADER_OUTPUT_DIR}/frag.spv ${ANDROID_ASSETS_DIR}/frag.spv
            COMMAND ${CMAKE_COMMAND} -E echo "Shaders copied to: ${ANDROID_ASSETS_DIR}"
            COMMENT "Copying compiled shaders to Android assets directory"
            VERBATIM
    )
endif()