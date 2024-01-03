/*
 * $Id: comparams.cpp,v 1.9 2011/07/27 17:08:33 vfrolov Exp $
 *
 * Copyright (c) 2006-2011 Vyacheslav Frolov
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
 * $Log: comparams.cpp,v $
 * Revision 1.9  2011/07/27 17:08:33  vfrolov
 * Implemented serial port share mode
 *
 * Revision 1.8  2010/09/14 16:31:50  vfrolov
 * Implemented --write-limit=0 to disable writing to the port
 *
 * Revision 1.7  2009/03/06 07:56:28  vfrolov
 * Fixed assertion with non ascii chars
 *
 * Revision 1.6  2008/11/27 13:44:52  vfrolov
 * Added --write-limit option
 *
 * Revision 1.5  2008/11/13 07:35:10  vfrolov
 * Changed for staticaly linking
 *
 * Revision 1.4  2008/10/16 06:57:29  vfrolov
 * Added ignoring the case of the params
 *
 * Revision 1.3  2008/08/28 10:24:35  vfrolov
 * Removed linking with ....utils.h and ....utils.cpp
 *
 * Revision 1.2  2008/04/07 12:28:03  vfrolov
 * Replaced --rt-events option by SET_RT_EVENTS message
 *
 * Revision 1.1  2008/03/26 08:44:13  vfrolov
 * Redesigned for using plugins
 *
 * Revision 1.4  2007/05/14 12:06:37  vfrolov
 * Added read interval timeout option
 *
 * Revision 1.3  2007/02/06 11:53:33  vfrolov
 * Added options --odsr, --ox, --ix and --idsr
 * Added communications error reporting
 *
 * Revision 1.2  2007/02/01 12:14:59  vfrolov
 * Redesigned COM port params
 *
 * Revision 1.1  2007/01/23 09:13:10  vfrolov
 * Initial revision
 *
 */

