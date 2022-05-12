# uncomment to enable the gui examples

# sdl
# add_subdirectory(sdl)

# qt
# add_subdirectory(qt)

# fltk test
# add_subdirectory(fltk)

# gtkmm test
# add_subdirectory(gtkmm)

# imgui example
# add_subdirectory(imgui)

# Nana
# add_subdirectory(nana)

find_package(fmt)
find_package(spdlog)
find_package(docopt)

# Generic test that uses conan libs
add_executable(intro main.cpp)

add_subdirectory(DiligentEngine/DiligentCore)

#target_include_directories(intro PRIVATE "../DiligentEngine/DiligentCore")
target_link_libraries(
  intro
  PRIVATE project_options
          project_warnings
          docopt::docopt
          fmt::fmt
          spdlog::spdlog)

target_compile_definitions(intro PRIVATE ENGINE_DLL=1)
#target_link_libraries(intro PRIVATE Diligent-BuildSettings)

#if(D3D12_SUPPORTED)
target_link_libraries(intro PRIVATE Diligent-GraphicsEngineD3D12-shared)
#endif()

copy_required_dlls(intro)


