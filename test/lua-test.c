#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../src/dsl.h"
#include "../src/dsl-region.h"
#include "../src/dsl-manager.h"

static char *ds1_gffs = NULL;

static void parse_args(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "--ds1") && i < (argc-1)) {
            ds1_gffs = argv[++i];
        }
    }

    if (!ds1_gffs) {
        error("Unable to get the location of the DarkSun 1 GFFs, please pass with '--ds1 <location>'\n");
        exit(1);
    }
}

void tick() {
}

void handle_input() {
}

void render() {
}

int8_t narrate_open_sdl(int16_t action, char *text, int16_t index) {
    return 0;
}

int main(int argc, char *argv[]) {
    //const int region_id = 42;
    parse_args(argc, argv);
    gff_init();
    gff_load_directory(ds1_gffs);
    dsl_init();

    dsl_lua_execute_script(42, 0, 1);
    dsl_lua_execute_script(5, 931, 0);
    //dsl_load_region(region_id);
    //dsl_change_region(region_id);

    dsl_cleanup();
    gff_cleanup();
    return 0;
}
