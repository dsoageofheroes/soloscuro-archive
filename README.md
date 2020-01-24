# Sol Oscuro
A Dark Sun Engine

### Linux dev setup
1) run make
   fix any lib issues that come up (keep in mind we use Lua 5.1!)

### Windows dev setup with Code Blocks:
1. Install Code Blocks with mingw support.
2. Download Lua 5.1.5 tarball (https://www.lua.org/versions.html#5.1)
3. Create a 'lua5.1' directory in mingw, mine was located @ C:\Program Files (x86)\CodeBlocks\MinGW\include
4. Extract all the src/*.h files (I had to manually selected each one) to the lua5.1 directory.  (C:\Program Files (x86)\CodeBlocks\MinGW\include\lua5.1\)
5. Now get the Lua 5.1 dll Library (https://sourceforge.net/projects/luabinaries/files/5.1.5/Windows%20Libraries/Dynamic/lua-5.1.5_Win32_dllw6_lib.zip/download)
6. Extract the liblua.dll file to mingw's lib location (C:\Program Files (x86)\CodeBlocks\MinGW\lib)
7. ... to be continued...
