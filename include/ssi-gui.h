#ifndef SSI_GUI_H
#define SSI_GUI_H

#include "port.h"
#include <stdint.h>

typedef struct ssi_resource_header_s {
    uint32_t type;
    uint32_t len;
    uint32_t id;
} ssi_resource_header_t;

typedef struct ssi_rect_s {
    int16_t xmin, ymin;
    int16_t xmax, ymax;
} ssi_rect_t;

typedef struct ssi_gui_item_s {
    int8_t data[4];
    uint32_t type;
    uint32_t id;
    ssi_rect_t init_bounds;
    ssi_rect_t item_bounds;
    uint16_t flags;
} __attribute__ ((__packed__)) ssi_gui_item_t;

typedef struct ssi_region_s {
    uint16_t depth;
    ssi_rect_t bounds;
    ssi_rect_t regions[16];
} ssi_region_t;

typedef struct ssi_frame_s {
    uint16_t flags;
    uint16_t type;
    ssi_rect_t bounds;
    ssi_rect_t initbounds;
    ssi_rect_t zonebounds;
    uint16_t width;
    uint16_t height;
    uint32_t border_bmp;
    int8_t data[4]; // This may be two bytes.
    uint32_t background_bmp;
    int8_t data1[4]; // This may be two bytes.
    char title[24];
} __attribute__ ((__packed__)) ssi_frame_t;

typedef struct ssi_window_s {
    ssi_resource_header_t rh;
    ssi_region_t region;
    int16_t x;
    int16_t y;
    int8_t data[4];
    uint16_t flags;
    short data1;
    ssi_frame_t  frame;
    int16_t offsetx;
    int16_t offsety;
    int8_t data2[4];
    uint8_t titleLen;
    uint16_t itemCount;
    uint32_t mem; // total mem needed
} __attribute__ ((__packed__)) ssi_window_t;

typedef struct ssi_app_frame_s {
    ssi_resource_header_t rh;
    ssi_frame_t frame;
    uint8_t data[4];
    int16_t event_filter;
    uint8_t data1[4];
    uint8_t data2[4];
    uint8_t data3[4];
    int16_t snapMode;
    int16_t snapOriginX;
    int16_t snapOriginY;
    int16_t snapSizeX;
    int16_t snapSizeY;
    uint8_t data4[4];
} __attribute__ ((__packed__)) ssi_app_frame_t;

typedef struct ssi_button_s {
    ssi_resource_header_t rh;
    ssi_frame_t frame;
    uint8_t pad0[4];
    uint16_t flags;
    uint16_t userid;
    int16_t  iconx, icony;
    int16_t  textx, texty;
    uint32_t icon_id;
    uint8_t pad1[4];
    uint8_t key;
    uint8_t textlen;
} ssi_button_t;

typedef struct ssi_ebox_s {
    ssi_resource_header_t rh;
    uint8_t   pad0[4];
    uint16_t  maxlines, styles, runs, bufsize, user_id;
    ssi_frame_t frame;
} __attribute__ ((__packed__)) ssi_ebox_t;

typedef struct sol_button_s {
    uint32_t     ssi_id;
    uint32_t     icon_id;
    uint16_t     base_width, base_height;
    uint16_t     offsetx, offsety;
    char        *text;
    sol_sprite_t spr;
} sol_button_t;

typedef struct sol_frame_s {
    uint32_t     ssi_id;
    uint32_t     bmp_id;
    uint16_t     base_width, base_height;
    uint16_t     offsetx, offsety;
    uint16_t     event;
    sol_sprite_t spr;
} sol_frame_t;

typedef struct sol_box_s {
    uint32_t     ssi_id;
    uint32_t     bmp_id;
    uint16_t     base_width, base_height;
    uint16_t     offsetx, offsety;
    sol_sprite_t spr;
} sol_box_t;

typedef struct sol_window_s {
    uint32_t      ssi_id;
    uint16_t      base_width, base_height;
    int16_t       startx, starty;
    uint16_t      num_buttons;
    sol_button_t *buttons;
    uint16_t      num_frames;
    sol_frame_t  *frames;
    uint16_t      num_boxes;
    sol_box_t    *boxes;
    uint16_t      num_underlays;
    sol_frame_t  *underlays;
} sol_window_t;

extern sol_window_t* sol_window_from_gff(const int res_id);
extern void          sol_window_free_base(sol_window_t *win);

extern void          sol_window_set_pos(sol_window_t *win, const int x, const int y);
extern void          sol_window_render_base(sol_window_t *win);
extern size_t        sol_window_get_button(sol_window_t *win, const int x, const int y);

extern void          sol_button_set_text(sol_button_t *button, const char *text);

#endif
