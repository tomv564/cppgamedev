
find_package(fmt)
find_package(spdlog)
find_package(docopt)
find_package(gainput)
find_package(miniaudio REQUIRED)
find_package(Freetype REQUIRED)

# Generic test that uses conan libs
add_executable(game WIN32 main.cpp GameApp.cpp UIRenderer.cpp)

# add_subdirectory(gainput)

add_subdirectory(DiligentEngine/DiligentCore)
add_subdirectory(DiligentEngine/DiligentTools)
add_subdirectory(DiligentEngine/DiligentSamples)

#target_include_directories(game PRIVATE "../DiligentEngine/DiligentCore")

target_include_directories(game PRIVATE ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(
  game
  PRIVATE project_options
          project_warnings
          fmt::fmt
          spdlog::spdlog
          gainput::gainput
          miniaudio::miniaudio
          ${FREETYPE_LIBRARIES})
#docopt::docopt
          

# /Zc:__cplusplus is required to make __cplusplus accurate
# /Zc:__cplusplus is available starting with Visual Studio 2017 version 15.7
# (according to https://docs.microsoft.com/en-us/cpp/build/reference/zc-cplusplus)
# That version is equivalent to _MSC_VER==1914
# (according to https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=vs-2019)
# CMake's ${MSVC_VERSION} is equivalent to _MSC_VER
# (according to https://cmake.org/cmake/help/latest/variable/MSVC_VERSION.html#variable:MSVC_VERSION)
if ((MSVC) AND (MSVC_VERSION GREATER_EQUAL 1914))
    target_compile_options(game PUBLIC "/Zc:__cplusplus")
endif()

target_compile_definitions(game PRIVATE ENGINE_DLL=1)
#target_link_libraries(game PRIVATE Diligent-BuildSettings)


get_supported_backends(ENGINE_LIBRARIES)

target_link_libraries(
    game
    PRIVATE
        Diligent-TextureLoader
    PUBLIC
        ${ENGINE_LIBRARIES}
    )

set_target_properties(game PROPERTIES 
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/assets"
)
copy_required_dlls(game)

if(PLATFORM_WIN32 OR PLATFORM_LINUX)
    # Copy assets to target folder
    add_custom_command(TARGET game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "\"$<TARGET_FILE_DIR:game>\"")
endif()
