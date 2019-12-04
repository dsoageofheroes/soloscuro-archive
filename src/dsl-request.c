#include "dsl.h"

enum requests {
    REQUEST_NONE, //0
    HEALING, //1
    DOOR, //2
    THIEFSKILL, //3
    REST, //4
    ANIMATION, //5
    CINEMATIC, //6
    TRAP, //7
    REQUEST_MONSTER, //8
    SWAP, //9
    SET_BLOCK, //10
    CLEAR_BLOCK, //11
    SET_LOS, //12
    CLEAR_LOS, //13
    REQUEST_BATTLE_DEMO, //14
    SET_GAME_MOVE, //15
    BRANCH_MUSIC, //16
    FLASH_ANIMATION, //17
    SET_ALLEGIANCE, //18
    END_GAME, //19
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

int req_animation(int16_t object, long notused1, long notused2);
int req_set_allegiance(int16_t object, long allegiance, long notused2);
int16_t request_to_do(int16_t name, int16_t rectype, int (*request_proc)(int16_t, long, long), long param1, long
param2);

uint32_t dsl_request_impl(int16_t token, int16_t name,
        int32_t num1, int32_t num2) {
    int answer = 0;
    switch (token) {
        case HEALING:
            printf("request HEALING not implemented\n");
            command_implemented = 0;
            break;
        case DOOR:
            printf("request DOOR not implemented\n");
            command_implemented = 0;
            break;
        case THIEFSKILL:
            printf("request THIEFSKILL not implemented\n");
            command_implemented = 0;
            break;
        case REST:
            printf("request REST not implemented\n");
            command_implemented = 0;
            break;
        case ANIMATION:
            request_to_do(name, DO_TO_ALL, req_animation, num1, num2);
            break;
        case CINEMATIC:
            printf("request CINEMATIC not implemented\n");
            command_implemented = 0;
            break;
        case TRAP:
            printf("request TRAP not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_MONSTER:
            printf("request REQUEST_MONSTER not implemented\n");
            command_implemented = 0;
            break;
        case SWAP:
            if (name > 0) {
                printf("I need to swap to %d from disk id %d with flags %d\n", name, num1, num2);
            } else {
                printf("I need to swap to iObjectIx from disk id %d with flags %d\n", num1, num2);
            }
            break;
        case SET_BLOCK:
            printf("Need to set the bit flags for map position (%d, %d) to %d & commit!\n", num1, num2, GB_BLOCK);
            break;
        case CLEAR_BLOCK:
            printf("request CLEAR_BLOCK not implemented\n");
            command_implemented = 0;
            break;
        case SET_LOS:
            printf("request SET_LOS not implemented\n");
            command_implemented = 0;
            break;
        case CLEAR_LOS:
            printf("request CLEAR_LOS not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_BATTLE_DEMO:
            printf("request REQUEST_BATTLE_DEMO not implemented\n");
            command_implemented = 0;
            break;
        case SET_GAME_MOVE:
            printf("request SET_GAME_MOVE not implemented\n");
            command_implemented = 0;
            break;
        case BRANCH_MUSIC:
            switch(num2) {
                case 0:
                    printf("I need to stop music and play %d\n", num1);
                    break;
                case 1:
                    printf("I need to play music %d next\n", num1);
                    break;
                case 2:
                    printf("CASE 2 notimplemented\n");
                    command_implemented = 0;
                    break;
                case 3:
                    printf("I need to play music %d next\n", num1);
                    break;
                case 4:
                    printf("CASE 4 notimplemented\n");
                    command_implemented = 0;
                    break;
            }
            break;
        case FLASH_ANIMATION:
            printf("request FLASH_ANIMATION not implemented\n");
            command_implemented = 0;
            break;
        case SET_ALLEGIANCE:
            request_to_do(name, DO_COMBAT, req_set_allegiance, num1, num2);
            break;
        case END_GAME:
            printf("request END_GAME not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_COUNT_COMBAT:
            printf("request REQUEST_COUNT_COMBAT not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_DIFFICULT_LEVEL:
            printf("request REQUEST_DIFFICULT_LEVEL not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_END_TURN:
            printf("request REQUEST_END_TURN not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_APPLY_CONDITION:
            printf("request REQUEST_APPLY_CONDITION not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_DETECT_CONDITION:
            printf("request REQUEST_DETECT_CONDITION not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SURPRISE_ATTACK:
            printf("request REQUEST_SURPRISE_ATTACK not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_PALETTE_CHANGE:
            printf("request REQUEST_PALETTE_CHANGE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SHAKE:
            printf("request REQUEST_SHAKE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SET_EXCHANGE_RATE:
            printf("request REQUEST_SET_EXCHANGE_RATE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_PUSH:
            printf("request REQUEST_PUSH not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_POP:
            printf("request REQUEST_POP not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SET_DANGER_BIT:
            printf("request REQUEST_SET_DANGER_BIT not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_CLEAR_DANGER_BIT:
            printf("request REQUEST_CLEAR_DANGER_BIT not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SET_BORDER:
            printf("request REQUEST_SET_BORDER not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SET_OVERHEAD:
            printf("request REQUEST_SET_OVERHEAD not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SET_DEATH_SCREEN:
            printf("request REQUEST_SET_DEATH_SCREEN not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_REDRAW_SCREEN:
            printf("request REQUEST_REDRAW_SCREEN not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_FADE_SCREEN:
            printf("request REQUEST_FADE_SCREEN not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_CENTER_XY:
            printf("request REQUEST_CENTER_XY not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_INTERMISSION:
            printf("request REQUEST_INTERMISSION not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_REMOVE_CONDITION:
            printf("request REQUEST_REMOVE_CONDITION not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SAVE_VS:
            printf("request REQUEST_SAVE_VS not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SET_PLAYLIST:
            printf("request REQUEST_SET_PLAYLIST not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_CURRENT_PLAYLIST:
            printf("request REQUEST_CURRENT_PLAYLIST not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_DO_SPEECH:
            printf("request REQUEST_DO_SPEECH not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_OBJECT_ANIMATING:
            printf("request REQUEST_OBJECT_ANIMATING not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_CAST_SPELL:
            printf("request REQUEST_CAST_SPELL not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_PREPARE_SPELL:
            printf("request REQUEST_PREPARE_SPELL not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_GET_DATA:
            printf("request REQUEST_GET_DATA not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SET_DATA:
            printf("request REQUEST_SET_DATA not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_IS_CD:
            printf("request REQUEST_IS_CD not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_DEPLOY_PARTY:
            printf("request REQUEST_DEPLOY_PARTY not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SET_PAIN:
            printf("request REQUEST_SET_PAIN not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_KILL_BRAIN:
            printf("request REQUEST_KILL_BRAIN not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SET_SHARED_GLOBAL:
            printf("request REQUEST_SET_SHARED_GLOBAL not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SET_SHARED_REGION:
            printf("request REQUEST_SET_SHARED_REGION not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_GET_SHARED_GLOBAL:
            printf("request REQUEST_GET_SHARED_GLOBAL not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_GET_SHARED_REGION:
            printf("request REQUEST_GET_SHARED_REGION not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_GET_SEMAPHORE_GLOBAL:
            printf("request REQUEST_GET_SEMAPHORE_GLOBAL not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_CHECK_SEMAPHORE_GLOBAL:
            printf("request REQUEST_CHECK_SEMAPHORE_GLOBAL not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_RELEASE_SEMAPHORE_GLOBAL:
            printf("request REQUEST_RELEASE_SEMAPHORE_GLOBAL not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_GET_SEMAPHORE_REGION:
            printf("request REQUEST_GET_SEMAPHORE_REGION not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_CHECK_SEMAPHORE_REGION:
            printf("request REQUEST_CHECK_SEMAPHORE_REGION not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_RELEASE_SEMAPHORE_REGION:
            printf("request REQUEST_RELEASE_SEMAPHORE_REGION not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_GAMETICK:
            printf("request REQUEST_GAMETICK not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_TEXT_SCOPE:
            printf("request REQUEST_TEXT_SCOPE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SOUND_SCOPE:
            printf("request REQUEST_SOUND_SCOPE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_WHOISLEADER:
            printf("request REQUEST_WHOISLEADER not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_DATE:
            printf("request REQUEST_DATE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_TIME:
            printf("request REQUEST_TIME not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_PARTY_ID:
            printf("request REQUEST_PARTY_ID not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_NEAR_ID:
            printf("request REQUEST_NEAR_ID not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_INFO_SHORT_ID:
            printf("request REQUEST_INFO_SHORT_ID not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_INFO_LONG_ID:
            printf("request REQUEST_INFO_LONG_ID not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_CHAR_INFO:
            printf("request REQUEST_CHAR_INFO not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_CHAR_FROM:
            printf("request REQUEST_CHAR_FROM not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_CHAR_TO:
            printf("request REQUEST_CHAR_TO not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_MY_CHAR_FROM:
            printf("request REQUEST_MY_CHAR_FROM not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_MY_CHAR_TO:
            printf("request REQUEST_MY_CHAR_TO not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SELECTED_ID:
            printf("request REQUEST_SELECTED_ID not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_COMBAT_NUM:
            printf("request REQUEST_COMBAT_NUM not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_CLONE_NUM:
            printf("request REQUEST_CLONE_NUM not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_GET_CLONE:
            printf("request REQUEST_GET_CLONE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_GET_COMBAT_NUM:
            printf("request REQUEST_GET_COMBAT_NUM not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_DETACH:
            printf("request REQUEST_DETACH not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_R_LIFE:
            printf("request REQUEST_R_LIFE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_REGIONCLEAR:
            printf("request REQUEST_REGIONCLEAR not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_REGIONCOUNT:
            printf("request REQUEST_REGIONCOUNT not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_NAMEID:
            printf("request REQUEST_NAMEID not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SPELLINFO:
            printf("request REQUEST_SPELLINFO not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_HIGHSCORE:
            printf("request REQUEST_HIGHSCORE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_QUEST:
            printf("request REQUEST_QUEST not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_LEADER_XYR:
            printf("request REQUEST_LEADER_XYR not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_PRINT_NAME:
            printf("request REQUEST_PRINT_NAME not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_ISPLAYER:
            printf("request REQUEST_ISPLAYER not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SCROLLWHOLE:
            printf("request REQUEST_SCROLLWHOLE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_ISCHEATMODE:
            printf("request REQUEST_ISCHEATMODE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_ABORTENDGAME:
            printf("request REQUEST_ABORTENDGAME not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_DRAINLEVEL:
            printf("request REQUEST_DRAINLEVEL not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SECONDS:
            printf("request REQUEST_SECONDS not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_L_ANIMATION:
            printf("request REQUEST_L_ANIMATION not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SEARCHITEM:
            printf("request REQUEST_SEARCHITEM not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_SPELLALLOW:
            printf("request REQUEST_SPELLALLOW not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_REGIONFILLED:
            printf("request REQUEST_REGIONFILLED not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_ABORTACTION:
            printf("request REQUEST_ABORTACTION not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_MONSTERCOUNT:
            printf("request REQUEST_MONSTERCOUNT not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_NOFLEE:
            printf("request REQUEST_NOFLEE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_ARRESTREASON:
            printf("request REQUEST_ARRESTREASON not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_CHECKNEW:
            printf("request REQUEST_CHECKNEW not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_IMPERSONATE:
            printf("request REQUEST_IMPERSONATE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_PASSTIME:
            printf("request REQUEST_PASSTIME not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_PROFILE:
            printf("request REQUEST_PROFILE not implemented\n");
            command_implemented = 0;
            break;
        case REQUEST_MYSLOT:
            printf("request REQUEST_MYSLOT not implemented\n");
            command_implemented = 0;
            break;
    }
    return answer;
}

int16_t request_to_do(int16_t name, int16_t rectype, int (*request_proc)(int16_t, long, long), long param1, long param2)
{
    //if (name >= 0 && name < NULL_OBJECT) {
    if (name < NULL_OBJECT) {
        if (rectype == DO_TO_ALL) {
            printf("Need to add LUA hooks to walk through every object and...");
            request_proc(name, param1, param2);
            return 1;
        } else {
            request_proc(name, param1, param2);
            return 1;
        }
    } 
    printf("request_to_do: object %d, request %d not implemented\n", name, rectype);
    command_implemented = 0;
    return 0;
}

int req_animation(int16_t object, long notused1, long notused2) {
    printf("Need to request animation on %d\n", object);
    return 0;
}

int req_set_allegiance(int16_t object, long allegiance, long notused2) {
    printf("Need to set object %d allegiance to %ld\n", object, allegiance);
    return 0;
}
