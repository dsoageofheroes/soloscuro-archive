// SCMD (Script CoMmanDs)
#ifndef SSI_SCMD_H
#define SSI_SCMD_H

typedef struct scmd_s {
    uint8_t bmp_idx; // 0-254 = valid, 255 = none
    uint8_t delay;   // 0-255 = valid, ticks are 96 per second
    uint8_t flags;   // See SCMD flags
    int8_t xoffset; // change in x for new bmp
    int8_t yoffset; // change in x for new bmp
    int8_t xoffsethot; // change in x for new bmp
    int8_t yoffsethot; // change in x for new bmp
    uint8_t soundidx;  // sound index to play
} scmd_t;

typedef struct disk_object_s {
    uint16_t flags;
    int16_t  xoffset;
    int16_t  yoffset;
    uint16_t xpos;
    uint16_t ypos;
    int8_t   zpos;
    uint8_t  object_index;
    uint16_t bmp_id;
    uint16_t script_id;
} disk_object_t;


// SCMD flags
#define SCMD_JUMP (0x01) // loop
#define SCMD_LAST (0x02) // end of script
#define SCMD_XMIRROR (0x04) // mirror x?
#define SCMD_YMIRROR (0x08) // mirror y?
#define SCMD_MOVING (0x10) // is moving?
#define SCMD_COMBAT (0x20) // load combat image.
#define SCMD_OK_HOT (0x40) // Don't change the hot
//#define SCMD_MAX      (256)
#define SCMD_MAX      (1<<10)
#define SCMD_MAX_SIZE (36)
#define SCMD_DEFAULT_DELAY (16)
#define SCMD_TICKS_PER_SECOND (96)

extern scmd_t* ssi_scmd_get(const int gff_idx, const int res_id, const int index);
extern scmd_t* ssi_scmd_empty();
extern int     ssi_scmd_is_default(const scmd_t *scmd, const int scmd_index);
extern int     ssi_scmd_total_delay(const scmd_t *scmd, int scmd_index);
extern int     ssi_scmd_next_pos(const scmd_t *scmd, const int scmd_index);

#endif
