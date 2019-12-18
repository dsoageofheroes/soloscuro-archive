# GPL/DSL Design Document

This document is to explain the connection between the C-library (called libds in this document) and Lua (called UI in
this document) for the Game Programming Language (GPL.)  For now, this document will only be concerned with the implementation of Darksun 1 Shattered Lands.  Further features are ignored for now in order to save time.

## Quick Data Flow:
----------                ------------                   --------
| GFFs   |  -loaded by->  |  libds  |  <-- Game data --> |  UI  |
----------                ------------                   --------

- libds, written in C, provides solid bit/byte manipulation which works well for the data structures of the GFF
files/DarkSun and is cross platform.

- UI, written in Lua/Love2D, provides many solid facilities for UI and sound, and is supported across many
platforms.

## Game Programming Language
The Game Programming Language (GPL) was created to assist in the development of the Darksun games.  Scripters would write
scripts in GPL to depict game events throughout Darksun.  It was the programmers job to run these scripts.  Websites
tell that there "sysops" tools used by the scripters to program Darksun.  Presumably, this means the DevOps was:

--------------                      ----------------------------                      --------------
|GFF with GPL| - loaded by sysop -> |Scripter creates/edits GPL| - saves via sysop -> |GFF with GPL|
--------------                      ----------------------------                      --------------

It is not known if the scripters edited the GPL as text or as a special "event" editor specific for the GPL.

The GPL manages the following:
- Game World State (Examples: Have we fought in the arena?  Did we visit Dagolar?)
- Game World Tiggers and effects (Examples: After a certain number of fights in the arena, we fight Scar.)
- NPC Narration/Communication

## GPL State:
To fulfill the task of managing the abov the following state is held in the GPL:
- Global flags (Flags used throughout the game)
- Local flags (Flags specific to a local area, EX: have we talked to this NPC?)
- Objects in the world (PCs, monsters, NPCs, etc...)
  - This includes stats and statuses.  Not, imagery data.
- tile and box triggers for maps.
- Text for narrate, as well as possible responses.

## GPL Functions:
I have classified GPL Functions into the following categories:
1) Arithmetic/Language Control: These are for language functionality (if/whens/function calls) as well as some specific
mechanisms for GPL.
2) Orders: These tell the game engine to make a PC/NPC perform an action (Example: flee, fight, go to X,Y)
3) Game State: These change the game state (get gold/exp, drop items, setup triggers)
4) Game Actions: These are non-order actions (narration, heal, rest, open door, etc..)
5) The final section is a list of functions that do not appear to be in DS1, but are used in DS2/DSO.

To see the list of functions, please jump to the end.

When starting the game the following occurs:
1) Run MAS 99 (master file 99 initializes the GPL Game State.)
2) If loading from a save file, load the GPL state from there.
3) When loading an area clear the local state and then load MAS XX (where XX is the region code.) to initialize the area
states.

#UI and libds Interface

The interface between the UI and libds shall consist of Lua objects implemented with C/libds and lua stubs.  The Lua objects will
represent any necessary state needed by the UI (NPC, monster, text, etc...)  The stubs (functions) will be what is called
when the GPL needs to perform an action (like create a narration dialog.)  This design
follows the classic wrapper method used in numerous programs, including Love's wrappers around C.  This should allow for quick
implementation, the ability to create mods, and allow us to change the backend in the future.

---------------                      -------
|UI/Lua object| - implemented via -> |libds|
---------------                      -------

-------            -------------
|libds| - calls -> |UI function|
-------            -------------

##Division of Work

Some parts lend itself to either the UI or libds.  Others will require a mix between the two.

Functions:
Arithmetic/Language Control: libds
Orders: UI
Game State: libds, with some UI.
Game Actions: UI

The objects are a hybrid since the game data is exposed to Lua via wrappers.

---------------------------------List of GPL Functions--------------------------------

Arithmetic/Language Control:
dsl_zero
dsl_long_divide_equal
dsl_byte_dec
dsl_word_dec
dsl_long_dec
dsl_byte_inc
dsl_word_inc
dsl_long_inc
dsl_string_copy
dsl_long_times_equal
dsl_jump
dsl_local_sub
dsl_global_sub
dsl_local_ret
dsl_load_variable
dsl_compare
dsl_load_accum
dsl_global_ret
dsl_nametonum
dsl_numtoname
dsl_bitsnoop
dsl_ifis
dsl_orelse
dsl_continue
dsl_exit_dsl
dsl_if
dsl_else
dsl_setrecord
dsl_setother
dsl_rand
dsl_cmpend
dsl_while
dsl_wend
dsl_endif
dsl_word_plus_equal
dsl_word_minus_equal
dsl_word_times_equal
dsl_word_divide_equal
dsl_long_plus_equal
dsl_long_minus_equal

Orders: (Tell NPC/PC to do something.)
dsl_hunt
dsl_fetch
dsl_search
dsl_fight
dsl_flee
dsl_follow
dsl_go
dsl_goxy
dsl_string_compare
dsl_wait

Game State:
dsl_getxy
dsl_p_damage
dsl_changemoney
dsl_getstatus
dsl_getlos
dsl_nextto
dsl_inloscheck
dsl_notinloscheck
dsl_clear_los
dsl_award
dsl_clone
dsl_getparty
dsl_give
dsl_readorders
dsl_setthing
dsl_statroll
dsl_take
dsl_get_range
dsl_attackcheck
dsl_lookcheck
dsl_move_tilecheck
dsl_door_tilecheck
dsl_move_boxcheck
dsl_door_boxcheck
dsl_pickup_itemcheck
dsl_usecheck
dsl_talktocheck
dsl_noorderscheck
dsl_usewithcheck

Game Actions
dsl_shop
dsl_damage
dsl_drop
dsl_passtime
dsl_sound
dsl_tport
  - Narrate:
  dsl_clearpic -- clear the picture of narration.
  dsl_getyn -- Ask Yes/No
  dsl_input_string
  dsl_menu
  dsl_print_string
  dsl_print_number
  dsl_printnl
  dsl_showpic
dsl_request, which includes:
  healing : camping
  door : operate a door
  thiefskill : perform thief skill
  animation : perform an object animation.
  cinematic : perform a cinmatic
  trap : create a trap
  request_monster : create a monster?
  swap : swap items
  set_block : set a block trigger
  clear_block : clear a block trigger
  set_los : set the line of sight.
  clear_los: clear the line of sight
  request_battle_demo : Do the battle demo (first battle in the arena.)
  set_game_move : exit everything (like battle) and move to regular mode.
  branch_music : play music!
  flash_animation : flash something.
  set_allegiance : change object's allegiance
  end_game : do the end game sequence.
  request_count_combat : get the number of combatants.

Not currently detected to be used in DS1.  (Warning, these may have been missed...):
dsl_setvar
dsl_toggle_accum
dsl_source_trace
dsl_trace_var
dsl_log
dsl_source_line_num
dsl_byte_plus_equal
dsl_byte_minus_equal
dsl_byte_times_equal
dsl_byte_divide_equal
dsl_input_bignum
dsl_input_number
dsl_input_money
dsl_joinparty
dsl_leaveparty
dsl_lockdoor
dsl_local_sub_trace
dsl_skillroll
dsl_match_string
dsl_music

