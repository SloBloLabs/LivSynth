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

## Flashing the firmware
 - make sure, dfu-util (https://dfu-util.sourceforge.net) is installed on your machine
   - alternatively, you can use stm32 cube programmer (https://www.st.com/en/development-tools/stm32cubeprog.html). I will not further explain this way, but it is a little more user friendly and pretty straight forward.
 - switch off the synthesizer
 - pull the run/program toggle switch next to the micro USB port to lower (program) position
 - connect a USB cable between the synth and your computer
 - switch on the synth again
 - execute the following command:
```
dfu-util -a 0 -s 0x08000000 -D <path to LivSynth.bin>
```
 - The output in the console shoud look similar to this:
```
dfu-util -a 0 -s 0x08000000 -D build/LivSynth.bin
dfu-util 0.11

Copyright 2005-2009 Weston Schmidt, Harald Welte and OpenMoko Inc.
Copyright 2010-2021 Tormod Volden and Stefan Schmidt
This program is Free Software and has ABSOLUTELY NO WARRANTY
Please report bugs to http://sourceforge.net/p/dfu-util/tickets/

dfu-util: Warning: Invalid DFU suffix signature
dfu-util: A valid DFU suffix will be required in a future dfu-util release
Opening DFU capable USB device...
Device ID 0483:df11
Device DFU version 011a
Claiming USB DFU Interface...
Setting Alternate Interface #0 ...
Determining device status...
DFU state(10) = dfuERROR, status(10) = Device's firmware is corrupt. It cannot return to run-time (non-DFU) operations
Clearing status
Determining device status...
DFU state(2) = dfuIDLE, status(0) = No error condition is present
DFU mode device DFU version 011a
Device returned transfer size 2048
DfuSe interface name: "Internal Flash  "
Downloading element to address = 0x08000000, size = 49412
Erase           [=========================] 100%        49412 bytes
Erase    done.
Download        [=========================] 100%        49412 bytes
Download done.
File downloaded successfully
```
 > Hint: if you are building the firmware from the repo, the DFU command is called by simply writing
```
make dfu
```
 - switch off the synth and remove the USB cable
 - pull the run/program switch up (run) again
 - switch on the synth
 - Click <kbd>shift</kbd> + <kbd>run</kbd> to enter sequence control mode
 - press and hold button <kbd>8</kbd> for more than 2 seconds. This will initialize the sequencer program in flash. The synth is now reset.
 - Press <kbd>run</kbd> again to get back to pattern mode

## Test & Calibration
 - connect a 9V center negative power supply to the barrel jack
 - switch on the device and wait a moment. Turn it off again if you see anything suspicious. ("smoke test")
 - At least the keyboard LEDs should light up in white colour and the LFO LEDs are blinking
### VCO
  - SAW amplitude
  - SQUARE amplitude
  - fine tune & tracking
    - Press a button and tune it to the lowest note (red) using the tune knob
    - Press another button and tune it to the red note two octaves higher
    - Press a third button and tune it to the highest red note
### VCA
  - balance...