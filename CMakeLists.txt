cmake_minimum_required(VERSION 3.1)
project(slingBros)

# Set c++11
# https://stackoverflow.com/questions/10851247/how-to-activate-c-11-in-cmake
if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif ()
set (CMAKE_CXX_STANDARD 17)

# nice hierarchichal structure in MSVC
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#Find OS
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(IS_OS_MAC 1)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(IS_OS_LINUX 1)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(IS_OS_WINDOWS 1)
else()
  message(FATAL_ERROR "OS ${CMAKE_SYSTEM_NAME} was not recognized")
endif()

# Create executable target

# You can switch to use the file GLOB for simplicity but at your own risk
file(GLOB SOURCE_FILES src/*.cpp src/*.hpp)

# external libraries will be installed into /usr/local/include and /usr/local/lib but that folder is not automatically included in the search on MACs
if (IS_OS_MAC)
  include_directories(/usr/local/include)
  link_directories(/usr/local/lib)
endif()

add_executable(
        ${PROJECT_NAME} ${SOURCE_FILES}
        "src/entities/slingbro.hpp"
        "src/entities/slingbro.cpp"
        "src/entities/ground_tile.hpp"
        "src/entities/ground_tile.cpp"
        "src/entities/grassy_tile.hpp"
        "src/entities/grassy_tile.cpp"
        "src/entities/windy_grass.hpp"
        "src/entities/windy_grass.cpp"
        "src/entities/lava_tile.hpp"
        "src/entities/lava_tile.cpp"
        "src/entities/start_tile.hpp"
        "src/entities/start_tile.cpp"
        "src/entities/goal_tile.hpp"
        "src/entities/goal_tile.cpp"
        "src/entities/basic_enemy.hpp"
        "src/entities/basic_enemy.cpp"
        "src/entities/projectile.hpp"
        "src/entities/projectile.cpp"
        src/ai/behavioral_tree.hpp
        src/ai/movement.cpp
        src/ai/behavioral_tree.cpp
        src/ai/movement.hpp
        src/ai/attack.cpp
        src/ai/attack.hpp
        "src/entities/powerup.hpp"
        "src/entities/speed_powerup.hpp"
        "src/entities/speed_powerup.cpp"
        "src/entities/snail_enemy.hpp"
        "src/entities/snail_enemy.cpp"
        "src/loader/level_manager.hpp"
        "src/loader/level_manager.cpp"
        "src/entities/button.hpp"
        "src/entities/button.cpp"
        src/ai/pathfinding.cpp
        src/ai/pathfinding.hpp
        "src/entities/parallax_background.hpp"
        "src/entities/parallax_background.cpp"
        "src/entities/size_up_powerup.hpp"
        "src/entities/size_up_powerup.cpp"
        "src/entities/size_down_powerup.hpp"
        "src/entities/size_down_powerup.cpp"
        "src/entities/sand_tile.hpp"
        "src/entities/sand_tile.cpp"
        "src/entities/projectedPath.hpp"
        "src/entities/projectedPath.cpp"
        "src/entities/glass_tile.hpp"
        "src/entities/glass_tile.cpp"
        "src/entities/hazard_tile_spike.hpp"
        "src/entities/hazard_tile_spike.cpp"
        "src/entities/bugdroid_enemy.hpp"
        "src/entities/bugdroid_enemy.cpp"
        "src/entities/bird_enemy.hpp"
        "src/entities/bird_enemy.cpp"
        "src/entities/blueb_enemy.hpp"
        "src/entities/blueb_enemy.cpp"
        "src/entities/dialogue_box.hpp"
        "src/entities/dialogue_box.cpp"
        "src/entities/screen.hpp"
        "src/entities/screen.cpp"
        "src/entities/beehive_enemy.hpp"
        "src/entities/beehive_enemy.cpp"
        "src/menu_system.hpp"
        "src/menu_system.cpp"
        "src/entities/helge_enemy.hpp"
        "src/entities/helge_enemy.cpp"
        "src/entities/helge_projectile.hpp"
        "src/entities/helge_projectile.cpp"
        "src/entities/spike_hazard.hpp"
        "src/entities/spike_hazard.cpp"
        "src/entities/coin_powerup.hpp"
        "src/entities/coin_powerup.cpp"
        "src/entities/snowy_tile.cpp"
        "src/entities/ice_tile.hpp"
        "src/entities/ice_tile.cpp"
        "src/entities/mass_up_powerup.hpp"
        "src/entities/mass_up_powerup.cpp"
        "src/weather.hpp")
target_include_directories(${PROJECT_NAME} PUBLIC src/)

# Added this so policy CMP0065 doesn't scream
set_target_properties(${PROJECT_NAME} PROPERTIES ENABLE_EXPORTS 0)

# External header-only libraries in the ext/
target_include_directories(${PROJECT_NAME} PUBLIC ext/stb_image/)
target_include_directories(${PROJECT_NAME} PUBLIC ext/gl3w/)
target_include_directories(${PROJECT_NAME} PUBLIC ext/entt/)
target_include_directories(${PROJECT_NAME} PUBLIC ext/yaml-cpp/)

# Add and link yaml-cpp to parse .yaml files
add_subdirectory(ext/yaml-cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC yaml-cpp)

# Find OpenGL
find_package(OpenGL REQUIRED)

if (OPENGL_FOUND)
   target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIR})
   target_link_libraries(${PROJECT_NAME} PUBLIC ${OPENGL_gl_LIBRARY})
endif()

set(glm_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glm/cmake/glm) # if necessary
find_package(glm REQUIRED)

# Copy data directory (meshes, audio, textures, etc) to build directory during compilation
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMENT "Copying audio, mesh, shader, font, and texture files from the data/ folder to the build directory..."
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/data"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/data"
)

# glfw, sdl could be precompiled (on windows) or installed by a package manager (on OSX and Linux)
if (IS_OS_LINUX OR IS_OS_MAC)
    # Try to find packages rather than to use the precompiled ones
    # Since we're on OSX or Linux, we can just use pkgconfig.
    find_package(PkgConfig REQUIRED)

    pkg_search_module(GLFW REQUIRED glfw3)
    pkg_search_module(SDL2 REQUIRED sdl2)
    pkg_search_module(SDL2MIXER REQUIRED SDL2_mixer)
    pkg_search_module(FREETYPE REQUIRED freetype2)

    # Link Frameworks on OSX
    if (IS_OS_MAC)
       find_library(COCOA_LIBRARY Cocoa)
       find_library(CF_LIBRARY CoreFoundation)
       target_link_libraries(${PROJECT_NAME} PUBLIC ${COCOA_LIBRARY} ${CF_LIBRARY})
    endif()

    # Increase warning level
    target_compile_options(${PROJECT_NAME} PUBLIC "-Wall")
elseif (IS_OS_WINDOWS)
    # https://stackoverflow.com/questions/17126860/cmake-link-precompiled-library-depending-on-os-and-architecture
    set(GLFW_FOUND TRUE)
    set(SDL2_FOUND TRUE)

    set(GLFW_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/include")
    set(SDL2_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/include/SDL")
    set(FREETYPE_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/ext/freetype/include")

    if (${CMAKE_SIZEOF_VOID_P} MATCHES "8")
        set(GLFW_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/lib/glfw3dll-x64.lib")
        set(SDL2_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2-x64.lib")
        set(SDL2MIXER_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2_mixer-x64.lib")
        set(FREETYPE_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/freetype/lib/freetype-x64.lib")
        set(GLFW_DLL "${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/lib/glfw3-x64.dll")
        set(SDL_DLL "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2-x64.dll")
        set(SDLMIXER_DLL "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2_mixer-x64.dll")
        set(FREETYPE_DLL "${CMAKE_CURRENT_SOURCE_DIR}/ext/freetype/lib/freetype-x64.dll")

    else ()
        set(GLFW_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/lib/glfw3dll-x86.lib")
        set(SDL2_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2-x86.lib")
        set(SDL2MIXER_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2_mixer-x86.lib")
        set(FREETYPE_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/freetype/lib/freetype-x86.lib")

        set(GLFW_DLL "${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/lib/glfw3-x86.dll")
        set(SDL_DLL "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2-x86.dll")
        set(SDLMIXER_DLL "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2_mixer-x86.dll")
        set(FREETYPE_DLL "${CMAKE_CURRENT_SOURCE_DIR}/ext/freetype/lib/freetype-x86.dll")

    endif ()

    # Copy and rename dlls
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GLFW_DLL}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/glfw3.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL_DLL}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/SDL2.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDLMIXER_DLL}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/SDL2_mixer.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FREETYPE_DLL}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/freetype.dll")

    # Make program run in the directory that it lives in (this is not the default VS behaviour for some strange reason)
    set_target_properties(
        ${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

    # Make Visual Studio better-behaved
    target_compile_options(${PROJECT_NAME} PUBLIC
        # increase warning level
        "/W4"

        # Patrol warning "not all control paths return a value" into an error
        "/we4715"

        # use sane exception handling, rather than trying to catch segfaults and allowing resource leaks and UB. Yup... See "Default exception handling behavior" at
        # https://docs.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model?view=vs-2019
        "/EHsc"

        # turn warning C4239 (non-standard extension that allows temporaries to be bound to
        # non-const references, yay microsoft) into an error
        "/we4239"

        # Use UTF-8 encoding for source and executable character set
        /utf-8
    )
endif()

# Can't find the include and lib. Quit.
if (NOT GLFW_FOUND)
    message(FATAL_ERROR "Can't find GLFW." )
elif(NOT FREETYPE_FOUND)
    message(FATAL_ERROR "Can't find FreeType.")
elif(NOT SDL2_FOUND)
    message(FATAL_ERROR "Can't find SDL." )
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${GLFW_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${FREETYPE_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
    PUBLIC ${GLFW_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${SDL2MIXER_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    glm::glm
)

# Needed to add this
if(IS_OS_LINUX)
  target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_DL_LIBS})
endif()
