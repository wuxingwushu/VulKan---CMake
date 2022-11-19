cmake_minimum_required(VERSION 3.12)
project(VulKan_CMake)

set(CMAKE_CXX_STANDARD 17)

aux_source_directory(. DIRSRCS)

include_directories(
SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}./Environment/GLFW/include
SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}./Environment/VulKan/Include
)

link_directories(
SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}./Environment/GLFW/lib
SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}./Environment/VulKan/Lib
)

add_subdirectory(VulKanWrapper) # 对子文件夹进行CMake
add_subdirectory(texture)

add_executable(VulKan_CMake  ${DIRSRCS})

target_link_libraries(VulKan_CMake vulkanLib textureLib vulkan-1.lib glfw3.lib)