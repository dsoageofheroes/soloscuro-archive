# Sol Oscuro
A Dark Sun Engine

Goals: To create an engine for Dark Sun 1, 2, and Crimson Sands.

### Linux dev setup
1) run make
   fix any lib issues that come up (keep in mind we use Lua 5.3!)

### Windows dev setup with Code Blocks:
Right now I don't have build instructions for Windows. The code is C + SDL + lua5.3. Below are the old instructions.
This needs to be updated, as we have migrated to C with SDL.
1. Download mingw (https://sourceforge.net/projects/mingw-w64)  Make sure you select 64-bit when installing (NOT i686!)
1. Install Code Blocks (just regular: nothing integrated.)  make sure mingw is fully installed before installing code blocks!
2. Download Lua 5.3 tarball (https://www.lua.org/versions.html#5.1)
3. Create a 'lua5.3' directory in mingw, mine was located @ C:\Program Files\mingw-w64\x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64\lib\gcc\x86_64-w64-mingw32\8.1.0
4. Extract all the src/*.h files (I had to manually selected each one) to the lua5.1 directory.  (C:\Program Files\mingw-w64\x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64\lib\gcc\x86_64-w64-mingw32\8.1.0\include)
