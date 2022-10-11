
### TODO

Immediate-mode text
Fix renderdoc / graphics tools compat?

blend texture with color
deal with texture not being used?


### Quad rendering & Shader cleanup

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

git clone git@github.com:DiligentGraphics/DiligentEngine.git
cd DiligentEngine
git checkout v2.5.2
git submodule update --init

TODO:

Following the tutorial in 


Fixed issues:
* proper recursive clone
* add_subdirectory to get DiligentCore includes and libs into project (+adjusted includes) 



### Steps to "hello world" window on Windows

Cloned cpp-best-practices

Followed:
https://docs.microsoft.com/en-us/windows/win32/learnwin32/your-first-windows-program


