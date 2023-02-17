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
    SOL_UNKNOWN_ERROR,
} sol_status_t;

extern void sol_status_print(sol_status_t error, const char *level, const char *msg);
extern void sol_status_exit(sol_status_t error, const char *msg);
extern void sol_status_check(sol_status_t status, const char *msg);
extern void sol_status_warn(sol_status_t status, const char *msg);

#endif

/*
 * Headers to look at next for update to status paradigm
combat.h
combat-region.h
gff-char.h
gff-common.h
gff.h
gff-image.h
gff-map.h
gfftypes.h
gff-xmi.h
port.h
replay.h
rules.h
save-load.h
settings.h
sol-lua-manager.h
sol-lua-settings.h
sol_textbox.h
ssi-gui.h
ssi-item.h
ssi-object.h
ssi-scmd.h
statics.h
stats.h
status.h
trigger.h
view-character.h
window-main.h
window-manager.h
wizard.h

Revisit:
entity.h
 */
