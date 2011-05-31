//  MaCoPiX = Mascot Construnctive Pilot for X
//                                (ActX / Gtk+ Evolution)
//
//
//      base64.h  
//      CodeConversion and MIME decoding for MaCoPiX
//           originated from Sylpheed and Sylpheed-Gtk2
//                        *** See below copyright.***
//
//                            Copyright 2002-2007  K.Chimari
//                                     http://rosegray.sakura.ne.jp/
//
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
// 
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
//


/*
 * Sylpheed -- a GTK+ based, lightweight, and fast e-mail client
 * Copyright (C) 1999-2002 Hiroyuki Yamamoto
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __BASE64_H__
#define __BASE64_H__

#include <glib.h>

typedef struct _Base64Decoder	Base64Decoder;

struct _Base64Decoder
{
	gint buf_len;
	gchar buf[4];
};

void base64_encode	(gchar		*out,
			 const guchar	*in,
			 gint		 inlen);
gint base64_decode	(guchar		*out,
			 const gchar	*in,
			 gint		 inlen);

Base64Decoder *base64_decoder_new	(void);
void	       base64_decoder_free	(Base64Decoder	*decoder);
gint	       base64_decoder_decode	(Base64Decoder	*decoder,
					 const gchar	*in,
					 guchar		*out);

#endif /* __BASE64_H__ */
