#include "unity.h"
#include "../src/gff.h"
#include "../src/gff-char.h"
#include "../src/rules.h"
#include "../src/dsl.h"
#include "../src/ds-load-save.h"
#include "../src/ds-player.h"
#include "../src/ds-state.h"
#include "../src/ds-region.h"
#include "../src/ds-region-manager.h"
#include <string.h>
#include <stdlib.h>

// A hack, but inclue base for default procedures (can be overwritten with #defs
#include "base.c"

void setUp() {
    gff_init();
    gff_load_directory("/home/pwest/dosbox/DARKSUN");
    dsl_init();
}

void tearDown() {
    dsl_cleanup();
    gff_cleanup();
}

#define RES_MAX (1<<10)

void create_players() {
    ds_character_t pc;
    psin_t psi;
    spell_list_t spells;
    psionic_list_t psionics;
    ds_inventory_t inv;

    memset(&pc, 0x0, sizeof(ds_character_t));
    memset(&psi, 0x0, sizeof(psin_t));
    memset(&spells, 0x0, sizeof(spells));
    memset(&psionics, 0x0, sizeof(psionics));
    memset(&inv, 0x0, sizeof(ds_inventory_t));
    for (int i = 0; i < 26; i++) { ((ds1_item_t*)(&inv))[i].slot = i; };
    pc.current_xp = 1234;
    pc.base_hp = 23;
    pc.high_hp = 23;
    pc.base_psp = 45;
    pc.id = 1;
    pc.race = RACE_HUMAN;
    pc.gender = GENDER_MALE;
    pc.alignment = LAWFUL_GOOD;
    pc.stats.str = pc.stats.dex = pc.stats.con = pc.stats.intel = pc.stats.wis = pc.stats.cha = 10;
    pc.real_class[0] = REAL_CLASS_FIGHTER;
    pc.level[0] = 3;
    pc.base_ac = 10;
    pc.base_move = 12;
    pc.num_dice[0] = 1;
    pc.num_sides[0] = 1;
    pc.num_attacks[0] = 8;
    pc.num_bonuses[0] = 1;
    pc.saving_throw.paral = 16;
    pc.saving_throw.wand = 12;
    pc.saving_throw.petr = 13;
    pc.saving_throw.breath = 14;
    pc.saving_throw.spell = 15;
    pc.allegiance = 1;
    spell_set_psin(&psi, PSIONIC_PSYCHOKINETIC, 1);
    spell_set_psin(&psi, PSIONIC_PSYCHOMETABOLISM, 0);
    spell_set_psin(&psi, PSIONIC_TELEPATH, 0);
    // Probably should add some psionics
    // Probably should add some spells
    ds_item_load(&(inv.hand0), -1252); // long sword.
    ds_item_load(&(inv.bp[11]), -1252); // long sword.
    ds_player_replace(0, &pc, &psi, &spells, &psionics, &inv, "Garak");

    memset(&pc, 0x0, sizeof(ds_character_t));
    memset(&psi, 0x0, sizeof(psin_t));
    memset(&spells, 0x0, sizeof(spells));
    memset(&psionics, 0x0, sizeof(psionics));
    memset(&inv, 0x0, sizeof(ds_inventory_t));
    for (int i = 0; i < 26; i++) { ((ds1_item_t*)(&inv))[i].slot = i; };
    pc.current_xp = 4321;
    pc.base_hp = 32;
    pc.high_hp = 32;
    pc.base_psp = 54;
    pc.id = 2;
    pc.race = RACE_DWARF;
    pc.gender = GENDER_FEMALE;
    pc.alignment = LAWFUL_EVIL;
    pc.stats.str = pc.stats.dex = pc.stats.con = pc.stats.intel = pc.stats.wis = pc.stats.cha = 12;
    pc.real_class[0] = REAL_CLASS_GLADIATOR;
    pc.level[0] = 5;
    pc.base_ac = 9;
    pc.base_move = 12;
    pc.num_attacks[0] = 1;
    pc.num_dice[0] = 1;
    pc.num_sides[0] = 8;
    pc.num_bonuses[0] = 1;
    pc.saving_throw.paral = 10;
    pc.saving_throw.wand = 11;
    pc.saving_throw.petr = 12;
    pc.saving_throw.breath = 13;
    pc.saving_throw.spell = 14;
    pc.allegiance = 1;
    spell_set_psin(&psi, PSIONIC_PSYCHOKINETIC, 0);
    spell_set_psin(&psi, PSIONIC_PSYCHOMETABOLISM, 1);
    spell_set_psin(&psi, PSIONIC_TELEPATH, 0);
    // Probably should add some psionics
    // Probably should add some spells
    ds_player_replace(1, &pc, &psi, &spells, &psionics, &inv, "Dwarf");

    memset(&pc, 0x0, sizeof(ds_character_t));
    memset(&psi, 0x0, sizeof(psin_t));
    memset(&spells, 0x0, sizeof(spells));
    memset(&psionics, 0x0, sizeof(psionics));
    memset(&inv, 0x0, sizeof(ds_inventory_t));
    for (int i = 0; i < 26; i++) { ((ds1_item_t*)(&inv))[i].slot = i; };
    pc.current_xp = 10000;
    pc.base_hp = 32;
    pc.high_hp = 32;
    pc.base_psp = 54;
    pc.id = 3;
    pc.race = RACE_HALFLING;
    pc.gender = GENDER_FEMALE;
    pc.alignment = TRUE_NEUTRAL;
    pc.stats.str = pc.stats.dex = pc.stats.con = pc.stats.intel = pc.stats.wis = pc.stats.cha = 16;
    pc.real_class[0] = REAL_CLASS_PSIONICIST;
    pc.level[0] = 9;
    pc.base_ac = 9;
    pc.base_move = 12;
    pc.num_attacks[0] = 1;
    pc.num_dice[0] = 1;
    pc.num_sides[0] = 8;
    pc.num_bonuses[0] = 1;
    pc.saving_throw.paral = 10;
    pc.saving_throw.wand = 11;
    pc.saving_throw.petr = 12;
    pc.saving_throw.breath = 13;
    pc.saving_throw.spell = 14;
    pc.allegiance = 1;
    spell_set_psin(&psi, PSIONIC_PSYCHOKINETIC, 1);
    spell_set_psin(&psi, PSIONIC_PSYCHOMETABOLISM, 1);
    spell_set_psin(&psi, PSIONIC_TELEPATH, 1);
    // Probably should add some psionics
    spell_set_psionic(&psionics, PSIONIC_MENTAL_BARRIER);
    spell_set_psionic(&psionics, PSIONIC_ENHANCED_STRENGTH);
    spell_set_psionic(&psionics, PSIONIC_GRAFT_WEAPON);
    spell_set_psionic(&psionics, PSIONIC_CONTROL_BODY);
    spell_set_psionic(&psionics, PSIONIC_DETONATE);
    spell_set_psionic(&psionics, PSIONIC_DISINTEGRATE);
    // Probably should add some spells
    ds_player_replace(2, &pc, &psi, &spells, &psionics, &inv, "Mel");

    memset(&pc, 0x0, sizeof(ds_character_t));
    memset(&psi, 0x0, sizeof(psin_t));
    memset(&spells, 0x0, sizeof(spells));
    memset(&psionics, 0x0, sizeof(psionics));
    memset(&inv, 0x0, sizeof(ds_inventory_t));
    for (int i = 0; i < 26; i++) { ((ds1_item_t*)(&inv))[i].slot = i; };
    pc.current_xp = 10000;
    pc.base_hp = 32;
    pc.high_hp = 32;
    pc.base_psp = 54;
    pc.id = 3;
    pc.race = RACE_THRIKREEN;
    pc.gender = GENDER_FEMALE;
    pc.alignment = CHAOTIC_GOOD;
    pc.stats.str = pc.stats.dex = pc.stats.con = pc.stats.intel = pc.stats.wis = pc.stats.cha = 13;
    pc.real_class[0] = REAL_CLASS_PRESERVER;
    pc.level[0] = 9;
    pc.base_ac = 9;
    pc.base_move = 12;
    pc.num_attacks[0] = 1;
    pc.num_dice[0] = 1;
    pc.num_sides[0] = 8;
    pc.num_bonuses[0] = 1;
    pc.saving_throw.paral = 10;
    pc.saving_throw.wand = 11;
    pc.saving_throw.petr = 12;
    pc.saving_throw.breath = 13;
    pc.saving_throw.spell = 14;
    pc.allegiance = 1;
    spell_set_psin(&psi, PSIONIC_PSYCHOKINETIC, 0);
    spell_set_psin(&psi, PSIONIC_PSYCHOMETABOLISM, 0);
    spell_set_psin(&psi, PSIONIC_TELEPATH, 1);
    spell_set_spell(&spells, WIZ_ARMOR);
    spell_set_spell(&spells, WIZ_BURNING_HANDS);
    spell_set_spell(&spells, WIZ_CHARM_PERSON);
    spell_set_spell(&spells, WIZ_CHILL_TOUCH);
    spell_set_spell(&spells, WIZ_COLOR_SPRAY);
    spell_set_spell(&spells, WIZ_ENLARGE);
    spell_set_spell(&spells, WIZ_GAZE_REFLECTION);
    spell_set_spell(&spells, WIZ_GREASE);
    spell_set_spell(&spells, WIZ_MAGIC_MISSILE);
    spell_set_spell(&spells, WIZ_SHIELD);
    spell_set_spell(&spells, WIZ_SHOCKING_GRASP);
    spell_set_spell(&spells, WIZ_WALL_OF_FOG);
    //spell_set_spell(spells, WIZ_BLUR,
    spell_set_spell(&spells, WIZ_DETECT_INVISIBILITY);
    spell_set_spell(&spells, WIZ_FLAMING_SPHERE);
    spell_set_spell(&spells, WIZ_FOG_CLOUD);
    spell_set_spell(&spells, WIZ_GLITTERDUST);
    spell_set_spell(&spells, WIZ_INVISIBILITY);
    ds_player_replace(3, &pc, &psi, &spells, &psionics, &inv, "Edwin");

    ds_player_get_pos(0)->map = 42;
    ds_player_get_pos(0)->xpos = 30;
    ds_player_get_pos(0)->ypos = 30;
    ds_player_get_pos(0)->zpos = 0;
    ds_player_get_pos(1)->map = 42;
    ds_player_get_pos(1)->xpos = 31;
    ds_player_get_pos(1)->ypos = 30;
    ds_player_get_pos(2)->map = 42;
    ds_player_get_pos(2)->xpos = 31;
    ds_player_get_pos(2)->ypos = 31;
    ds_player_get_pos(3)->map = 42;
    ds_player_get_pos(3)->xpos = 30;
    ds_player_get_pos(3)->ypos = 29;

    ds_player_get_robj(0)->mapx = ds_player_get_pos(0)->xpos;
    ds_player_get_robj(0)->mapy = ds_player_get_pos(0)->ypos;
    ds_player_get_robj(1)->mapx = ds_player_get_pos(1)->xpos;
    ds_player_get_robj(1)->mapy = ds_player_get_pos(1)->ypos;
    ds_player_get_robj(2)->mapx = ds_player_get_pos(2)->xpos;
    ds_player_get_robj(2)->mapy = ds_player_get_pos(2)->ypos;
    ds_player_get_robj(3)->mapx = ds_player_get_pos(3)->xpos;
    ds_player_get_robj(3)->mapy = ds_player_get_pos(3)->ypos;
}

