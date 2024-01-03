/*
 * $Id: filter.cpp,v 1.7 2009/02/02 15:21:42 vfrolov Exp $
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
 * $Log: filter.cpp,v $
 * Revision 1.7  2009/02/02 15:21:42  vfrolov
 * Optimized filter's API
 *
 * Revision 1.6  2008/12/22 09:40:45  vfrolov
 * Optimized message switching
 *
 * Revision 1.5  2008/11/25 16:40:40  vfrolov
 * Added assert for port handle
 *
 * Revision 1.4  2008/11/24 12:36:59  vfrolov
 * Changed plugin API
 *
 * Revision 1.3  2008/11/13 07:45:58  vfrolov
 * Changed for staticaly linking
 *
 * Revision 1.2  2008/10/02 07:59:37  vfrolov
 * Changed example
 *
 * Revision 1.1  2008/04/03 14:53:06  vfrolov
 * Initial revision
 *
 */

#include "precomp.h"
#include "../plugins_api.h"
///////////////////////////////////////////////////////////////
namespace FilterEcho {
///////////////////////////////////////////////////////////////
#ifndef _DEBUG
  #define DEBUG_PARAM(par)
#else   /* _DEBUG */
  #define DEBUG_PARAM(par) par
#endif  /* _DEBUG */
///////////////////////////////////////////////////////////////
static ROUTINE_MSG_INSERT_BUF *pMsgInsertBuf = NULL;
///////////////////////////////////////////////////////////////
static PLUGIN_TYPE CALLBACK GetPluginType()
{
  return PLUGIN_TYPE_FILTER;
}
///////////////////////////////////////////////////////////////
static const PLUGIN_ABOUT_A about = {
  sizeof(PLUGIN_ABOUT_A),
  "echo",
  "Copyright (c) 2008 Vyacheslav Frolov",
  "GNU General Public License",
  "Echo filter (alternative to --echo-route option)",
};

static const PLUGIN_ABOUT_A * CALLBACK GetPluginAbout()
{
  return &about;
}
///////////////////////////////////////////////////////////////
static void CALLBACK Help(const char *pProgPath)
{
  cerr
  << "Usage:" << endl
  << "  " << pProgPath << " ... --create-filter=" << GetPluginAbout()->pName << "[,<FID>] ... --add-filters=<ports>:[...,]<FID>[,...] ..." << endl
  << endl
  << "IN method input data stream description:" << endl
  << "  LINE_DATA(<data>)     - <data> is the bytes." << endl
  << endl
  << "IN method output data stream description:" << endl
  << "  LINE_DATA(<data>)     - <data> is the input data stream bytes." << endl
  << endl
  << "IN method echo data stream description:" << endl
  << "  LINE_DATA(<data>)     - <data> is the input data stream bytes." << endl
  << endl
  << "Examples:" << endl
  << "  " << pProgPath << " --load=,,_END_" << endl
  << "      --create-filter=echo" << endl
  << "      --create-filter=flt" << endl
  << "      --add-filters=0:echo,flt" << endl
  << "      COM1" << endl
  << "      COM2" << endl
  << "      _END_" << endl
  << "    - receive data from COM1 and send it ASIS back to COM1 and send it to COM2" << endl
  << "      via filter flt." << endl
  ;
}
///////////////////////////////////////////////////////////////
static BOOL CALLBACK InMethod(
    HFILTER /*hFilter*/,
    HFILTERINSTANCE /*hFilterInstance*/,
    HUB_MSG *pInMsg,
    HUB_MSG **ppEchoMsg)
{
  _ASSERTE(pInMsg != NULL);
  _ASSERTE(ppEchoMsg != NULL);
  _ASSERTE(*ppEchoMsg == NULL);

  switch (HUB_MSG_T2N(pInMsg->type)) {
  case HUB_MSG_T2N(HUB_MSG_TYPE_LINE_DATA):
    _ASSERTE(pInMsg->u.buf.pBuf != NULL || pInMsg->u.buf.size == 0);

    if (pInMsg->u.buf.size == 0)
      return TRUE;

    *ppEchoMsg = pMsgInsertBuf(NULL,
                               HUB_MSG_TYPE_LINE_DATA,
                               pInMsg->u.buf.pBuf,
                               pInMsg->u.buf.size);

    if (!*ppEchoMsg)
      return FALSE;

    break;
  }

  return TRUE;
}
///////////////////////////////////////////////////////////////
static const FILTER_ROUTINES_A routines = {
  sizeof(FILTER_ROUTINES_A),
  GetPluginType,
  GetPluginAbout,
  Help,
  NULL,           // ConfigStart
  NULL,           // Config
  NULL,           // ConfigStop
  NULL,           // Create
  NULL,           // Delete
  NULL,           // CreateInstance
  NULL,           // DeleteInstance
  InMethod,
  NULL,           // OutMethod
};

static const PLUGIN_ROUTINES_A *const plugins[] = {
  (const PLUGIN_ROUTINES_A *)&routines,
  NULL
};
///////////////////////////////////////////////////////////////
PLUGIN_INIT_A InitA;
const PLUGIN_ROUTINES_A *const * CALLBACK InitA(
    const HUB_ROUTINES_A * pHubRoutines)
{
  if (!ROUTINE_IS_VALID(pHubRoutines, pMsgInsertBuf))
    return NULL;

  pMsgInsertBuf = pHubRoutines->pMsgInsertBuf;

  return plugins;
}
///////////////////////////////////////////////////////////////
} // end namespace
///////////////////////////////////////////////////////////////
