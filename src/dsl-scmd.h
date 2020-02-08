// SCMD (Script CoMmanDs)
#ifndef DSL_SCMD_H
#define DSL_SCMD_H

typedef struct _scmd_t {
    uint8_t bmp_idx; // 0-254 = valid, 255 = none
    uint8_t delay;   // 0-255 = valid, ticks are 96 per second
    uint8_t flags;   // See SCMD flags
    int8_t xoffset; // change in x for new bmp
    int8_t yoffset; // change in x for new bmp
    int8_t xoffsethot; // change in x for new bmp
    int8_t yoffsethot; // change in x for new bmp
    uint8_t soundidx;  // sound index to play
} scmd_t;

// SCMD flags
#define SCMD_JUMP (0x01) // loop
#define SCMD_LAST (0x02) // end of script
#define SCMD_XMIRROR (0x04) // mirror x?
#define SCMD_YMIRROR (0x08) // mirror y?
#define SCMD_MOVING (0x10) // is moving?
#define SCMD_COMBAT (0x20) // load combat image.
#define SCMD_OK_HOT (0x40) // Don't change the hot
#define SCMD_MAX      (240)
#define SCMD_MAX_SIZE (36)
#define SCMD_DEFAULT_DELAY (16)
#define SCMD_TICKS_PER_SECOND (96)

void dsl_scmd_print(int gff_file, int res_id);
scmd_t* dsl_scmd_get(const int gff_file, const int res_id, const int index);
int dsl_scmd_is_default(const scmd_t *scmd, const int scmd_index);

#endif
