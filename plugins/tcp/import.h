/*
 * $Id: import.h,v 1.7 2009/09/14 09:08:48 vfrolov Exp $
 *
 * Copyright (c) 2008-2009 Vyacheslav Frolov
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 * $Log: import.h,v $
 * Revision 1.7  2009/09/14 09:08:48  vfrolov
 * Added discarding owned tick (for optimization)
 *
 * Revision 1.6  2009/01/23 16:55:05  vfrolov
 * Utilized timer routines
 *
 * Revision 1.5  2008/12/01 17:09:34  vfrolov
 * Improved write buffering
 *
 * Revision 1.4  2008/11/24 16:30:56  vfrolov
 * Removed pOnXoffXon
 *
 * Revision 1.3  2008/11/24 12:37:00  vfrolov
 * Changed plugin API
 *
 * Revision 1.2  2008/11/13 07:41:09  vfrolov
 * Changed for staticaly linking
 *
 * Revision 1.1  2008/03/27 17:19:18  vfrolov
 * Initial revision
 *
 */

#ifndef _IMPORT_H
#define _IMPORT_H

///////////////////////////////////////////////////////////////
extern ROUTINE_MSG_REPLACE_NONE *pMsgReplaceNone;
extern ROUTINE_BUF_ALLOC *pBufAlloc;
extern ROUTINE_BUF_FREE *pBufFree;
extern ROUTINE_BUF_APPEND *pBufAppend;
extern ROUTINE_ON_READ *pOnRead;
extern ROUTINE_TIMER_CREATE *pTimerCreate;
extern ROUTINE_TIMER_SET *pTimerSet;
///////////////////////////////////////////////////////////////

#endif  // _IMPORT_H
