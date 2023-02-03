/*
 * Stats capabilities for functionality and error codes
 */
#ifndef SOL_STATUS_H
#define SOL_STATUS_H

typedef enum sol_status_e {
    SOL_SUCCESS = 0, // AKA TRUE
    SOL_NOT_IMPLEMENTED,
    SOL_UNKNOWN_ERROR,
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
} sol_status_t;

extern void sol_status_print(sol_status_t error);
extern void sol_status_exit(sol_status_t error, const char *msg);
extern void sol_status_check(sol_status_t status, const char *msg);

#endif

/*
 * Headers to look at next for update to status paradigm
combat.h
combat-region.h
common.h
description.h
ds-load-save.h
effect.h
entity-animation.h
entity.h
entity-list.h
examine.h
font.h
gameloop.h
game-menu.h
gff-char.h
gff-common.h
gff.h
gff-image.h
gff-map.h
gfftypes.h
gff-xmi.h
gpl.h
gpl-lua.h
gpl-manager.h
gpl-state.h
gpl-string.h
gpl-var.h
innate.h
input.h
interact.h
inventory.h
item.h
label.h
lua-entity.h
lua-inc.h
lua-region.h
map.h
mouse.h
narrate.h
new-character.h
player.h
popup.h
port.h
portrait.h
powers.h
psionic.h
race.h
region.h
region-manager.h
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
 */
