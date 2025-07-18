
Some brief notes from my own experience on Debian (I use a Debian VM
in VirtualBox since although I have Fedora running on my laptop,
Debian seemed to have much better support for the pico):

    sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi \
      libstdc++-arm-none-eabi-newlib g++ picotool gcc-multilib

Either

    git clone https://github.com/TomWhitwell/Workshop_Computer.git

or (if you've forked to your own account)

    git clone https://github.com/USER/Workshop_Computer.git

then

    cd Workshop_Computer/Demonstrations+HelloWorlds/PicoSDK
    mkdir build
    PICO_SDK_FETCH_FROM_GIT=1 cmake -S ComputerCard -B build
    cd build
    cmake --build .

after that, copy the appropriate uf2 file to the host machine (if
working in a VM), connect the hardware to the computer using USB, and
then

    cp xxx.uf2 /run/media/USER/RPI-RP2/

(or whatever device appears for you).

- Andrew Cooke




