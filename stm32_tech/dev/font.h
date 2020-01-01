/*
 *  font.h -- `Soft' font definitions
 *
 *  Created 1995 by Geert Uytterhoeven
 *
 *  This file is subject to the terms and conditions of the GNU General Public
 *  License.  See the file COPYING in the main directory of this archive
 *  for more details.
 */

#ifndef _VIDEO_FONT_H
#define _VIDEO_FONT_H



struct fbcon_font_desc 
{

    char *name;
	char *path;
    u16 width, height;
	u16 size;//每个字符字节数
};


extern struct fbcon_font_desc font_vga_8x8;

#endif /* _VIDEO_FONT_H */

