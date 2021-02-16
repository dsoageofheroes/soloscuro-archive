-- player_input.lua
-- Handles all player input that isn't hard-coded (TODO: Add mouse support?)

-- To add/change bindings: Look up SDL2's key bindings, replace SDLK with DSLK (https://wiki.libsdl.org/SDL_Keycode)
-- Add "key_handled = true" to any keys you don't want passed to the engine
-- If you don't, the key will execute its default function (if any) in addition to the one here

-- Anonymous functions are only necessary for functions that require arguments, but uniformity is aesthetic

-- *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME*
-- How to properly handle keyup with modifier keys? If a player has an o UP binding, an ALT+o DOWN binding, and an ALT+o UP binding,
-- if the player lets go of ALT first, the ALT+o UP binding won't execute, but if the player lets go of o first, the ALT+o up binding
-- will execute as desired. Additionally, if the player holds down SHIFT, while holding down ALT+o and then releases the o key, the
-- ALT+SHIFT+o UP binding will execute, if any.

-- Three options I can easily think of (other than leaving it as-is):
-- 1)	Store the last keydown combo and execute its keyup counterpart when any new key is pressed
-- 2)	Execute the last keydown combo's keyup counterpart only when a new modifier key is pressed
-- 3)	Store a list of keydowns that were executed (until all keys are released), then execute keyups when the primary
--		(non-modifier) key for each is released (best solution?)
-- *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME* *FIXME*


-- DOWN: NO MODIFIERS
KEYS_NOMODS_DOWN					= {}
KEYS_NOMODS_DOWN[DSLK_l]			= function() ds.toggle_inventory() end
KEYS_NOMODS_DOWN[DSLK_h]			= function() ds.player_move(PLAYER_LEFT) end
KEYS_NOMODS_DOWN[DSLK_KP_8]			= function() ds.set_yscroll(-5) end
KEYS_NOMODS_DOWN[DSLK_ESCAPE]		= function() ds.exit_game() key_handled = true end


-- DOWN: ALT
KEYS_ALT_DOWN						= {}


-- DOWN: CTRL
KEYS_CTRL_DOWN						= {}
KEYS_CTRL_DOWN[DSLK_o]				= function() ds.toggle_inventory() end


-- DOWN: SHIFT
KEYS_SHIFT_DOWN						= {}


-- DOWN: ALT+CTRL
KEYS_ALT_CTRL_DOWN					= {}


-- DOWN: ALT+SHIFT
KEYS_ALT_SHIFT_DOWN					= {}


-- DOWN: ALT+CTRL+SHIFT
KEYS_ALT_CTRL_SHIFT_DOWN			= {}


-- UP: NO MODIFIERS
KEYS_NOMODS_UP						= {}
KEYS_NOMODS_UP[DSLK_h]				= function() ds.player_unmove(PLAYER_LEFT) end
KEYS_NOMODS_UP[DSLK_KP_8]			= function() ds.set_yscroll(0) end


-- UP: ALT
KEYS_ALT_UP							= {}


-- UP: CTRL
KEYS_CTRL_UP						= {}
KEYS_CTRL_UP[DSLK_o]				= function() ds.toggle_inventory() end


-- UP: SHIFT
KEYS_SHIFT_UP						= {}


-- UP: ALT+CTRL
KEYS_ALT_CTRL_UP					= {}


-- UP: ALT+SHIFT
KEYS_ALT_SHIFT_UP					= {}


-- UP: CTRL+SHIFT
KEYS_CTRL_SHIFT_UP					= {}


-- UP: ALT+CTRL+SHIFT
KEYS_ALT_CTRL_SHIFT_UP				= {}


-- KEY DOWN --
function keydown(key)
	key_handled = false

	-- Handle the 
	-- arg1 = keycode, arg2 = true for being called from keydown(), false for being called from keyup()
	-- We check the KEYS_NOMODS_DOWN table first so we can allow the player to bind ALT/SHIFT/CTRL, if they choose to
	if KEYS_NOMODS_DOWN[key] == nil and handle_modifier_keys(key, true) then
		return false -- return false so engine receives key input
	end

	-- No modifier keys are down, check the key alone
	if KEYS_NOMODS_DOWN[key] then
		KEYS_NOMODS_DOWN[key]()
		return key_handled
	end

	-- ALT+key
	if alt_is_down() and KEYS_ALT_DOWN[key] then
		KEYS_ALT_DOWN[key]()
		return key_handled
	end

	-- CTRL+key
	if ctrl_is_down() and KEYS_CTRL_DOWN[key] then
		KEYS_CTRL_DOWN[key]()
		return key_handled
	end

	-- SHIFT+key
	if shift_is_down() and KEYS_SHIFT_DOWN[key] then
		KEYS_SHIFT_DOWN[key]()
		return key_handled
	end

	-- ALT+CTRL+key
	if alt_is_down() and ctrl_is_down() and KEYS_ALT_CTRL_DOWN[key] then
		KEYS_ALT_CTRL_DOWN[key]()
		return key_handled
	end

	-- ALT+SHIFT+key
	if alt_is_down() and shift_is_down() and KEYS_ALT_SHIFT_DOWN[key] then
		KEYS_ALT_SHIFT_DOWN[key]()
		return key_handled
	end

	-- CTRL+SHIFT+key
	if ctrl_is_down() and shift_is_down() and KEYS_CTRL_SHIFT_DOWN[key] then
		KEYS_CTRL_SHIFT_DOWN[key]()
		return key_handled
	end

	-- ALT+CTRL+SHIFT+key
	if alt_is_down() and ctrl_is_down() and shift_is_down() and KEYS_ALT_CTRL_SHIFT_DOWN[key] then
		KEYS_ALT_CTRL_SHIFT_DOWN[key]()
		return key_handled
	end

    return false	-- returning false allows the engine to do any default actions.
					-- returning true would make lua handle ALL keydown functions and disable the engine's keybindings.
