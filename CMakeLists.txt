cmake_minimum_required(VERSION 3.10)
project(MochiUI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Match Skia's Official Build Runtime and Architecture
if(MSVC)
    add_compile_options(
        $<$<CONFIG:Release>:/MT>
        $<$<CONFIG:Debug>:/MTd>
        /arch:AVX2  
    )
endif()

include_directories(
    .
    include
    External/skia/include
)

set(SKIA_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/External/skia/lib)

# MochiKit Static Library Sources
set(MOCHIKIT_SOURCES
    src/gui/MochiUI.cpp
    src/gui/MenuBar.cpp
    src/gui/Components/TextNode.cpp
    src/gui/Components/CheckboxNode.cpp
    src/gui/Components/SliderNode.cpp
    src/gui/Components/KnobNode.cpp
    src/gui/Components/VUMeterNode.cpp
    src/gui/Components/ScrollAreaNode.cpp
    src/utils/FontManager/FontMgr.cpp
    src/utils/Misc/ThemeSwitcher.cpp
)

# Create MochiKit static library
add_library(MochiKit STATIC ${MOCHIKIT_SOURCES})

# MochiKit compile definitions
target_compile_definitions(MochiKit PUBLIC
    SKIA_STATIC
    SK_RELEASE
    SK_D3D
    SK_DIRECTWRITE
    SK_DISABLE_LEGACY_PATH_METHODS
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

# MochiKit link libraries
target_link_libraries(MochiKit
    PUBLIC
    ${SKIA_LIB_DIR}/skia.lib
    ${SKIA_LIB_DIR}/libpng.lib
    ${SKIA_LIB_DIR}/zlib.lib
    d3d12.lib
    dxgi.lib
    d3dcompiler.lib
    Dwrite.lib
    User32.lib
    Gdi32.lib
    Ole32.lib
    Shell32.lib
    Usp10.lib
    Dwmapi.lib
    Shlwapi.lib
    ComCtl32.lib
    Imm32.lib
)

# MochiExample executable
add_executable(MochiExample WIN32 example/main.cpp)

# Link MochiExample with MochiKit
target_link_libraries(MochiExample
    PRIVATE
    MochiKit
)