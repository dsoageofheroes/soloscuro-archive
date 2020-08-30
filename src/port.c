#include <string.h>
#include <stdlib.h>
#include "dsl.h"
#include "gameloop.h"
#include "port.h"
#include "replay.h"

static char *ds1_gffs = NULL;
static char *replay = NULL;

void parse_args(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "--ds1") && i < (argc-1)) {
            ds1_gffs = argv[++i];
        }
        if (!strcmp(argv[i], "--replay") && i < (argc-1)) {
            replay = argv[++i];
        }
    }

    if (!ds1_gffs) {
        error("Unable to get the location of the DarkSun 1 GFFs, please pass with '--ds1 <location>'\n");
        exit(1);
    }
    replay_init("replay.lua");
}

int pmain(int argc, char *argv[]) {
    parse_args(argc, argv);

    // Order matters.
    gff_init();
    gff_load_directory(ds1_gffs);
    dsl_init();

    port_init(argc, argv);

    if (replay) {
        replay_game(replay);
    }

    game_loop();

    port_cleanup();
    replay_cleanup();

    return 0;
}
