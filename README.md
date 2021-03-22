# Sol Oscuro
A Dark Sun Engine

Goals: To create an engine for Dark Sun 1, 2, and Crimson Sands.

### Latest Downloads:
<http://dso.paulofthewest.com/downloads.html>

### Linux dev setup
1) run make
   fix any lib issues that come up (keep in mind we use Lua 5.3!)

### Windows dev setup with msys2:

1. Install msys2 (http://msys.org) download the installer and run (must be 64-bit.)
  * default C:\msys64 is fine and assumed for these instructions.
  * update pacman:

     ```$ pacman -Syu```

  * Say yes to everything, make sure the window closes, and then open msys2 again.
  * Update pacman's cache:

     ```$ pacman -Su```

  * Close the window and open MSYS2 Mingw 64-bit

     **MAKE SURE IT IS THE 64-bit one!!!**

  * install all the packages:

     ```$ pacman -S mingw-w64-x86_64-toolchain git mingw-w64-x86_64-toolchain mingw64/mingw-w64-x86_64-SDL2 mingw64/mingw-w64-x86_64-SDL2_mixer mingw64/mingw-w64-x86_64-SDL2_image mingw64/mingw-w64-x86_64-SDL2_ttf mingw64/mingw-w64-x86_64-SDL2_net mingw64/mingw-w64-x86_64-cmake make git mingw-w64-x86_64-libsndfile```

  * The above does take a long time.

2. clone the soloscuro repository:
  * ```$ git clone https://github.com/dsoageofheroes/soloscuro.git```

3. To simplify window dev creation, prebuild libs are provided and must be installed.
  * create a libs/ folder in your clone

    ```$ mkdir libs/```

  * Download the pack (<http://dso.paulofthewest.com/downloads/soloscuro-win64-dev-pack.zip>)
  * FYI: Your clone repos is probably in C:\msys2\home\<your username>\soloscuro
  * Create a libs/ folder inside your repository.
  * extract the lib53.dll and ADLMIDI.dll file into libs/
  * Don't forget to create the libs directory and put the dlls in there.

4. To compile:
  * ```$ make -f makefile.win mdark```

5. Now copy everything from the 'soloscuro-lib' folder in the previous zip file (<http://dso.paulofthewest.com/downloads/soloscuro-win64-dev-pack.zip>) into the run folder (which should have only a mdark.exe file in it right now.

  * This should setup all the dlls/so for the game.

6. To run you will need to create a ds1 folder in run with all the ds1's gffs in it. Now run mdark.exe (inside of the run folder.)
