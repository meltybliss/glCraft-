cmake_minimum_required(VERSION 3.24)

project(glCraftPP VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(GLCRAFT_FETCH_GLFW "Download and build GLFW when no installed package is found" ON)

find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 QUIET CONFIG)

if(TARGET glfw)
    set(GLCRAFT_GLFW_TARGET glfw)
elseif(TARGET glfw3::glfw)
    set(GLCRAFT_GLFW_TARGET glfw3::glfw)
elseif(TARGET glfw::glfw)
    set(GLCRAFT_GLFW_TARGET glfw::glfw)
elseif(GLCRAFT_FETCH_GLFW)
    include(FetchContent)

    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(glfw)
    set(GLCRAFT_GLFW_TARGET glfw)
else()
    message(FATAL_ERROR
        "GLFW was not found. Install GLFW or configure with -DGLCRAFT_FETCH_GLFW=ON."
    )
endif()

file(GLOB_RECURSE GLCRAFT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

set(GLCRAFT_IMGUI_SOURCES
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_impl_glfw.cpp
    third_party/imgui/imgui_impl_opengl3.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp
)

add_executable(glcraft
    ${GLCRAFT_SOURCES}
    ${GLCRAFT_IMGUI_SOURCES}
    third_party/glad.c
)

set_target_properties(glcraft PROPERTIES
    OUTPUT_NAME "glCraft++"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_include_directories(glcraft PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui"
)

target_compile_definitions(glcraft PRIVATE GLFW_INCLUDE_NONE)

target_link_libraries(glcraft PRIVATE
    ${GLCRAFT_GLFW_TARGET}
    OpenGL::GL
    Threads::Threads
)

if(MSVC)
    target_compile_options(glcraft PRIVATE /W3 /utf-8)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(glcraft PRIVATE -Wall -Wextra)
endif()

if(MINGW)
    set(GLCRAFT_MINGW_RUNTIME_FILES)
    get_filename_component(GLCRAFT_MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)

    foreach(runtime_dll IN ITEMS
        libgcc_s_seh-1.dll
        libstdc++-6.dll
        libwinpthread-1.dll
    )
        set(runtime_path "${GLCRAFT_MINGW_BIN_DIR}/${runtime_dll}")

        if(NOT EXISTS "${runtime_path}")
            execute_process(
                COMMAND "${CMAKE_CXX_COMPILER}" "-print-file-name=${runtime_dll}"
                OUTPUT_VARIABLE runtime_path
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
        endif()

        if(EXISTS "${runtime_path}")
            list(APPEND GLCRAFT_MINGW_RUNTIME_FILES "${runtime_path}")
            add_custom_command(TARGET glcraft POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    "${runtime_path}"
                    "$<TARGET_FILE_DIR:glcraft>"
                COMMENT "Copying ${runtime_dll}"
            )
        endif()
    endforeach()
endif()

add_custom_target(glcraft_assets ALL
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:glcraft>/assets"
    COMMENT "Copying runtime assets"
)

install(TARGETS glcraft RUNTIME DESTINATION .)
install(DIRECTORY assets DESTINATION .)
if(GLCRAFT_MINGW_RUNTIME_FILES)
    install(FILES ${GLCRAFT_MINGW_RUNTIME_FILES} DESTINATION .)
endif()
