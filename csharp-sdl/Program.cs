using System;
using SDL2;

namespace Sol
{
    class Program
    {
        static void Main(string[] args)
        {
            int wait = 1;
            SDL.SDL_Event ev;
		    SDL.SDL_Init(SDL.SDL_INIT_EVERYTHING);
            //SDL.SDL_WM_SetCaption("Hello World! :)", 0x0);
            //SDL.SDL_SetVideoMode(800, 600, 32, SDL_HWSURFACE);
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
