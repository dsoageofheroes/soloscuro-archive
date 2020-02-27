using System;
using System.IO;
using SDL2;

namespace Sol
{
    class Program
    {
        static void Main(string[] args)
        {
            GFF.gff_print_something();

            // Breakpoint here, console shows nothing
            var gff = new GFF();

            // This prints a ton of stuff
            gff.LoadDirectory("/home/david/Games/DOS/DARKSUN");
            var fileIndex = GFF.gff_find_index("resource.gff");
            var frameCount = GFF.get_frame_count(fileIndex, 13000, 542133570);


            // Console.WriteLine("\r\nFrame count:" + frameCount);

            int wait = 1;
            SDL.SDL_Event ev;
		    SDL.SDL_Init(SDL.SDL_INIT_EVERYTHING);
            // SDL.SDL_SetVideoMode(800, 600, 32, SDL_HWSURFACE);
            SDL.SDL_CreateWindow("TITLE", 100, 100, 800, 600, 0x0);
            while (wait == 1) {
                SDL.SDL_WaitEvent(out ev);
                if (ev.type == SDL.SDL_EventType.SDL_QUIT) {
                    wait = 0;
                }
            }
            SDL.SDL_Quit();
        }
    }
}


