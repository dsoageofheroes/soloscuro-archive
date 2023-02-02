#include "alignment.h"
#include "gff.h"

// FIXME - Templars can only be "one step away" from their
//         patron in either axis, but not both
static const uint32_t class_alignments[] = {
//  REAL_CLASS_NONE
    ANY_ALIGNMENT,
//  Air Cleric            Earth Cleric          Fire Cleric           Water Cleric
    ANY_ALIGNMENT,        ANY_ALIGNMENT,        ANY_ALIGNMENT,        ANY_ALIGNMENT,    
//  Air Druid             Earth Druid           Fire Druid            Water Druid
    ONLY_TRUE_NEUTRAL,    ONLY_TRUE_NEUTRAL,    ONLY_TRUE_NEUTRAL,    ONLY_TRUE_NEUTRAL,
//  Fighter
    ANY_ALIGNMENT,
//  Gladiator
    ANY_ALIGNMENT,
//  Preserver
    NOT_EVIL_MORALS,
//  Psionicist
    ANY_ALIGNMENT,
//  Air Ranger            Earth Ranger          Fire Ranger           Water Ranger
    ONLY_GOOD_MORALS,     ONLY_GOOD_MORALS,     ONLY_GOOD_MORALS,     ONLY_GOOD_MORALS,
//  Thief
    NOT_LAWFUL_GOOD,
//  Defiler
    NOT_GOOD_MORALS,
//  Templar
    NOT_GOOD_MORALS,
};

