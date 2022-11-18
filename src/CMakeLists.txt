
find_package(fmt)
find_package(spdlog)
find_package(docopt)

# Generic test that uses conan libs
add_executable(game WIN32 main.cpp GameApp.cpp)

add_subdirectory(DiligentEngine/DiligentCore)
add_subdirectory(DiligentEngine/DiligentTools)
add_subdirectory(DiligentEngine/DiligentSamples)

#target_include_directories(game PRIVATE "../DiligentEngine/DiligentCore")
target_link_libraries(
  game
  PRIVATE project_options
          project_warnings
          fmt::fmt
          spdlog::spdlog)
#docopt::docopt
          

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
