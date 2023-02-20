/*
 * Stats capabilities for functionality and error codes
 */
#ifndef SOL_STATUS_H
#define SOL_STATUS_H

typedef enum sol_status_e {
    SOL_SUCCESS = 0, // AKA TRUE
    SOL_NOT_IMPLEMENTED,
    SOL_ILLEGAL_ALIGNMENT,
    SOL_NO_CUSTOM_ACTION,
    SOL_MEMORY_ERROR,
    SOL_NO_EXAMINE_TRIGGER,
    SOL_NULL_ARGUMENT,
    SOL_ILLEGAL_ARGUMENT,
    SOL_ILLEGAL_ATTACK,
    SOL_IN_COMBAT,
    SOL_NOT_IN_COMBAT,
    SOL_AUDIO_INIT_FAILURE,
    SOL_DNE,
    SOL_BUFFER_OVERRUN,
    SOL_DEVICE_OPEN_ERROR,
    SOL_AUDIO_QUEUE_ERROR,
    SOL_NOT_FOUND,
    SOL_ALREADY_FREED,
    SOL_NOT_LOADED,
    SOL_ILLEGAL_CLASS,
    SOL_ILLEGAL_LEVEL,
    SOL_NO_ATTACK,
    SOL_SPRITE_MAX_LIMIT,
    SOL_SPRITE_NOT_INITIALIZED,
    SOL_SPRITE_NOT_FOUND,
    SOL_OUT_OF_RANGE,
    SOL_SPRITE_NOT_IN_RECT,
    SOL_LABEL_NOT_FOUND,
    SOL_IS_REAL,
    SOL_GFF_ERROR,
    SOL_NOT_TURN,
    SOL_BLOCKED,
    SOL_WAIT_ACTIONS,
    SOL_NO_MOVES_LEFT,
    SOL_NO_CLASS,
    SOL_NO_SLOTS_LEFT,
    SOL_NOT_EMPTY,
    SOL_GFF_NO_ID,
    SOL_IS_CLOSED,
    SOL_STOPPED,
    SOL_GPL_EXECUTE_ERROR,
    SOL_GPL_FUNCTION_DNE,
    SOL_GFF_CHUNK_NOT_FOUND,
    SOL_ACTIVE,
    SOL_GPL_UNKNOWN_TYPE,
    SOL_UNKNOWN_KEY,
    SOL_ILLEGAL_SLOT,
    SOL_NOT_INITIALIZED,
    SOL_NO,
    SOL_GAME_OVER,
    SOL_PAUSED,
    SOL_ILLEGAL_STATS,
    SOL_NO_SEX,
    SOL_ILLEGAL_MAGIC_RESISTANCE,
    SOL_ILLEGAL_ALLEGIANCE,
    SOL_ILLEGAL_SIZE,
    SOL_ILLEGAL_PSIN,
    SOL_NO_MELEE_LEFT,
    SOL_IN_COMBAT_ERROR,
    SOL_IN_NARRATE_ERROR,
    SOL_FILESYSTEM_ERROR,
    SOL_LUA_NO_INSTANCE,
    SOL_LUA_NO_FUNCTION,
    SOL_NO_TARGET,
    SOL_NOT_HANDLED,
    SOL_UNKNOWN_ERROR,
} sol_status_t;

extern void sol_status_print(sol_status_t error, const char *level, const char *msg);
extern void sol_status_exit(sol_status_t error, const char *msg);
extern void sol_status_check(sol_status_t status, const char *msg);
extern void sol_status_warn(sol_status_t status, const char *msg);

#endif

/*
Revisit for updates:
port.h

These were left out so we could separate them.
gff-char.h
gff-common.h
gff.h
gff-image.h
gff-map.h
gfftypes.h
gff-xmi.h
ssi-gui.h
ssi-item.h
ssi-object.h
ssi-scmd.h

 */
