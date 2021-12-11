// The arbiter decides what hits and moves.
// The idea is the network version will query the server, while the local version does the calucaltions.
#include "arbiter.h"

extern int sol_arbiter_hits(entity_animation_node_t *ea) {
    if (!ea) { return 0; }
    return 1;
}
