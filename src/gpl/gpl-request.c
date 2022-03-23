#include <stdlib.h>
#include "gpl.h"
#include "region.h"
#include "region-manager.h"
#include "gpl-state.h"
#include "port.h"
#include "trigger.h"

enum requests {
    REQUEST_NONE, //0
    REQUEST_HEALING, //1
    REQUEST_DOOR, //2
    REQUEST_THIEFSKILL, //3
    REQUEST_REST, //4
    REQUEST_ANIMATION, //5
    REQUEST_CINEMATIC, //6
    REQUEST_TRAP, //7
    REQUEST_MONSTER, //8
    REQUEST_SWAP, //9
    REQUEST_SET_BLOCK, //10
    REQUEST_CLEAR_BLOCK, //11
    REQUEST_SET_LOS, //12
    REQUEST_CLEAR_LOS, //13
    REQUEST_BATTLE_DEMO, //14
    REQUEST_SET_GAME_MOVE, //15
    REQUEST_BRANCH_MUSIC, //16
    REQUEST_FLASH_ANIMATION, //17
    REQUEST_SET_ALLEGIANCE, //18
    REQUEST_END_GAME, //19
    REQUEST_COUNT_COMBAT, //20
    REQUEST_DIFFICULT_LEVEL, //21
    REQUEST_END_TURN, //22
    REQUEST_APPLY_CONDITION, //23
    REQUEST_DETECT_CONDITION, // 24
    REQUEST_SURPRISE_ATTACK, // 25
    REQUEST_PALETTE_CHANGE, // 26
    REQUEST_SHAKE, // 27
    REQUEST_SET_EXCHANGE_RATE, // 28
    REQUEST_PUSH, // 29
    REQUEST_POP, // 30
    REQUEST_SET_DANGER_BIT, // 31
    REQUEST_CLEAR_DANGER_BIT, // 32
    REQUEST_SET_BORDER, // 33
    REQUEST_SET_OVERHEAD, // 34
    REQUEST_SET_DEATH_SCREEN, // 35
    REQUEST_REDRAW_SCREEN, // 36
    REQUEST_FADE_SCREEN, // 37
    REQUEST_CENTER_XY, // 38
    REQUEST_INTERMISSION, // 39
    REQUEST_REMOVE_CONDITION, // 40
    REQUEST_SAVE_VS, // 41
    REQUEST_SET_PLAYLIST, // 42
    REQUEST_CURRENT_PLAYLIST, // 43
    REQUEST_DO_SPEECH,
    REQUEST_OBJECT_ANIMATING,
    REQUEST_CAST_SPELL,
    REQUEST_PREPARE_SPELL,
    REQUEST_GET_DATA,
    REQUEST_SET_DATA,
    REQUEST_IS_CD,
    REQUEST_DEPLOY_PARTY,
    REQUEST_SET_PAIN,
    REQUEST_KILL_BRAIN,
    REQUEST_SET_SHARED_GLOBAL,
    REQUEST_SET_SHARED_REGION,
    REQUEST_GET_SHARED_GLOBAL,
    REQUEST_GET_SHARED_REGION,
    REQUEST_GET_SEMAPHORE_GLOBAL,
    REQUEST_CHECK_SEMAPHORE_GLOBAL,
    REQUEST_RELEASE_SEMAPHORE_GLOBAL,
    REQUEST_GET_SEMAPHORE_REGION,
    REQUEST_CHECK_SEMAPHORE_REGION,
    REQUEST_RELEASE_SEMAPHORE_REGION,
    REQUEST_GAMETICK,
    REQUEST_TEXT_SCOPE,
    REQUEST_SOUND_SCOPE,
    REQUEST_WHOISLEADER,
    REQUEST_DATE,
    REQUEST_TIME,
    REQUEST_PARTY_ID,
    REQUEST_NEAR_ID,
    REQUEST_INFO_SHORT_ID,
    REQUEST_INFO_LONG_ID,
    REQUEST_CHAR_INFO,
    REQUEST_CHAR_FROM,
    REQUEST_CHAR_TO,
    REQUEST_MY_CHAR_FROM,
    REQUEST_MY_CHAR_TO,
    REQUEST_SELECTED_ID,
    REQUEST_COMBAT_NUM,
    REQUEST_CLONE_NUM,
    REQUEST_GET_CLONE,
    REQUEST_GET_COMBAT_NUM,
    REQUEST_DETACH,
    REQUEST_R_LIFE,
    REQUEST_REGIONCLEAR,
    REQUEST_REGIONCOUNT,
    REQUEST_NAMEID,
    REQUEST_SPELLINFO,
    REQUEST_HIGHSCORE,
    REQUEST_QUEST,
    REQUEST_LEADER_XYR,
    REQUEST_PRINT_NAME,
    REQUEST_ISPLAYER,
    REQUEST_SCROLLWHOLE,
    REQUEST_ISCHEATMODE,
    REQUEST_ABORTENDGAME,
    REQUEST_DRAINLEVEL,
    REQUEST_SECONDS,
    REQUEST_L_ANIMATION,
    REQUEST_SEARCHITEM,
    REQUEST_SPELLALLOW,
    REQUEST_REGIONFILLED,
    REQUEST_ABORTACTION,
    REQUEST_MONSTERCOUNT,
    REQUEST_NOFLEE,
    REQUEST_ARRESTREASON,
    REQUEST_CHECKNEW,
    REQUEST_IMPERSONATE,
    REQUEST_PASSTIME,
    REQUEST_PROFILE,
    REQUEST_MYSLOT,
    MAX_REQUEST
};

