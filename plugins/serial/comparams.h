/*
 * $Id: comparams.h,v 1.4 2011/07/27 17:08:33 vfrolov Exp $
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
 * $Log: comparams.h,v $
 * Revision 1.4  2011/07/27 17:08:33  vfrolov
 * Implemented serial port share mode
 *
 * Revision 1.3  2008/11/27 13:44:52  vfrolov
 * Added --write-limit option
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

#ifndef _COMPARAMS_H
#define _COMPARAMS_H

///////////////////////////////////////////////////////////////
class ComParams
{
  public:
    ComParams();

    BOOL SetBaudRate(const char *pBaudRate);
    BOOL SetByteSize(const char *pByteSize);
    BOOL SetParity(const char *pParity);
    BOOL SetStopBits(const char *pStopBits);
    BOOL SetOutCts(const char *pOutCts) { return SetFlag(pOutCts, &outCts); }
    BOOL SetOutDsr(const char *pOutDsr) { return SetFlag(pOutDsr, &outDsr); }
    BOOL SetOutX(const char *pOutX) { return SetFlag(pOutX, &outX); }
    BOOL SetInX(const char *pInX) { return SetFlag(pInX, &inX); }
    BOOL SetInDsr(const char *pInDsr) { return SetFlag(pInDsr, &inDsr); }
    BOOL SetIntervalTimeout(const char *pIntervalTimeout);
    BOOL SetWriteQueueLimit(const char *pWriteQueueLimit);
    BOOL SetShareMode(const char *pShareMode) { return SetFlag(pShareMode, &shareMode, FALSE); }

    static string BaudRateStr(long baudRate);
    static string ByteSizeStr(int byteSize);
    static string ParityStr(int parity);
    static string StopBitsStr(int stopBits);
    static string OutCtsStr(int outCts) { return FlagStr(outCts); }
    static string OutDsrStr(int outDsr) { return FlagStr(outDsr); }
    static string OutXStr(int outX) { return FlagStr(outX); }
    static string InXStr(int inX) { return FlagStr(inX); }
    static string InDsrStr(int inDsr) { return FlagStr(inDsr); }
    static string IntervalTimeoutStr(long intervalTimeout);
    static string WriteQueueLimitStr(long writeQueueLimit);
    static string ShareModeStr(int shareMode) { return FlagStr(shareMode, FALSE); }

    string BaudRateStr() const { return BaudRateStr(baudRate); }
    string ByteSizeStr() const { return ByteSizeStr(byteSize); }
    string ParityStr() const { return ParityStr(parity); }
    string StopBitsStr() const { return StopBitsStr(stopBits); }
    string OutCtsStr() const { return OutCtsStr(outCts); }
    string OutDsrStr() const { return OutDsrStr(outDsr); }
    string OutXStr() const { return OutXStr(outX); }
    string InXStr() const { return InXStr(inX); }
    string InDsrStr() const { return InDsrStr(inDsr); }
    string IntervalTimeoutStr() const { return IntervalTimeoutStr(intervalTimeout); }
    string WriteQueueLimitStr() const { return WriteQueueLimitStr(writeQueueLimit); }
    string ShareModeStr() const { return ShareModeStr(shareMode); }

    static const char *BaudRateLst();
    static const char *ByteSizeLst();
    static const char *ParityLst();
    static const char *StopBitsLst();
    static const char *OutCtsLst() { return FlagLst(); }
    static const char *OutDsrLst() { return FlagLst(); }
    static const char *OutXLst() { return FlagLst(); }
    static const char *InXLst() { return FlagLst(); }
    static const char *InDsrLst() { return FlagLst(); }
    static const char *IntervalTimeoutLst();
    static const char *WriteQueueLimitLst();
    static const char *ShareModeLst() { return FlagLst(FALSE); }

    long BaudRate() const { return baudRate; }
    int ByteSize() const { return byteSize; }
    int Parity() const { return parity; }
    int StopBits() const { return stopBits; }
    int OutCts() const { return outCts; }
    int OutDsr() const { return outDsr; }
    int OutX() const { return outX; }
    int InX() const { return inX; }
    int InDsr() const { return inDsr; }
    long IntervalTimeout() const { return intervalTimeout; }
    long WriteQueueLimit() const { return writeQueueLimit; }
    int ShareMode() const { return shareMode; }

  private:
    BOOL SetFlag(const char *pFlagStr, int *pFlag, BOOL withCurrent = TRUE);
    static string FlagStr(int flag, BOOL withCurrent = TRUE);
    static const char *FlagLst(BOOL withCurrent = TRUE);

    long baudRate;
    int byteSize;
    int parity;
    int stopBits;
    int outCts;
    int outDsr;
    int outX;
    int inX;
    int inDsr;
    long intervalTimeout;
    long writeQueueLimit;
    int shareMode;
};
///////////////////////////////////////////////////////////////

#endif  // _COMPARAMS_H
