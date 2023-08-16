add_rules("mode.debug", "mode.release")

set_targetdir("bin")
set_languages("c++20")

-- add_defines('PROJECT_ROOT_DIR="$(projectdir)"')


add_requires("glfw","glm","stb","glad")
add_requires("gtest","fmt")
add_requires("libsdl" ,"libsdl_mixer")
add_requires("freetype")

add_packages("glfw","glm","stb","glad","fmt")
add_packages("libsdl" ,"libsdl_mixer")

add_includedirs("./pkgs/")


local audio_backend = "sdl";
if(audio_backend == "sdl") then
	add_files("pkgs/audio/sdl_wrapper/**.cc")
end


includes("./pkgs/m_log")
add_includedirs("./pkgs/m_log")
-- Add this to top_level xmake if msvc
if is_os("windows") then
	add_cxxflags("/Zc:preprocessor")
end

target("breakout")
    set_kind("binary")
    add_files("src/**.cpp")
	add_includedirs("include")

	add_packages("glfw","glm","stb","glad")
	add_packages("fmt")
	-- add_packages("miniaudio")
	add_packages("freetype")

	add_deps("m_log")


target("test")
    set_kind("binary")
    add_files("test/*.cpp")
    add_files("test/*.cc")
	add_includedirs("src","include")

	add_defines("DISABLE_TEST_CASE")

	add_packages("glfw","glm","stb")
	add_packages("glad")
	add_packages("gtest")
	add_packages("libsdl" ,"libsdl_mixer")

	add_deps("m_log")


--> Add the Audio Library 
--add_requires("openal-soft")
-- add_requires("miniaudio")
-- add_packages("miniaudio")
-- if(is_os("windows")) then
-- 	add_links("mmdevapi")
-- end
-- add_files("pkgs/miniaudio_wrapper/**.cc")

-- target("manual_test_audio")
--     set_kind("binary")
--     add_files("test/manual/with_deps/audio_test.cc")

-- target("manual_test_miniaudio_playback")
--     set_kind("binary")
--     add_files("test/manual/with_deps/miniaudio_playback_test.cc")

target("manual_test_sdl_play_wave")
    set_kind("binary")
    add_files("test/manual/with_deps/sdl_play_wave_test.cc")
	add_packages("libsdl" ,"libsdl_mixer")
	add_deps("m_log")