#define DO_TO_ALL (-1)
#define DO_COMBAT (2)

#define GB_LOS    (0x80)
#define GB_BLOCK  (0x40)
#define GB_ACTOR  (0x20)
#define GB_DANGER (0x07)
#define GB_GPL    (0x04)

static int req_animation(int16_t object, long notused1, long notused2);
static int req_set_allegiance(int16_t object, long allegiance, long notused2);
static int16_t request_to_do(int16_t name, int16_t rectype, int (*request_proc)(int16_t, long, long), long param1, long
param2);

#define CAMP_HEAL      (0)
#define CAMP_RESURRECT (1)
#define CAMP_TIME      (1000)

static void camp(int16_t instr, int16_t hours, int16_t who) {
    debug("%d camps for %d hours\n", who, hours);
    if (instr == CAMP_RESURRECT) {
        debug("Also ressurect the dead, unstone the petrify, revive undead, etc...\n");
    }
}

extern uint32_t gpl_request_impl(int16_t token, int16_t name,
        int32_t num1, int32_t num2) {
    int answer = 0;
    disk_object_t dobj;

    switch (token) {
        case REQUEST_HEALING:
            debug("Time to camp!\n");
            camp(CAMP_RESURRECT, num1, num2);
            break;
        case REQUEST_DOOR:
            debug("I need to do operation %d on door %d\n", num2, num1);
            break;
        case REQUEST_THIEFSKILL:
            debug("I need to run a thief skill on %d, skill %d, bonus %d\n", name, num1, num2);
            break;
        case REQUEST_REST:
            debug("request REST not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_ANIMATION:
            request_to_do(name, 0, req_animation, num1, num2);
            break;
        case REQUEST_CINEMATIC:
            debug("I need to play the %d cinematic\n", num1);
            break;
        case REQUEST_TRAP:
            debug("I need to lay a trap of type %d, at %d with %d\n", name, num1, num2);
            break;
        case REQUEST_MONSTER:
            debug("I need to request a monster %d, %d\n", num1, num2);// Not helpful, I know...
            break;
        case REQUEST_SWAP:
            if (!gff_read_object(num1, &dobj)) {
                error("Unable to satisfy REQUEST_SWAP, not obj: %d\n", num1);
            }
            sol_region_t *reg = sol_region_manager_get_current();
            dude_t *dude = sol_region_find_entity_by_id(reg, name);
            dude->sprite.bmp_id = dobj.bmp_id;
            dude->anim.bmp_id = dobj.bmp_id;
            sol_trigger_object_clear(name);
            port_swap_enitity(name, dude);
            break;
        case REQUEST_SET_BLOCK:
            debug("Need to set (BLOCK) the bit flags for %d map position (%d, %d) to %d & commit!\n", name, num1, num2, GB_BLOCK);
            sol_region_set_block(sol_region_manager_get_current(), num2, num1, MAP_BLOCK);
            sol_trigger_enable_object(name);
            break;
        case REQUEST_CLEAR_BLOCK:
            debug("I need to clear (UNBLOCK) the block at (%d, %d) with flags %d\n", num1, num2, GB_BLOCK);
            sol_region_clear_block(sol_region_manager_get_current(), num2, num1, MAP_BLOCK);
            break;
        case REQUEST_SET_LOS:
            debug("request SET_LOS not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_CLEAR_LOS:
            debug("I need to CLEAR_LOS, whatever that means...\n");
            break;
        case REQUEST_BATTLE_DEMO:
            debug("request REQUEST_BATTLE_DEMO: Need to call lua or something to run the demo!\n");
            sol_trigger_noorders_enable_all();
            break;
        case REQUEST_SET_GAME_MOVE:
            debug("I need to set the game back to regular moving around (not combat/look/xfer/target).\n");
            break;
        case REQUEST_BRANCH_MUSIC:
            switch(num2) {
                case 0:
                    debug("I need to stop music and play %d\n", num1);
                    break;
                case 1:
                    debug("I need to play music %d next\n", num1);
                    break;
                case 2:
                    debug("CASE 2 notimplemented\n");
                    command_implemented = 0;
                    break;
                case 3:
                    debug("I need to play music %d next\n", num1);
                    break;
                case 4:
                    debug("CASE 4 notimplemented\n");
                    command_implemented = 0;
                    break;
            }
            break;
        case REQUEST_FLASH_ANIMATION:
            debug("I need to flash (-)%d at (%d, %d)\n", name, num1, num2);
            break;
        case REQUEST_SET_ALLEGIANCE:
            return request_to_do(name, DO_COMBAT, req_set_allegiance, num1, num2);
            break;
        case REQUEST_END_GAME:
            debug("Request END_GAME: end game and start over...\n");
            break;
        case REQUEST_COUNT_COMBAT:
            debug("I need to return the number of combatants that have no allegiance to %d\n", num1);
            break;
        case REQUEST_DIFFICULT_LEVEL:
            debug("request REQUEST_DIFFICULT_LEVEL not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_END_TURN:
            debug("request REQUEST_END_TURN not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_APPLY_CONDITION:
            debug("request REQUEST_APPLY_CONDITION not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_DETECT_CONDITION:
            debug("request REQUEST_DETECT_CONDITION not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SURPRISE_ATTACK:
            debug("request REQUEST_SURPRISE_ATTACK not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_PALETTE_CHANGE:
            debug("request REQUEST_PALETTE_CHANGE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SHAKE:
            debug("request REQUEST_SHAKE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SET_EXCHANGE_RATE:
            debug("request REQUEST_SET_EXCHANGE_RATE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_PUSH:
            debug("request REQUEST_PUSH not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_POP:
            debug("request REQUEST_POP not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SET_DANGER_BIT:
            debug("request REQUEST_SET_DANGER_BIT not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_CLEAR_DANGER_BIT:
            debug("request REQUEST_CLEAR_DANGER_BIT not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SET_BORDER:
            debug("request REQUEST_SET_BORDER not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SET_OVERHEAD:
            debug("request REQUEST_SET_OVERHEAD not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SET_DEATH_SCREEN:
            debug("request REQUEST_SET_DEATH_SCREEN not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_REDRAW_SCREEN:
            debug("request REQUEST_REDRAW_SCREEN not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_FADE_SCREEN:
            debug("request REQUEST_FADE_SCREEN not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_CENTER_XY:
            debug("request REQUEST_CENTER_XY not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_INTERMISSION:
            debug("request REQUEST_INTERMISSION not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_REMOVE_CONDITION:
            debug("request REQUEST_REMOVE_CONDITION not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SAVE_VS:
            debug("request REQUEST_SAVE_VS not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SET_PLAYLIST:
            debug("request REQUEST_SET_PLAYLIST not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_CURRENT_PLAYLIST:
            debug("request REQUEST_CURRENT_PLAYLIST not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_DO_SPEECH:
            debug("request REQUEST_DO_SPEECH not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_OBJECT_ANIMATING:
            debug("request REQUEST_OBJECT_ANIMATING not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_CAST_SPELL:
            debug("request REQUEST_CAST_SPELL not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_PREPARE_SPELL:
            debug("request REQUEST_PREPARE_SPELL not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_GET_DATA:
            debug("request REQUEST_GET_DATA not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SET_DATA:
            debug("request REQUEST_SET_DATA not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_IS_CD:
            debug("request REQUEST_IS_CD not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_DEPLOY_PARTY:
            debug("request REQUEST_DEPLOY_PARTY not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SET_PAIN:
            debug("request REQUEST_SET_PAIN not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_KILL_BRAIN:
            debug("request REQUEST_KILL_BRAIN not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SET_SHARED_GLOBAL:
            debug("request REQUEST_SET_SHARED_GLOBAL not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SET_SHARED_REGION:
            debug("request REQUEST_SET_SHARED_REGION not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_GET_SHARED_GLOBAL:
            debug("request REQUEST_GET_SHARED_GLOBAL not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_GET_SHARED_REGION:
            debug("request REQUEST_GET_SHARED_REGION not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_GET_SEMAPHORE_GLOBAL:
            debug("request REQUEST_GET_SEMAPHORE_GLOBAL not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_CHECK_SEMAPHORE_GLOBAL:
            debug("request REQUEST_CHECK_SEMAPHORE_GLOBAL not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_RELEASE_SEMAPHORE_GLOBAL:
            debug("request REQUEST_RELEASE_SEMAPHORE_GLOBAL not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_GET_SEMAPHORE_REGION:
            debug("request REQUEST_GET_SEMAPHORE_REGION not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_CHECK_SEMAPHORE_REGION:
            debug("request REQUEST_CHECK_SEMAPHORE_REGION not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_RELEASE_SEMAPHORE_REGION:
            debug("request REQUEST_RELEASE_SEMAPHORE_REGION not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_GAMETICK:
            debug("request REQUEST_GAMETICK not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_TEXT_SCOPE:
            debug("request REQUEST_TEXT_SCOPE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SOUND_SCOPE:
            debug("request REQUEST_SOUND_SCOPE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_WHOISLEADER:
            debug("request REQUEST_WHOISLEADER not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_DATE:
            debug("request REQUEST_DATE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_TIME:
            debug("request REQUEST_TIME not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_PARTY_ID:
            debug("request REQUEST_PARTY_ID not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_NEAR_ID:
            debug("request REQUEST_NEAR_ID not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_INFO_SHORT_ID:
            debug("request REQUEST_INFO_SHORT_ID not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_INFO_LONG_ID:
            debug("request REQUEST_INFO_LONG_ID not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_CHAR_INFO:
            debug("request REQUEST_CHAR_INFO not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_CHAR_FROM:
            debug("request REQUEST_CHAR_FROM not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_CHAR_TO:
            debug("request REQUEST_CHAR_TO not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_MY_CHAR_FROM:
            debug("request REQUEST_MY_CHAR_FROM not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_MY_CHAR_TO:
            debug("request REQUEST_MY_CHAR_TO not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SELECTED_ID:
            debug("request REQUEST_SELECTED_ID not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_COMBAT_NUM:
            debug("request REQUEST_COMBAT_NUM not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_CLONE_NUM:
            debug("request REQUEST_CLONE_NUM not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_GET_CLONE:
            debug("request REQUEST_GET_CLONE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_GET_COMBAT_NUM:
            debug("request REQUEST_GET_COMBAT_NUM not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_DETACH:
            debug("request REQUEST_DETACH not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_R_LIFE:
            debug("request REQUEST_R_LIFE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_REGIONCLEAR:
            debug("request REQUEST_REGIONCLEAR not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_REGIONCOUNT:
            debug("request REQUEST_REGIONCOUNT not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_NAMEID:
            debug("request REQUEST_NAMEID not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SPELLINFO:
            debug("request REQUEST_SPELLINFO not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_HIGHSCORE:
            debug("request REQUEST_HIGHSCORE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_QUEST:
            debug("request REQUEST_QUEST not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_LEADER_XYR:
            debug("request REQUEST_LEADER_XYR not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_PRINT_NAME:
            debug("request REQUEST_PRINT_NAME not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_ISPLAYER:
            debug("request REQUEST_ISPLAYER not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SCROLLWHOLE:
            debug("request REQUEST_SCROLLWHOLE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_ISCHEATMODE:
            debug("request REQUEST_ISCHEATMODE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_ABORTENDGAME:
            debug("request REQUEST_ABORTENDGAME not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_DRAINLEVEL:
            debug("request REQUEST_DRAINLEVEL not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SECONDS:
            debug("request REQUEST_SECONDS not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_L_ANIMATION:
            debug("request REQUEST_L_ANIMATION not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SEARCHITEM:
            debug("request REQUEST_SEARCHITEM not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SPELLALLOW:
            debug("request REQUEST_SPELLALLOW not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_REGIONFILLED:
            debug("request REQUEST_REGIONFILLED not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_ABORTACTION:
            debug("request REQUEST_ABORTACTION not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_MONSTERCOUNT:
            debug("request REQUEST_MONSTERCOUNT not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_NOFLEE:
            debug("request REQUEST_NOFLEE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_ARRESTREASON:
            debug("request REQUEST_ARRESTREASON not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_CHECKNEW:
            debug("request REQUEST_CHECKNEW not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_IMPERSONATE:
            debug("request REQUEST_IMPERSONATE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_PASSTIME:
            debug("request REQUEST_PASSTIME not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_PROFILE:
            debug("request REQUEST_PROFILE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_MYSLOT:
            debug("request REQUEST_MYSLOT not implemented\n");
            command_implemented = 0;
            break;
        default:
            error("request %d not implemented\n", token);
    }
    return answer;
}

static int16_t request_to_do(int16_t name, int16_t rectype, int (*request_proc)(int16_t, long, long), long param1, long param2) {
    if (name < NULL_OBJECT) {
        if (rectype == DO_TO_ALL) {
            debug("Need to add LUA hooks to walk through every object and...\n");
            return request_proc(name, param1, param2);
        } else {
            return request_proc(name, param1, param2);
        }
    } 
    debug("request_to_do: object %d, request %d not implemented\n", name, rectype);
    command_implemented = 0;
    return 0;
}

static int req_animation(int16_t object, long notused1, long notused2) {
    dude_t *dude = sol_region_find_entity_by_id(sol_region_manager_get_current(), object);
    gpl_set_gname(GNAME_PASSIVE, object);

    if (dude) {
        dude->anim.pos++;
        while (! (dude->anim.scmd[dude->anim.pos].flags & SCMD_LAST) && dude->anim.pos < SCMD_MAX_SIZE) {
            dude->anim.pos++;
        }

        if (dude->anim.pos == SCMD_MAX_SIZE) { dude->anim.pos = 0; }
        port_entity_update_scmd(dude);
    } else {
        error("Unable to find object %d\n", object);
    }

    return 0;
}

static int req_set_allegiance(int16_t object, long allegiance, long notused2) {
    dude_t *dude = NULL;

    entity_list_for_each(sol_region_manager_get_current()->entities, dude) {
        if (dude->ds_id == object) {
            //printf("set_allegiance: %d to %d\n", object, allegiance);
            dude->allegiance = allegiance;
        }
    }

    return object;
}