end

-- KEY UP --
function keyup(key)
	key_handled = false

	-- Key modifiers are in a separate function that handles both key up and key up, so this function
	-- isn't cluttered with anything other than actual keybindings.
	-- arg1 = keycode, arg2 = true for being called from keyup(), false for being called from keydown()
	-- We check the KEYS_NOMODS_UP table first so we can allow the player to bind ALT/SHIFT/CTRL, if they choose to
	if KEYS_NOMODS_UP[key] == nil and handle_modifier_keys(key, false) then
		return false -- return false so engine receives key input
	end

	-- No modifier keys are up, check the key alone
	if KEYS_NOMODS_UP[key] then
		KEYS_NOMODS_UP[key]()
		return key_handled
	end

	-- ALT+key
	if alt_is_down() and KEYS_ALT_UP[key] then
		KEYS_ALT_UP[key]()
		return key_handled
	end

	-- CTRL+key
	if ctrl_is_down() and KEYS_CTRL_UP[key] then
		KEYS_CTRL_UP[key]()
		return key_handled
	end

	-- SHIFT+key
	if shift_is_down() and KEYS_SHIFT_UP[key] then
		KEYS_SHIFT_UP[key]()
		return key_handled
	end

	-- ALT+CTRL+key
	if alt_is_down() and ctrl_is_down() and KEYS_ALT_CTRL_UP[key] then
		KEYS_ALT_CTRL_UP[key]()
		return key_handled
	end

	-- ALT+SHIFT+key
	if alt_is_down() and shift_is_down() and KEYS_ALT_SHIFT_UP[key] then
		KEYS_ALT_SHIFT_UP[key]()
		return key_handled
	end

	-- CTRL+SHIFT+key
	if ctrl_is_down() and shift_is_down() and KEYS_CTRL_SHIFT_UP[key] then
		KEYS_CTRL_SHIFT_UP[key]()
		return key_handled
	end

	-- ALT+CTRL+SHIFT+key
	if alt_is_down() and ctrl_is_down() and shift_is_down() and KEYS_ALT_CTRL_SHIFT_UP[key] then
		KEYS_ALT_CTRL_SHIFT_UP[key]()
		return key_handled
	end

    return false	-- returning false allows the engine to do any default actions.
					-- returning true would make lua handle ALL keyup functions and disable the engine's keybindings.
end

function handle_modifier_keys(key, called_from_key_down)
	if called_from_key_down then -- We were called from the keydown() function
		if key == DSLK_LALT then -- LALT
			lalt_is_down = true
			return true
		end
		if key == DSLK_RALT then -- RALT
			ralt_is_down = true
			return true
		end
		if key == DSLK_LCTRL then -- LCTRL
			lctrl_is_down = true
			return true
		end
		if key == DSLK_RCTRL then -- RCTRL
			rctrl_is_down = true
			return true
		end
		if key == DSLK_LSHIFT then -- LSHIFT
			lshift_is_down = true
			return true
		end
		if key == DSLK_RSHIFT then -- RSHIFT
			rshift_is_down = true
			return true
		end
	else -- We were called from the keyup() function
		if key == DSLK_LALT then -- LALT
			lalt_is_down = false
			return true
		end
		if key == DSLK_RALT then -- RALT
			ralt_is_down = false
			return true
		end
		if key == DSLK_LCTRL then -- LCTRL
			lctrl_is_down = false
			return true
		end
		if key == DSLK_RCTRL then -- RCTRL
			rctrl_is_down = false
			return true
		end
		if key == DSLK_LSHIFT then -- LSHIFT
			lshift_is_down = false
			return true
		end
		if key == DSLK_RSHIFT then -- RSHIFT
			rshift_is_down = false
			return true
		end
	end

	return false -- The key isn't a recognized modifier key
end

function alt_is_down() return lalt_is_down or ralt_is_down end -- Returns true if we think LALT or RALT are in the down state
function ctrl_is_down() return lctrl_is_down or rctrl_is_down end -- Returns true if we think LCTRL or RCTRL are in the down state
function shift_is_down() return lshift_is_down or rshift_is_down end -- Returns true if we think LSHIFT or RSHIFT are in the down state
function alt_is_up() return not(lalt_is_down and ralt_is_down) end -- Returns true if we think LALT and RALT are in the up state
function ctrl_is_up() return not(lctrl_is_down and rctrl_is_down) end -- Returns true if we think LCTRL and RCTRL are in the up state
function shift_is_up() return not(lshift_is_down and rshift_is_down) end -- Returns true if we think LSHIFT and RSHIFT are in the up state
