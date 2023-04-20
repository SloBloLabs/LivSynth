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

--

Bake a new release on Github:
git tag <tagname> // e.g. v0.2.1
git push origin <tagname>

--

# Build Instructions

Use the ibom tool inside the hardware section of the repository to track your work!
## Power
 - Place USB Micro Connector J21
 - Place all components from power section
   - U8
   - D12
   - FB1
   - U1
   - J24, J24, J12, J7
   - C49, C52, C32, C33, C1, C2, C3, C7
   - R1, R2, R11
   - SW7
 - Check voltages at J201, do proceed if everything is ok!

## Assembly of backside
 - assemble all components (except 5mm LEDs!) on the backside of the mainboard, preferably using the following order:
   - SMD components
   - resistors
   - diodes
   - DIP socket and DIP components
   - transistors (Q4/Q5, Q8/Q9 and Q11/Q12 must be matched)
   - tempco resistor
   - pin headers
   - capacitors

## Assemply of frontside
 - assemble potentiometers, trimmers, spacers and switches of the mainboard, DO NOT SOLDER THEM YET!
 - put the faceplate on top and make sure all components incl. trimmers fit into the wholes
 - fasten the M2.5 screws in the corners
 - turn the set around and solder everthing
 - remove the faceplate again

## Assmebly of speaker
 - First put screws from backside, hex nuts on the front side and tighten them
 - solder the hex nuts to the solder pads
 - apply two pieces of bell wire of about 5cm in lenght to the speakers solder pads
 - remove screws and mount the speaker to the frontside using the screws the other way around
 - if using the battery holder you may need to shorten one of the screws to make it fit
 - connect the loose ends of speaker wires to J4

## LEDs
 - assemble the 5mm LEDs on the backside
 - bend their heads in a 180° U-turn into the holes behind the transparent potentiometers, so they can emit as much light as possible into their shafts

## Test & Calibration
 - connect a 9V center negative power supply to the barrel jack
 - switch on the device and wait a moment. Turn it off again if you see anything suspicious. ("smoke test")
 - At least the keyboard LEDs should light up in white colour and the LFO LEDs are blinking
 - 

