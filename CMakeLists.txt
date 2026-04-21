cmake_minimum_required(VERSION 3.10)
project(SkiaGUIExample)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Skia was built with static runtime (/MT), so we must match it
if(MSVC)
    add_compile_options(
        $<$<CONFIG:Release>:/MT>
        $<$<CONFIG:Debug>:/MTd>
    )
endif()

# Skia Include Directories
include_directories(
    .
    include
    External/skia/include
    External/skia/include/include
)

# Source files
set(SOURCES
    src/main.cpp
    src/gui/MochiUI.cpp
    src/gui/MenuBar.cpp
    src/utils/FontManager/FontMgr.cpp
)

add_executable(SkiaGUIExample WIN32 ${SOURCES})

# Skia Libraries
set(SKIA_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/External/skia/lib)

target_link_libraries(SkiaGUIExample
    PRIVATE
    ${SKIA_LIB_DIR}/skia.lib
    ${SKIA_LIB_DIR}/freetype.lib
    ${SKIA_LIB_DIR}/libpng.lib
    ${SKIA_LIB_DIR}/zlib.lib
    # Windows System Libraries
    User32.lib
    Gdi32.lib
    Ole32.lib
    Shell32.lib
    Dwrite.lib
    Usp10.lib
    Dwmapi.lib
    Shlwapi.lib
    ComCtl32.lib
)

# Preprocessor definitions often required for Skia on Windows
target_compile_definitions(SkiaGUIExample PRIVATE
    SK_GAMMA_APPLY_TO_A8
    SK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)
