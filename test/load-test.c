#include "unity.h"
#include "../src/gff.h"
#include "../src/gff-char.h"
#include "../src/rules.h"
#include "../src/dsl.h"
#include "../src/ds-load-save.h"
#include "../src/ds-player.h"
#include <string.h>
#include <stdlib.h>

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
    pc.current_xp = 1234;
    pc.base_hp = 23;
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
    ds_player_replace(0, &pc, &psi, &spells, &psionics, &inv, "Garak");

    memset(&pc, 0x0, sizeof(ds_character_t));
    memset(&psi, 0x0, sizeof(psin_t));
    memset(&spells, 0x0, sizeof(spells));
    memset(&psionics, 0x0, sizeof(psionics));
    memset(&inv, 0x0, sizeof(ds_inventory_t));
    pc.current_xp = 4321;
    pc.base_hp = 32;
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
    pc.current_xp = 10000;
    pc.base_hp = 32;
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
    pc.current_xp = 10000;
    pc.base_hp = 32;
    pc.base_psp = 54;
    pc.id = 3;
    pc.race = RACE_TRIKEEN;
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
    ds_player_get_pos(0)->xpos = 25;
    ds_player_get_pos(0)->ypos = 12;
    ds_player_get_pos(0)->zpos = 0;
}

void test_load_save_char(void) {
    char cmd[1<<10];
    create_players();
    char* path = ls_create_save_file();

    ds_player_init(); // clear all the players

    TEST_ASSERT_EQUAL_STRING("",  ds_player_get_combat(0)->name); // make sure it was cleared.
    ls_load_save_file(path);
    TEST_ASSERT_EQUAL_STRING("Garak",  ds_player_get_combat(0)->name); // make sure it was cleared.
    TEST_ASSERT_EQUAL_INT(1234, ds_player_get_char(0)->current_xp);
    TEST_ASSERT_EQUAL_INT(23, ds_player_get_char(0)->base_hp);
    TEST_ASSERT_EQUAL_INT(45, ds_player_get_char(0)->base_psp);
    TEST_ASSERT_EQUAL_INT(1, ds_player_get_char(0)->id);
    TEST_ASSERT_EQUAL_INT(RACE_HUMAN, ds_player_get_char(0)->race);
    TEST_ASSERT_EQUAL_INT(GENDER_MALE, ds_player_get_char(0)->gender);
    TEST_ASSERT_EQUAL_INT(LAWFUL_GOOD, ds_player_get_char(0)->alignment);
    TEST_ASSERT_EQUAL_INT(10, ds_player_get_char(0)->stats.str);
    TEST_ASSERT_EQUAL_INT(REAL_CLASS_FIGHTER, ds_player_get_char(0)->real_class[0]);
    TEST_ASSERT_EQUAL_INT(3, ds_player_get_char(0)->level[0]);
    TEST_ASSERT_EQUAL_INT(10, ds_player_get_char(0)->base_ac);
    TEST_ASSERT_EQUAL_INT(12, ds_player_get_char(0)->base_move);
    TEST_ASSERT_EQUAL_INT(1, ds_player_get_char(0)->num_dice[0]);
    TEST_ASSERT_EQUAL_INT(1, ds_player_get_char(0)->num_sides[0]);
    TEST_ASSERT_EQUAL_INT(8, ds_player_get_char(0)->num_attacks[0]);
    TEST_ASSERT_EQUAL_INT(1, ds_player_get_char(0)->num_bonuses[0]);
    TEST_ASSERT_EQUAL_INT(16, ds_player_get_char(0)->saving_throw.paral );
    TEST_ASSERT_EQUAL_INT(12, ds_player_get_char(0)->saving_throw.wand );
    TEST_ASSERT_EQUAL_INT(13, ds_player_get_char(0)->saving_throw.petr );
    TEST_ASSERT_EQUAL_INT(14, ds_player_get_char(0)->saving_throw.breath );
    TEST_ASSERT_EQUAL_INT(15, ds_player_get_char(0)->saving_throw.spell );
    TEST_ASSERT_EQUAL_INT(1, ds_player_get_char(0)->allegiance);
    TEST_ASSERT_EQUAL_INT(1, spell_has_psin(ds_player_get_psi(0), PSIONIC_PSYCHOKINETIC));
    TEST_ASSERT_EQUAL_INT(0, spell_has_psin(ds_player_get_psi(0), PSIONIC_PSYCHOMETABOLISM));
    TEST_ASSERT_EQUAL_INT(0, spell_has_psin(ds_player_get_psi(0), PSIONIC_TELEPATH));
    // Probably should add some psionics
    // Probably should add some spells

    TEST_ASSERT_EQUAL_STRING("Dwarf",  ds_player_get_combat(1)->name); // make sure it was cleared.
    TEST_ASSERT_EQUAL_INT(4321, ds_player_get_char(1)->current_xp);
    TEST_ASSERT_EQUAL_INT(32, ds_player_get_char(1)->base_hp);
    TEST_ASSERT_EQUAL_INT(54, ds_player_get_char(1)->base_psp);
    TEST_ASSERT_EQUAL_INT(2, ds_player_get_char(1)->id);
    TEST_ASSERT_EQUAL_INT(RACE_DWARF, ds_player_get_char(1)->race);
    TEST_ASSERT_EQUAL_INT(GENDER_FEMALE, ds_player_get_char(1)->gender);
    TEST_ASSERT_EQUAL_INT(LAWFUL_EVIL, ds_player_get_char(1)->alignment);
    TEST_ASSERT_EQUAL_INT(12, ds_player_get_char(1)->stats.str);
    TEST_ASSERT_EQUAL_INT(REAL_CLASS_GLADIATOR, ds_player_get_char(1)->real_class[0]);
    TEST_ASSERT_EQUAL_INT(5, ds_player_get_char(1)->level[0]);
    TEST_ASSERT_EQUAL_INT(9, ds_player_get_char(1)->base_ac);
    TEST_ASSERT_EQUAL_INT(12, ds_player_get_char(1)->base_move);
    TEST_ASSERT_EQUAL_INT(1, ds_player_get_char(1)->num_dice[0]);
    TEST_ASSERT_EQUAL_INT(8, ds_player_get_char(1)->num_sides[0]);
    TEST_ASSERT_EQUAL_INT(1, ds_player_get_char(1)->num_attacks[0]);
    TEST_ASSERT_EQUAL_INT(1, ds_player_get_char(1)->num_bonuses[0]);
    TEST_ASSERT_EQUAL_INT(10, ds_player_get_char(1)->saving_throw.paral );
    TEST_ASSERT_EQUAL_INT(11, ds_player_get_char(1)->saving_throw.wand );
    TEST_ASSERT_EQUAL_INT(12, ds_player_get_char(1)->saving_throw.petr );
    TEST_ASSERT_EQUAL_INT(13, ds_player_get_char(1)->saving_throw.breath );
    TEST_ASSERT_EQUAL_INT(14, ds_player_get_char(1)->saving_throw.spell );
    TEST_ASSERT_EQUAL_INT(1, ds_player_get_char(1)->allegiance);
    TEST_ASSERT_EQUAL_INT(0, spell_has_psin(ds_player_get_psi(1), PSIONIC_PSYCHOKINETIC));
    TEST_ASSERT_EQUAL_INT(1, spell_has_psin(ds_player_get_psi(1), PSIONIC_PSYCHOMETABOLISM));
    TEST_ASSERT_EQUAL_INT(0, spell_has_psin(ds_player_get_psi(1), PSIONIC_TELEPATH));
    // Probably should add some psionics
    // Probably should add some spells
    TEST_ASSERT_EQUAL_STRING("Mel",  ds_player_get_combat(2)->name); // make sure it was cleared.
    TEST_ASSERT_EQUAL_INT(10000, ds_player_get_char(2)->current_xp);
    TEST_ASSERT_EQUAL_INT(32, ds_player_get_char(2)->base_hp);
    TEST_ASSERT_EQUAL_INT(54, ds_player_get_char(2)->base_psp);
    TEST_ASSERT_EQUAL_INT(3, ds_player_get_char(2)->id);
    TEST_ASSERT_EQUAL_INT(RACE_HALFLING, ds_player_get_char(2)->race);
    TEST_ASSERT_EQUAL_INT(GENDER_FEMALE, ds_player_get_char(2)->gender);
    TEST_ASSERT_EQUAL_INT(TRUE_NEUTRAL, ds_player_get_char(2)->alignment);
    TEST_ASSERT_EQUAL_INT(16, ds_player_get_char(2)->stats.str);
    TEST_ASSERT_EQUAL_INT(REAL_CLASS_PSIONICIST, ds_player_get_char(2)->real_class[0]);
    TEST_ASSERT_EQUAL_INT(9, ds_player_get_char(2)->level[0]);
    TEST_ASSERT_EQUAL_INT(9, ds_player_get_char(2)->base_ac);
    TEST_ASSERT_EQUAL_INT(12, ds_player_get_char(2)->base_move);
    TEST_ASSERT_EQUAL_INT(1, ds_player_get_char(2)->num_dice[0]);
    TEST_ASSERT_EQUAL_INT(8, ds_player_get_char(2)->num_sides[0]);
    TEST_ASSERT_EQUAL_INT(1, ds_player_get_char(2)->num_attacks[0]);
    TEST_ASSERT_EQUAL_INT(1, ds_player_get_char(2)->num_bonuses[0]);
    TEST_ASSERT_EQUAL_INT(10, ds_player_get_char(2)->saving_throw.paral );
    TEST_ASSERT_EQUAL_INT(11, ds_player_get_char(2)->saving_throw.wand );
    TEST_ASSERT_EQUAL_INT(12, ds_player_get_char(2)->saving_throw.petr );
    TEST_ASSERT_EQUAL_INT(13, ds_player_get_char(2)->saving_throw.breath );
    TEST_ASSERT_EQUAL_INT(14, ds_player_get_char(2)->saving_throw.spell );
    TEST_ASSERT_EQUAL_INT(1, ds_player_get_char(2)->allegiance);
    TEST_ASSERT_EQUAL_INT(1, spell_has_psin(ds_player_get_psi(2), PSIONIC_PSYCHOKINETIC));
    TEST_ASSERT_EQUAL_INT(1, spell_has_psin(ds_player_get_psi(2), PSIONIC_PSYCHOMETABOLISM));
    TEST_ASSERT_EQUAL_INT(1, spell_has_psin(ds_player_get_psi(2), PSIONIC_TELEPATH));
    TEST_ASSERT_EQUAL_INT(1, spell_has_psionic(ds_player_get_psionics(2), PSIONIC_MENTAL_BARRIER));
    TEST_ASSERT_EQUAL_INT(1, spell_has_psionic(ds_player_get_psionics(2), PSIONIC_ENHANCED_STRENGTH));
    TEST_ASSERT_EQUAL_INT(1, spell_has_psionic(ds_player_get_psionics(2), PSIONIC_GRAFT_WEAPON));
    TEST_ASSERT_EQUAL_INT(1, spell_has_psionic(ds_player_get_psionics(2), PSIONIC_CONTROL_BODY));
    TEST_ASSERT_EQUAL_INT(1, spell_has_psionic(ds_player_get_psionics(2), PSIONIC_DETONATE));
    TEST_ASSERT_EQUAL_INT(1, spell_has_psionic(ds_player_get_psionics(2), PSIONIC_DISINTEGRATE));
    TEST_ASSERT_EQUAL_INT(0, spell_has_psionic(ds_player_get_psionics(2), PSIONIC_MIND_BLANK));
    // Probably should add some spells
    TEST_ASSERT_EQUAL_STRING("Edwin",  ds_player_get_combat(3)->name); // make sure it was cleared.
    TEST_ASSERT_EQUAL_INT(10000, ds_player_get_char(3)->current_xp);
    TEST_ASSERT_EQUAL_INT(32, ds_player_get_char(3)->base_hp);
    TEST_ASSERT_EQUAL_INT(54, ds_player_get_char(3)->base_psp);
    TEST_ASSERT_EQUAL_INT(3, ds_player_get_char(3)->id);
    TEST_ASSERT_EQUAL_INT(RACE_TRIKEEN, ds_player_get_char(3)->race);
    TEST_ASSERT_EQUAL_INT(GENDER_FEMALE, ds_player_get_char(3)->gender);
    TEST_ASSERT_EQUAL_INT(CHAOTIC_GOOD, ds_player_get_char(3)->alignment);
    TEST_ASSERT_EQUAL_INT(13, ds_player_get_char(3)->stats.str);
    TEST_ASSERT_EQUAL_INT(REAL_CLASS_PRESERVER, ds_player_get_char(3)->real_class[0]);
    TEST_ASSERT_EQUAL_INT(9, ds_player_get_char(3)->level[0]);
    TEST_ASSERT_EQUAL_INT(9, ds_player_get_char(3)->base_ac);
    TEST_ASSERT_EQUAL_INT(12, ds_player_get_char(3)->base_move);
    TEST_ASSERT_EQUAL_INT(1, ds_player_get_char(3)->num_dice[0]);
    TEST_ASSERT_EQUAL_INT(8, ds_player_get_char(3)->num_sides[0]);
    TEST_ASSERT_EQUAL_INT(1, ds_player_get_char(3)->num_attacks[0]);
    TEST_ASSERT_EQUAL_INT(1, ds_player_get_char(3)->num_bonuses[0]);
    TEST_ASSERT_EQUAL_INT(10, ds_player_get_char(3)->saving_throw.paral );
    TEST_ASSERT_EQUAL_INT(11, ds_player_get_char(3)->saving_throw.wand );
    TEST_ASSERT_EQUAL_INT(12, ds_player_get_char(3)->saving_throw.petr );
    TEST_ASSERT_EQUAL_INT(13, ds_player_get_char(3)->saving_throw.breath );
    TEST_ASSERT_EQUAL_INT(14, ds_player_get_char(3)->saving_throw.spell );
    TEST_ASSERT_EQUAL_INT(1, ds_player_get_char(3)->allegiance);
    TEST_ASSERT_EQUAL_INT(0, spell_has_psin(ds_player_get_psi(3), PSIONIC_PSYCHOKINETIC));
    TEST_ASSERT_EQUAL_INT(0, spell_has_psin(ds_player_get_psi(3), PSIONIC_PSYCHOMETABOLISM));
    TEST_ASSERT_EQUAL_INT(1, spell_has_psin(ds_player_get_psi(3), PSIONIC_TELEPATH));
    TEST_ASSERT_EQUAL_INT(0, spell_has_psionic(ds_player_get_psionics(3), PSIONIC_MENTAL_BARRIER));
    TEST_ASSERT_EQUAL_INT(0, spell_has_psionic(ds_player_get_psionics(3), PSIONIC_ENHANCED_STRENGTH));
    TEST_ASSERT_EQUAL_INT(0, spell_has_psionic(ds_player_get_psionics(3), PSIONIC_GRAFT_WEAPON));
    TEST_ASSERT_EQUAL_INT(0, spell_has_psionic(ds_player_get_psionics(3), PSIONIC_CONTROL_BODY));
    TEST_ASSERT_EQUAL_INT(0, spell_has_psionic(ds_player_get_psionics(3), PSIONIC_DETONATE));
    TEST_ASSERT_EQUAL_INT(0, spell_has_psionic(ds_player_get_psionics(3), PSIONIC_DISINTEGRATE));
    TEST_ASSERT_EQUAL_INT(0, spell_has_psionic(ds_player_get_psionics(3), PSIONIC_MIND_BLANK));
    //spell_has_spell(spell_list_t *psi, uint16_t spell);
    // Probably should add some spells
    TEST_ASSERT_EQUAL_INT(1, spell_has_spell(ds_player_get_spells(3), WIZ_ARMOR));
    TEST_ASSERT_EQUAL_INT(1, spell_has_spell(ds_player_get_spells(3), WIZ_BURNING_HANDS));
    TEST_ASSERT_EQUAL_INT(1, spell_has_spell(ds_player_get_spells(3), WIZ_CHARM_PERSON));
    TEST_ASSERT_EQUAL_INT(1, spell_has_spell(ds_player_get_spells(3), WIZ_CHILL_TOUCH));
    TEST_ASSERT_EQUAL_INT(1, spell_has_spell(ds_player_get_spells(3), WIZ_COLOR_SPRAY));
    TEST_ASSERT_EQUAL_INT(1, spell_has_spell(ds_player_get_spells(3), WIZ_ENLARGE));
    TEST_ASSERT_EQUAL_INT(1, spell_has_spell(ds_player_get_spells(3), WIZ_GAZE_REFLECTION));
    TEST_ASSERT_EQUAL_INT(1, spell_has_spell(ds_player_get_spells(3), WIZ_GREASE));
    TEST_ASSERT_EQUAL_INT(1, spell_has_spell(ds_player_get_spells(3), WIZ_MAGIC_MISSILE));
    TEST_ASSERT_EQUAL_INT(1, spell_has_spell(ds_player_get_spells(3), WIZ_SHIELD));
    TEST_ASSERT_EQUAL_INT(1, spell_has_spell(ds_player_get_spells(3), WIZ_SHOCKING_GRASP));
    TEST_ASSERT_EQUAL_INT(1, spell_has_spell(ds_player_get_spells(3), WIZ_WALL_OF_FOG));
    TEST_ASSERT_EQUAL_INT(0, spell_has_spell(ds_player_get_spells(3), WIZ_BLUR));
    TEST_ASSERT_EQUAL_INT(1, spell_has_spell(ds_player_get_spells(3), WIZ_DETECT_INVISIBILITY));
    TEST_ASSERT_EQUAL_INT(1, spell_has_spell(ds_player_get_spells(3), WIZ_FLAMING_SPHERE));
    TEST_ASSERT_EQUAL_INT(1, spell_has_spell(ds_player_get_spells(3), WIZ_FOG_CLOUD));
    TEST_ASSERT_EQUAL_INT(1, spell_has_spell(ds_player_get_spells(3), WIZ_GLITTERDUST));
    TEST_ASSERT_EQUAL_INT(1, spell_has_spell(ds_player_get_spells(3), WIZ_INVISIBILITY));
    TEST_ASSERT_EQUAL_INT(0, spell_has_spell(ds_player_get_spells(3), WIZ_FIREBALL));

    TEST_ASSERT_EQUAL_INT(42, ds_player_get_pos(0)->map);
    TEST_ASSERT_EQUAL_INT(25, ds_player_get_pos(0)->xpos);
    TEST_ASSERT_EQUAL_INT(12, ds_player_get_pos(0)->ypos);
    TEST_ASSERT_EQUAL_INT(0, ds_player_get_pos(0)->zpos);

    snprintf(cmd, 1<<10, "rm %s", path);
    //cmd[0] = '\0';
    system(cmd);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_load_save_char);
    return UNITY_END();
}