static region_object_t *monsters[100];
static int num_monsters = 0;

static void create_basic_monsters(dsl_region_t *reg) {
    int entry_id = dsl_region_create_from_objex(reg, -269, 25, 20); // Slig
    region_object_t* robj = dsl_region_get_object(entry_id);
    if (robj->scmd == NULL) { robj->scmd = ds_scmd_empty(); }
    monsters[num_monsters] = dsl_region_get_object(entry_id);
    combat_set_hunt(&(reg->cr), monsters[num_monsters]->combat_id);
    num_monsters++;

    entry_id = dsl_region_create_from_objex(reg, -269, 25, 20); // Slig
    robj = dsl_region_get_object(entry_id);
    if (robj->scmd == NULL) { robj->scmd = ds_scmd_empty(); }
    monsters[num_monsters] = dsl_region_get_object(entry_id);
    combat_set_hunt(&(reg->cr), monsters[num_monsters]->combat_id);
    num_monsters++;

    entry_id = dsl_region_create_from_objex(reg, -269, 25, 20); // Slig
    robj = dsl_region_get_object(entry_id);
    if (robj->scmd == NULL) { robj->scmd = ds_scmd_empty(); }
    monsters[num_monsters] = dsl_region_get_object(entry_id);
    combat_set_hunt(&(reg->cr), monsters[num_monsters]->combat_id);
    num_monsters++;

    entry_id = dsl_region_create_from_objex(reg, -150, 25, 20); // Screamer Beatle
    robj = dsl_region_get_object(entry_id);
    if (robj->scmd == NULL) { robj->scmd = ds_scmd_empty(); }
    monsters[num_monsters] = dsl_region_get_object(entry_id);
    combat_set_hunt(&(reg->cr), monsters[num_monsters]->combat_id);
    num_monsters++;
}

