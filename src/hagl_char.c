/*

MIT License

Copyright (c) 2018-2023 Mika Tuupola

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

-cut-

This file is part of the HAGL graphics library:
https://github.com/tuupola/hagl


SPDX-License-Identifier: MIT

*/

#include <wchar.h>
#include "hagl/color.h"
#include "hagl/bitmap.h"
#include "hagl/blit.h"
#include "hagl.h"
#include "fontx.h"


uint8_t
hagl_get_glyph( void const* _surface, wchar_t code, hagl_color_t color, hagl_bitmap_t* bitmap, const uint8_t* font )
{
	const hagl_surface_t* surface = _surface;
	uint8_t status, set;
	fontx_glyph_t glyph;

	status = fontx_glyph( &glyph, code, font );

	if ( 0 != status )
	{
		return status;
	}

	/* Initialise bitmap dimensions. */
	bitmap->depth = surface->depth;
	bitmap->width = glyph.width;
	bitmap->height = glyph.height;
	bitmap->pitch = bitmap->width * ( bitmap->depth / 8 );
	bitmap->size = bitmap->pitch * bitmap->height;

	hagl_color_t* ptr = ( hagl_color_t* )bitmap->buffer;

	for ( uint8_t y = 0; y < glyph.height; y++ )
	{
		for ( uint8_t x = 0; x < glyph.width; x++ )
		{
			set = *( glyph.buffer ) & ( 0x80 >> ( x % 8 ) );
			if ( set )
			{
				*( ptr++ ) = color;
			}
			else
			{
				*( ptr++ ) = 0x0000;
			}
		}
		glyph.buffer += glyph.pitch;
	}

	return 0;
}


uint8_t hagl_put_char( void const* _surface, wchar_t code, int16_t x, int16_t y, hagl_color_t fg_color, hagl_color_t bg_color, uint32_t flags, const uint8_t* font )
{
	hagl_color_t* buffer = NULL;
	const hagl_surface_t* surface = _surface;
	hagl_bitmap_t bitmap;
	fontx_glyph_t glyph;

	if ( fontx_glyph( &glyph, code, font ) != 0 )
	{
		return 0;
	}

	buffer = calloc( glyph.width * glyph.height, sizeof( hagl_color_t ) );

	if ( flags & HAGL_TEXT_FLAG_CENTERED_VERTICAL ) y -= glyph.height / 2;

	hagl_bitmap_init( &bitmap, glyph.width, glyph.height, surface->depth, buffer );

	hagl_color_t* ptr = bitmap.buffer;

	for ( uint8_t row = 0; row < glyph.height; row++ )
	{
		for ( uint8_t col = 0; col < glyph.width; col++ )
		{
			if ( *( glyph.buffer + col / 8 ) & ( 0x80 >> ( col % 8 ) ) )
			{
				*( ptr++ ) = fg_color;
			}
			else
			{
				*( ptr++ ) = bg_color;

			}
		}
		glyph.buffer += glyph.pitch;
	}


	hagl_blit( surface, x, y, &bitmap );

	free( buffer );

	return bitmap.width;
}


uint16_t hagl_put_text( void const* surface, const wchar_t* str, int16_t x, int16_t y, hagl_color_t fg_color, hagl_color_t bg_color, uint32_t flags, const unsigned char* font )
{
	fontx_meta_t meta;
	const uint16_t start = x;

	if ( fontx_meta( &meta, font ) != 0 ) return 0;

	const size_t string_length = wcslen( str );
	const size_t string_width = string_length * meta.width;

	if ( flags & HAGL_TEXT_FLAG_CENTERED_HORIZONTAL ) x -= string_width / 2;

	do
	{
		const wchar_t current_char = *str++;

		if ( 13 == current_char || 10 == current_char )
		{
			x = 0;
			y += meta.height;
		}
		else
		{
			x += hagl_put_char( surface, current_char, x, y, fg_color, bg_color, flags, font );
		}
	}
	while ( *str != 0 );

	return x - start;
}