#include "precomp.h"
///////////////////////////////////////////////////////////////
namespace PortSerial {
///////////////////////////////////////////////////////////////
#include "comparams.h"
///////////////////////////////////////////////////////////////
ComParams::ComParams()
  : baudRate(CBR_115200)
  , byteSize(8)
  , parity(NOPARITY)
  , stopBits(ONESTOPBIT)
  , outCts(1)
  , outDsr(0)
  , outX(0)
  , inX(0)
  , inDsr(0)
  , intervalTimeout(0)
  , writeQueueLimit(256)
  , shareMode(0)
{
}

BOOL ComParams::SetBaudRate(const char *pBaudRate)
{
  if (tolower((unsigned char)*pBaudRate) == 'c') {
    baudRate = -1;
    return TRUE;
  }

  if (isdigit((unsigned char)*pBaudRate)) {
    baudRate = atol(pBaudRate);
    return TRUE;
  }

  return FALSE;
}

BOOL ComParams::SetByteSize(const char *pByteSize)
{
  if (tolower((unsigned char)*pByteSize) == 'c') {
    byteSize = -1;
    return TRUE;
  }

  if (isdigit((unsigned char)*pByteSize)) {
    byteSize = atoi(pByteSize);
    return TRUE;
  }

  return FALSE;
}

BOOL ComParams::SetParity(const char *pParity)
{
  switch (tolower((unsigned char)*pParity)) {
    case 'n': parity = NOPARITY; break;
    case 'o': parity = ODDPARITY; break;
    case 'e': parity = EVENPARITY; break;
    case 'm': parity = MARKPARITY; break;
    case 's': parity = SPACEPARITY; break;
    case 'c': parity = -1; break;
    default : return FALSE;
  }
  return TRUE;
}

BOOL ComParams::SetStopBits(const char *pStopBits)
{
  switch (tolower((unsigned char)*pStopBits)) {
    case '1':
      if ((pStopBits[1] == '.' || pStopBits[1] == ',') && pStopBits[2] == '5')
        stopBits = ONE5STOPBITS;
      else
        stopBits = ONESTOPBIT;
      break;
    case '2': stopBits = TWOSTOPBITS; break;
    case 'c': stopBits = -1; break;
    default : return FALSE;
  }
  return TRUE;
}

BOOL ComParams::SetIntervalTimeout(const char *pIntervalTimeout)
{
  if (isdigit((unsigned char)*pIntervalTimeout)) {
    intervalTimeout = atol(pIntervalTimeout);
    return TRUE;
  }

  return FALSE;
}

BOOL ComParams::SetWriteQueueLimit(const char *pWriteQueueLimit)
{
  if (isdigit((unsigned char)*pWriteQueueLimit)) {
    writeQueueLimit = atol(pWriteQueueLimit);
    return writeQueueLimit >= 0;
  }

  return FALSE;
}

BOOL ComParams::SetFlag(const char *pFlagStr, int *pFlag, BOOL withCurrent)
{
  if (_stricmp(pFlagStr, "on") == 0) {
    *pFlag = 1;
  }
  else
  if (_stricmp(pFlagStr, "off") == 0) {
    *pFlag = 0;
  }
  else
  if (withCurrent && tolower((unsigned char)*pFlagStr) == 'c') {
    *pFlag = -1;
  }
  else
    return FALSE;

  return TRUE;
}
///////////////////////////////////////////////////////////////
string ComParams::BaudRateStr(long baudRate)
{
  if (baudRate >= 0) {
    stringstream buf;
    buf << baudRate;
    return buf.str();
  }

  return baudRate == -1 ? "current" : "?";
}

string ComParams::ByteSizeStr(int byteSize)
{
  if (byteSize >= 0) {
    stringstream buf;
    buf << byteSize;
    return buf.str();
  }

  return byteSize == -1 ? "current" : "?";
}

string ComParams::ParityStr(int parity)
{
  switch (parity) {
    case NOPARITY: return "no";
    case ODDPARITY: return "odd";
    case EVENPARITY: return "even";
    case MARKPARITY: return "mark";
    case SPACEPARITY: return "space";
    case -1: return "current";
  }
  return "?";
}

string ComParams::StopBitsStr(int stopBits)
{
  switch (stopBits) {
    case ONESTOPBIT: return "1";
    case ONE5STOPBITS: return "1.5";
    case TWOSTOPBITS: return "2";
    case -1: return "current";
  }
  return "?";
}

string ComParams::IntervalTimeoutStr(long intervalTimeout)
{
  if (intervalTimeout > 0) {
    stringstream buf;
    buf << intervalTimeout;
    return buf.str();
  }

  return "0";
}

string ComParams::WriteQueueLimitStr(long writeQueueLimit)
{
  if (writeQueueLimit >= 0) {
    stringstream buf;
    buf << writeQueueLimit;
    return buf.str();
  }

  return "?";
}

string ComParams::FlagStr(int flag, BOOL withCurrent)
{
  switch (flag) {
    case 1: return "on";
    case 0: return "off";
    case -1: return withCurrent ? "current" : "?";
  }
  return "?";
}
///////////////////////////////////////////////////////////////
const char *ComParams::BaudRateLst()
{
  return "a positive number or c[urrent]";
}

const char *ComParams::ByteSizeLst()
{
  return "a positive number or c[urrent]";
}

const char *ComParams::ParityLst()
{
  return "n[o], o[dd], e[ven], m[ark], s[pace] or c[urrent]";
}

const char *ComParams::StopBitsLst()
{
  return "1, 1.5, 2 or c[urrent]";
}

const char *ComParams::IntervalTimeoutLst()
{
  return "a positive number or 0 milliseconds";
}

const char *ComParams::WriteQueueLimitLst()
{
  return "a positive number or 0";
}

const char *ComParams::FlagLst(BOOL withCurrent)
{
  return withCurrent ? "on, off or c[urrent]" : "on or off";
}
///////////////////////////////////////////////////////////////
} // end namespace
///////////////////////////////////////////////////////////////
