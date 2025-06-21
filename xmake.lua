add_rules("mode.debug", "mode.release")

set_warnings("all")
set_optimize("fastest")
set_symbols("debug")

-- Ensure glew is present here
add_requires("glfw", "opengl", "glm", "glew")

add_includedirs("include", "external/imgui", "external/imgui/backends")

add_files("src/*.cpp")
add_files("external/imgui/*.cpp") 
add_files("external/imgui/backends/imgui_impl_glfw.cpp")
add_files("external/imgui/backends/imgui_impl_opengl3.cpp")

target("Physics_Engine")
    set_kind("binary")
    set_languages("c++20")
    set_targetdir("bin")

    add_files("src/*.cpp", "glad-generated/src/glad.c", "external/imgui/*.cpp")

    add_includedirs("include", "external/imgui", "glad-generated/include") 

    -- Ensure glew is present here
    add_packages("opengl", "glfw", "glm", "glew")

    set_optimize("fastest")
    add_ldflags("-flto")
