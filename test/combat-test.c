#include "unity.h"
#include "gff.h"
#include "gff-char.h"
#include "rules.h"
#include "ds-load-save.h"
#include "player.h"
#include "region.h"
#include "region-manager.h"
#include <string.h>
#include <stdlib.h>

// A hack, but inclue base for default procedures (can be overwritten with #defs
#include "base.c"

void setUp() {
    gff_init();
    gff_load_directory("/home/pwest/dosbox/DARKSUN");
}

void tearDown() {
    gff_cleanup();
}

#define RES_MAX (1<<10)

void create_players() {
    dude_t *dude = player_get_entity(0);
    dude->stats.hp = 23;
    dude->stats.high_hp = 23;
    dude->stats.psp = 45;
    dude->ds_id = 1;
    dude->race = RACE_HUMAN;
    dude->gender = GENDER_MALE;
    dude->alignment = LAWFUL_GOOD;
    dude->stats.str = dude->stats.dex = dude->stats.con = dude->stats.intel = dude->stats.wis = dude->stats.cha = 10;
    dude->class[0].class = REAL_CLASS_FIGHTER;
    dude->class[0].level = 3;
    dude->stats.base_ac = 10;
    dude->stats.base_move = 12;
    dude->stats.attacks[0].num_dice = 1;
    dude->stats.attacks[0].sides = 1;
    dude->stats.attacks[0].number = 8;
    dude->stats.attacks[0].bonus = 1;
    dude->stats.saves.paralysis = 16;
    dude->stats.saves.wand = 12;
    dude->stats.saves.petrify = 13;
    dude->stats.saves.breath = 14;
    dude->stats.saves.spell = 15;
    dude->allegiance = 1;
    dude->name = strdup("Mel");
    dude->mapx = 35;
    dude->mapy = 20;
}

static entity_t *monsters[100];
static int num_monsters = 0;

static void create_basic_monsters(region_t *reg) {
    monsters[num_monsters] = entity_create_from_objex(-269); // Slig
    monsters[num_monsters]->mapx = 25;
    monsters[num_monsters]->mapy = 20;
    monsters[num_monsters]->abilities.hunt = 1;
    monsters[num_monsters]->allegiance = 2;
    entity_list_add(reg->cr.combatants, monsters[num_monsters]);
    num_monsters++;

    monsters[num_monsters] = entity_create_from_objex(-269); // Slig
    monsters[num_monsters]->mapx = 25;
    monsters[num_monsters]->mapy = 21;
    monsters[num_monsters]->abilities.hunt = 1;
    monsters[num_monsters]->allegiance = 2;
    entity_list_add(reg->cr.combatants, monsters[num_monsters]);
    num_monsters++;

    monsters[num_monsters] = entity_create_from_objex(-269); // Slig
    monsters[num_monsters]->mapx = 24;
    monsters[num_monsters]->mapy = 20;
    monsters[num_monsters]->abilities.hunt = 1;
    monsters[num_monsters]->allegiance = 2;
    entity_list_add(reg->cr.combatants, monsters[num_monsters]);
    num_monsters++;

    monsters[num_monsters] = entity_create_from_objex(-150); // Screamer Beatle
    monsters[num_monsters]->mapx = 24;
    monsters[num_monsters]->mapy = 20;
    monsters[num_monsters]->abilities.hunt = 1;
    monsters[num_monsters]->allegiance = 2;
    entity_list_add(reg->cr.combatants, monsters[num_monsters]);
    num_monsters++;
}

void test_basic(void) {
    srand(200);
    create_players();
    combat_action_t player_action;
    region_t *reg = region_manager_get_region(42);
    create_basic_monsters(reg);

    //printf("%d, %d\n", monsters[0]->mapx, monsters[0]->mapy);
    for (int i = 0; i < 6 * 30; i++) {
        TEST_ASSERT(NO_COMBAT == combat_player_turn());
        combat_update(reg);
    }
    //printf("%d, %d\n", monsters[0]->mapx, monsters[0]->mapy);
    for (int i = 0; i < 1 * 30; i++) {
        combat_update(reg);
    }
    //We should be in combat now.
    //printf("%d\n", combat_player_turn());
    TEST_ASSERT(NO_COMBAT != combat_player_turn()); 
    //printf("%d, %d\n", monsters[0]->mapx, monsters[0]->mapy);
    TEST_ASSERT(PLAYER1_TURN == combat_player_turn());
    player_action.action = CA_GUARD;
    combat_player_action(player_action);
    // Wait a round
    for (int i = 0; i < 1 * 30; i++) {
        combat_update(reg);
    }
    TEST_ASSERT(NONPLAYER_TURN == combat_player_turn());
    for (int i = 0; i < 200 * 30; i++) {
        //if (combat_player_turn() != NONPLAYER_TURN) {
            player_action.action = CA_GUARD;
            combat_player_action(player_action);
        //}
        combat_update(reg);
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_basic);
    return UNITY_END();
}
