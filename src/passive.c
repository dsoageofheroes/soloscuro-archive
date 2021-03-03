#include "passive.h"
#include <string.h>
#include <stdio.h>

void passive_load_from_etab(passive_t *passive, gff_map_object_t *entry_table, uint32_t id) {
    const gff_map_object_t *gm = entry_table + id;
    disk_object_t disk_object;

    memset(passive, 0x0, sizeof(passive_t));
    gff_read_object(gm->index, &disk_object);

    //Not read from gm: index(disk_idx), object_index
    passive->flags = disk_object.flags;
    passive->bmp_id = disk_object.bmp_id;
    passive->bmpx = gm->xpos - disk_object.xoffset;
    passive->bmpy = gm->ypos - disk_object.yoffset - disk_object.zpos;
    passive->xoffset = disk_object.xoffset;
    passive->yoffset = disk_object.yoffset;
    passive->mapx = (gm->xpos + disk_object.xoffset) / 16;
    passive->mapy = (gm->ypos + disk_object.yoffset - disk_object.zpos) / 16;
    passive->mapz = gm->zpos;
    passive->entry_id = id;
    passive->scmd_flags = gm->flags;
}