static const uint32_t alignment_flags[] = {
    // Lawful Good
    ONLY_LAWFUL_ETHICS   | NOT_NEUTRAL_ETHICS      | NOT_CHAOTIC_ETHICS    |
    ONLY_GOOD_MORALS     | NOT_NEUTRAL_MORALS      | NOT_EVIL_MORALS       |
    ONLY_LAWFUL_GOOD     | NOT_NEUTRAL_GOOD        | NOT_CHAOTIC_GOOD      |
    NOT_LAWFUL_NEUTRAL   | NOT_TRUE_NEUTRAL        | NOT_CHAOTIC_NEUTRAL   |
    NOT_LAWFUL_EVIL      | NOT_NEUTRAL_EVIL        | NOT_CHAOTIC_EVIL,

    // Lawful Neutral
    ONLY_LAWFUL_ETHICS   | NOT_NEUTRAL_ETHICS      | NOT_CHAOTIC_ETHICS    |
    NOT_GOOD_MORALS      | ONLY_NEUTRAL_MORALS     | NOT_EVIL_MORALS       |
    NOT_LAWFUL_GOOD      | NOT_NEUTRAL_GOOD        | NOT_CHAOTIC_GOOD      |
    ONLY_LAWFUL_NEUTRAL  | NOT_TRUE_NEUTRAL        | NOT_CHAOTIC_NEUTRAL   |
    NOT_LAWFUL_EVIL      | NOT_NEUTRAL_EVIL        | NOT_CHAOTIC_EVIL,

    // Lawful Evil
    ONLY_LAWFUL_ETHICS   | NOT_NEUTRAL_ETHICS      | NOT_CHAOTIC_ETHICS    |
    NOT_GOOD_MORALS      | NOT_NEUTRAL_MORALS      | ONLY_EVIL_MORALS      |
    NOT_LAWFUL_GOOD      | NOT_NEUTRAL_GOOD        | NOT_CHAOTIC_GOOD      |
    NOT_LAWFUL_NEUTRAL   | NOT_TRUE_NEUTRAL        | NOT_CHAOTIC_NEUTRAL   |
    ONLY_LAWFUL_EVIL     | NOT_NEUTRAL_EVIL        | NOT_CHAOTIC_EVIL,

    // Neutral Good
    NOT_LAWFUL_ETHICS    | ONLY_NEUTRAL_ETHICS     | NOT_CHAOTIC_ETHICS    |
    ONLY_GOOD_MORALS     | NOT_NEUTRAL_MORALS      | NOT_EVIL_MORALS       |
    NOT_LAWFUL_GOOD      | ONLY_NEUTRAL_GOOD       | NOT_CHAOTIC_GOOD      |
    NOT_LAWFUL_NEUTRAL   | NOT_TRUE_NEUTRAL        | NOT_CHAOTIC_NEUTRAL   |
    NOT_LAWFUL_EVIL      | NOT_NEUTRAL_EVIL        | NOT_CHAOTIC_EVIL,

    // True Neutral
    NOT_LAWFUL_ETHICS    | ONLY_NEUTRAL_ETHICS     | NOT_CHAOTIC_ETHICS    |
    NOT_GOOD_MORALS      | ONLY_NEUTRAL_MORALS     | NOT_EVIL_MORALS       |
    NOT_LAWFUL_GOOD      | NOT_NEUTRAL_GOOD        | NOT_CHAOTIC_GOOD      |
    NOT_LAWFUL_NEUTRAL   | ONLY_TRUE_NEUTRAL       | NOT_CHAOTIC_NEUTRAL   |
    NOT_LAWFUL_EVIL      | NOT_NEUTRAL_EVIL        | NOT_CHAOTIC_EVIL,

    // Neutral Evil
    NOT_LAWFUL_ETHICS    | ONLY_NEUTRAL_ETHICS     | NOT_CHAOTIC_ETHICS    |
    NOT_GOOD_MORALS      | NOT_NEUTRAL_MORALS      | ONLY_EVIL_MORALS      |
    NOT_LAWFUL_GOOD      | NOT_NEUTRAL_GOOD        | NOT_CHAOTIC_GOOD      |
    NOT_LAWFUL_NEUTRAL   | NOT_TRUE_NEUTRAL        | NOT_CHAOTIC_NEUTRAL   |
    NOT_LAWFUL_EVIL      | ONLY_NEUTRAL_EVIL       | NOT_CHAOTIC_EVIL,

    // Chaotic Good
    NOT_LAWFUL_ETHICS    | NOT_NEUTRAL_ETHICS      | ONLY_CHAOTIC_ETHICS   |
    ONLY_GOOD_MORALS     | NOT_NEUTRAL_MORALS      | NOT_EVIL_MORALS       |
    NOT_LAWFUL_GOOD      | NOT_NEUTRAL_GOOD        | ONLY_CHAOTIC_GOOD     |
    NOT_LAWFUL_NEUTRAL   | NOT_TRUE_NEUTRAL        | NOT_CHAOTIC_NEUTRAL   |
    NOT_LAWFUL_EVIL      | NOT_NEUTRAL_EVIL        | NOT_CHAOTIC_EVIL,

    // Chaotic Neutral
    NOT_LAWFUL_ETHICS    | NOT_NEUTRAL_ETHICS      | ONLY_CHAOTIC_ETHICS   |
    NOT_GOOD_MORALS      | ONLY_NEUTRAL_MORALS     | NOT_EVIL_MORALS       |
    NOT_LAWFUL_GOOD      | NOT_NEUTRAL_GOOD        | NOT_CHAOTIC_GOOD      |
    NOT_LAWFUL_NEUTRAL   | NOT_TRUE_NEUTRAL        | ONLY_CHAOTIC_NEUTRAL  |
    NOT_LAWFUL_EVIL      | NOT_NEUTRAL_EVIL        | NOT_CHAOTIC_EVIL,

    // Chaotic Evil
    NOT_LAWFUL_ETHICS    | NOT_NEUTRAL_ETHICS      | ONLY_CHAOTIC_ETHICS   |
    NOT_GOOD_MORALS      | NOT_NEUTRAL_MORALS      | ONLY_EVIL_MORALS      |
    NOT_LAWFUL_GOOD      | NOT_NEUTRAL_GOOD        | NOT_CHAOTIC_GOOD      |
    NOT_LAWFUL_NEUTRAL   | NOT_TRUE_NEUTRAL        | NOT_CHAOTIC_NEUTRAL   |
    NOT_LAWFUL_EVIL      | NOT_NEUTRAL_EVIL        | ONLY_CHAOTIC_EVIL,
};

sol_status_t sol_dnd2e_alignment_allowed(const uint8_t alignment, const class_t classes[3], int target_is_pc) {
    int allowed = 1;
    int game_type = gff_get_game_type();

    for (int i = 0; i < 3; i++) {
        if (classes[i].class == -1) {
            break;
        }

        // Alignment has to match alignment flags for the class
        // NPCs can be any alignment - PCs can only be non-evil UNLESS the game is DSO
        allowed = (  class_alignments[classes[i].class] == ANY_ALIGNMENT ||
                     class_alignments[classes[i].class] & alignment_flags[alignment] ) &&
                   ( !target_is_pc || ( game_type != DARKSUN_ONLINE && alignment_flags[alignment] & NOT_EVIL_MORALS ) );

        if (!allowed) {
            return SOL_ILLEGAL_ALIGNMENT;
        }
    }

    return SOL_SUCCESS;
}
