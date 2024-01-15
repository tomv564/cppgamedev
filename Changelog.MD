
### commands

Configure

cmake -S . -B ./build




### compile commmands


ninja build?
cmake -S . -GNinja -B ./ninjabuild

CMake Error at C:/Users/Tom/.conan/data/freetype/2.13.0/_/_/build/a43c6e0e36517840f45af1fe54e76eac265bab11/build/Debug/generators/cmakedeps_macros.cmake:39 (message):
  Library 'png16d' not found in package.  If 'png16d' is a system library,
  declare it with 'cpp_info.system_libs' property

  

### TODO


More UI features:
https://github.com/nxp-imx/gtec-demo-framework/blob/master/Doc/FslSimpleUI.md

Make a dialog
Layouting


spdlog to debug output
console output??

Make a button


Add Sol
Expose Input state
Hook up CreateSurface / UpdateSurface ?


### Done



### Dec 22


Moved input, audio to GameApp
Added lua
Easy to add but FindLua registered names as lua_INCLUDE_DIRS instead of LUA_INCLUDE_DIRS 
And Cmake loves to ignore this ??


### Dec ???



Extracted UIRenderer
Text rendering using freetype
Using clip instead of blending for now
Fixed color bug in toFloat4
Fix order of rendering (apparently need to issue draw calls front to back??)


### Earlier

Input: https://gainput.johanneskuhlmann.de/



### Fix CI

DiligentEngine not checked out

### Fix renderdoc / graphics tools

try running with -mode d3d11 or -mode gl/vk/d3d12
C:\Users\Tom\Projects\cppgamedev\build\src\DiligentEngine\DiligentSamples\Tutorials\Tutorial01_HelloTriangle\RelWithDebInfo\Tutorial01_HelloTriangle.exe
works!
HelloCube fails with d3d12, works with d3d11 and gl! This must be the difference??
https://www.reddit.com/r/opengl/comments/ws20gv/exe_runs_normally_but_crashes_with_render_doc/



### Drawing per surface


Static buffers 
dynamic draw 


Per-surface textures
blend texture with color
deal with texture not being used?




## DONE


### Support more rendering backends



### Quad rendering & Shader cleanup


Strings on Surface instead of chars
Immediate-mode text
Extract shaders
Immediate-mode rectangle
With colors


### Tutorial refactor


Extracted app class


### Fix compile_commands

install mingw-get and install  
edit .conan/profiles/default to hardwire CC to clang as it fails to autodetect
cmake -G "MinGW Makefiles" -S . -B mingw4 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
copy compile_commands.json from mingw4 to build


### Add DiligentEngine

https://dev.to/funatsufumiya/comparison-of-c-low-level-graphics-cross-platform-frameworks-and-libraries-58e5

cd src
git clone git@github.com:DiligentGraphics/DiligentEngine.git
cd DiligentEngine
git checkout v2.5.2
git submodule update --init

cd src && git clone --branch v2.5.2 --depth 1 --recurse-submodules git@github.com:DiligentGraphics/DiligentEngine.git
TODO:

Following the tutorial in 


Fixed issues:
* proper recursive clone
* add_subdirectory to get DiligentCore includes and libs into project (+adjusted includes) 



### Steps to "hello world" window on Windows

Cloned cpp-best-practices

Followed:
https://docs.microsoft.com/en-us/windows/win32/learnwin32/your-first-windows-program