void test_basic(void) {
    srand(200);
    create_players();
    combat_action_t player_action;
    ds_region_t* reg = ds_region_load_region(42);
    create_basic_monsters(reg);

    printf("%d, %d\n", monsters[0]->mapx, monsters[0]->mapy);
    for (int i = 0; i < 5 * 30; i++) {
        TEST_ASSERT(NO_COMBAT == combat_player_turn());
        combat_update(reg);
    }
    for (int i = 0; i < 5 * 30; i++) {
        combat_update(reg);
    }
    //We should be in combat now.
    TEST_ASSERT(NO_COMBAT != combat_player_turn()); 
    printf("%d, %d\n", monsters[0]->mapx, monsters[0]->mapy);
    TEST_ASSERT(PLAYER3_TURN == combat_player_turn());
    // Wait a round
    for (int i = 0; i < 1 * 30; i++) {
        combat_update(reg);
    }

    player_action.action = CA_GUARD;
    combat_player_action(player_action);
    // Wait a round
    for (int i = 0; i < 1 * 30; i++) {
        combat_update(reg);
    }
    TEST_ASSERT(NONPLAYER_TURN == combat_player_turn());
    for (int i = 0; i < 7 * 30; i++) {
        combat_update(reg);
        TEST_ASSERT(NONPLAYER_TURN == combat_player_turn());
    }
    //Attack
    for (int i = 0; i < 1 * 30; i++) {
        combat_update(reg);
    }
    for (int i = 0; i < 1 * 30; i++) {
        combat_update(reg);
    }
    TEST_ASSERT(PLAYER2_TURN == combat_player_turn());
    player_action.action = CA_GUARD;
    combat_player_action(player_action);
    for (int i = 0; i < 200 * 30; i++) {
        //if (combat_player_turn() != NONPLAYER_TURN) {
            player_action.action = CA_GUARD;
            combat_player_action(player_action);
        //}
        combat_update(reg);
    }
    /*
    */
    //void combat_set_hunt(combat_region_t *cr, const uint32_t combat_id);
    printf("turn: %d\n", combat_player_turn());
    TEST_ASSERT(NO_COMBAT == combat_player_turn());

    //TEST_ASSERT_EQUAL_INT(0, ds_player_get_pos(0)->zpos);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_basic);
    return UNITY_END();
}
