cmake_minimum_required(VERSION 3.15)
project (AdskEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(dxsdk-d3dx CONFIG REQUIRED)
find_package(assimp REQUIRED)
find_package(Jolt CONFIG REQUIRED)
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/assets")
qt5_add_resources(RESOURCES_CPP "${RESOURCES_DIR}/resources.qrc")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.h"
)

include_directories(
    ${CMAKE_SOURCE_DIR}/src/enum
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/components
    ${CMAKE_SOURCE_DIR}/src/render
    ${CMAKE_SOURCE_DIR}/src/graphics
    ${CMAKE_SOURCE_DIR}/src/ui
    ${CMAKE_SOURCE_DIR}/src/editor
)

add_executable(AdskEngine
    ${SOURCES}
    ${RESOURCES_CPP}
)

target_link_libraries(AdskEngine PRIVATE
    Microsoft::D3DX9
    d3d9.lib
    assimp::assimp
    Jolt::Jolt
    winmm.lib
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)