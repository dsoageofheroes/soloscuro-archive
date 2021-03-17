# Sol Oscuro
A Dark Sun Engine

Goals: To create an engine for Dark Sun 1, 2, and Crimson Sands.

### Latest Downloads:
<http://192.168.254.28/downloads.php>

### Linux dev setup
1) run make
   fix any lib issues that come up (keep in mind we use Lua 5.3!)

### Windows dev setup with msys2:

1. install msys2 and be sure to pull in SDL2.
2. I've streamlined the install of many libraries with a install pack. Get it here: <http://dso.paulofthewest.com/downloads/soloscuro-win64-dev-pack.zip>
3. Unzip that pack and copy everyting from include to mingw's include directory (I need to update this step.)
4. Now copy all the libs (.dll AND .so) into a libs/ directory in root of the repository. When you do a windows build it will look in libs/ of the repository.
5. to compile make -f makefile.win mdark.
