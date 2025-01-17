#ifndef _ZFTAPE_WRITE_H
#define _ZFTAPE_WRITE_H

/*
 * Copyright (C) 1996, 1997 Claus-Justus Heine

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *
 * $Source: /home/cvs/gemini-2.6.15/drivers/char/ftape/zftape/zftape-write.h,v $
 * $Revision: 1.1.1.1 $
 * $Date: 2006/04/03 08:40:46 $
 *
 *      This file contains the definitions for the write functions
 *      for the zftape driver for Linux.
 *
 */

extern int  zft_flush_buffers(void);
extern int  zft_update_header_segments(void);
extern void zft_prevent_flush(void);

/*  hook for the VFS interface 
 */
extern int _zft_write(const char __user *buff, int req_len);
#endif /* _ZFTAPE_WRITE_H */
