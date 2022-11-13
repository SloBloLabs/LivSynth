How to convert a CubeMX / CubeIDE project to GNU ARM Toolchain with CMake, openocd and VS Code

Prerequisites

Available on PATH
 - ARM toolchain
 - make
 - cmake
 - openocd

VS Code extensions
 - MS C/C++
 - MS C/C++ Extension Pack
 - Cortex Debug

---

Workflow

Copy to project directory:
 - README.md
 - Makefile
 - CMakeLists.txt
 - cmake folder with arm toolchain cmake directives
 - .gitignore file
 - swvtrace.sh
 - svd file (optional)
 - folder .vscode/launch.json and adapt settings
 - folder "App"

then
 - Adapt settings in CMakeLists.txt header
 - rename myAppMain [.hpp/.cpp]
 - include myAppMain.hpp in Core/Src/main.c
 - add projectMain() to main.c (right before while loop)
 - right click CMakeLists.txt -> configure all projects
 - start coding

Tested on
 - Mac
 - Linux

2022-03-08: There is an issue using ARM Toolchain 11.2.1: https://community.arm.com/support-forums/f/compilers-and-libraries-forum/52377/internal-compiler-errors-with-arm-none-eabi-gcc-11-2-2022-02-on-macs-with-apple-silicon
Stick with 10.3 for now!!
2022-11-13: The issue is solved within Toolchain version 11.3.1

---

Upload to github
 - git init
 - git add .
 - git config user.name "<<username>>"
 - git config user.email "<<email>>"
 - git commit -m "First commit"
 - git branch -M main
 - git remote add origin https://github.com/SloBloLabs/<"ProjectRepo">.git
 - git push -u origin main

--

Update github access token
 - git remote remove origin
 - git remote add origin https://oauth2:<<new oauth token>>@github.com/<<username>>/<<reponame>>.git
 - git remote -v