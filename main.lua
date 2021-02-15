function init()
    ds.load_charsave(0, 9) -- load character from charsave.gff
    ds.load_charsave(1, 10) -- load character from charsave.gff

    ds.load_region(42) -- right now only 42 works (there is some hard-coding.)
    ds.set_player_frame_delay(7) -- set how many delay ticks between frame changes of a player sprite. (default: 7)
    ds.set_player_move(10) -- set how many ticks to move one slot. (default: 10)

    return true; -- returning true, means that lua has setup the game, don't do the default
                 -- returning false, would allow the engine to proceed with normal startup
end

function keydown(key)
    if key == DSLK_l then -- Look up SDL2's key bindings, replace SDLK with DSLK (https://wiki.libsdl.org/SDL_Keycode)
        ds.toggle_inventory();
    end
    if key == DSLK_LCTRL then -- Look up SDL2's key bindings, replace SDLK with DSLK (https://wiki.libsdl.org/SDL_Keycode)
        ctrl_is_down = true;
    end
    if ctrl_is_down and key == DSLK_o then -- effectivly left ctrl+o
        ds.toggle_inventory();
    end
    if key == DSLK_h then -- keep in mind you have to unset the move with player (see keyup)
        ds.player_move(PLAYER_LEFT)
    end
    if key == DSLK_KP_8 then -- keep in mind you have to unset the move with xscroll (see keyup)
        ds.set_yscroll(-5)
    end
    if key == DSLK_ESCAPE then -- exit the game
        ds.exit_game()
        return true; -- If you return true in the if, then you say we handle this key and don't do pass-through.
    end
    return false; -- returning false allows the engine to do any default actions.
                  -- returning true would make lua handle ALL keydown functions and disable the engine's keybindings.
end

function keyup(key)
    if key == DSLK_LCTRL then -- Look up SDL2's key bindings, replace SDLK with DSLK (https://wiki.libsdl.org/SDL_Keycode)
        ctrl_is_down = false;
    end
    if key == DSLK_h then -- Look up SDL2's key bindings, replace SDLK with DSLK (https://wiki.libsdl.org/SDL_Keycode)
        ds.player_unmove(PLAYER_LEFT)
    end
    if key == DSLK_KP_8 then -- keep in mind you have to unset the move with player (see keyup)
        ds.set_yscroll(0)
    end
    return false; -- returning false allows the engine to do any default actions.
                  -- returning true would make lua handle ALL keyup functions and disable the engine's keybindings.
end
