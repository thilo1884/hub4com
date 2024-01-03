/*
 * $Id: opt_comport.h,v 1.4 2008/12/18 16:50:52 vfrolov Exp $
 *
 * Copyright (c) 2008 Vyacheslav Frolov
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
 * $Log: opt_comport.h,v $
 * Revision 1.4  2008/12/18 16:50:52  vfrolov
 * Extended the number of possible IN options
 *
 * Revision 1.3  2008/12/11 13:13:40  vfrolov
 * Implemented PURGE-DATA (RFC 2217)
 *
 * Revision 1.2  2008/11/24 16:39:58  vfrolov
 * Implemented FLOWCONTROL-SUSPEND and FLOWCONTROL-RESUME commands (RFC 2217)
 *
 * Revision 1.1  2008/10/24 06:51:23  vfrolov
 * Initial revision
 *
 */

#ifndef _OPT_COMPORT_H
#define _OPT_COMPORT_H

#include "telnet.h"

///////////////////////////////////////////////////////////////
class TelnetOptionComPort : public TelnetOption
{
  public:
    TelnetOptionComPort(TelnetProtocol &_telnet, BOOL _isClient, DWORD (&_goMask)[2], DWORD &_soMask);

    void AddXoffXon(BOOL xoff);

    virtual void SetBR(DWORD br) = 0;
    virtual void SetLC(DWORD lc) = 0;

    virtual void NotifyMST(BYTE mst) = 0;
    virtual void NotifyLSR(BYTE lsr) = 0;

    virtual void SetMCR(BYTE mcr, BYTE mask) = 0;
    virtual void SetBreak(BOOL on) = 0;

    virtual void PurgeTx() = 0;

  protected:
    void OnSuspendResume(BOOL suspend, HUB_MSG **ppMsg);

    BOOL isClient;

    DWORD (&goMask)[2];
    DWORD &soMask;

    int countXoff;
    BOOL suspended;
};
///////////////////////////////////////////////////////////////
class TelnetOptionComPortClient : public TelnetOptionComPort
{
  public:
    TelnetOptionComPortClient(TelnetProtocol &_telnet, DWORD (&_goMask)[2], DWORD &_soMask);

    virtual void SetBR(DWORD br);
    virtual void SetLC(DWORD lc);

    virtual void NotifyMST(BYTE /*mst*/) { _ASSERTE(FALSE); return; }
    virtual void NotifyLSR(BYTE /*lsr*/) { _ASSERTE(FALSE); return; }

    virtual void SetMCR(BYTE mcr, BYTE mask);
    virtual void SetBreak(BOOL on);

    virtual void PurgeTx();

  protected:
    virtual BOOL OnSubNegotiation(const BYTE_vector &params, HUB_MSG **ppMsg);
};
///////////////////////////////////////////////////////////////
class TelnetOptionComPortServer : public TelnetOptionComPort
{
  public:
    TelnetOptionComPortServer(TelnetProtocol &_telnet, DWORD (&_goMask)[2], DWORD &_soMask);

    virtual void SetBR(DWORD _br);
    virtual void SetLC(DWORD _lc);

    virtual void NotifyMST(BYTE _mst);
    virtual void NotifyLSR(BYTE lsr);

    virtual void SetMCR(BYTE _mcr, BYTE mask);
    virtual void SetBreak(BOOL on);

    virtual void PurgeTx() { _ASSERTE(FALSE); return; }

  protected:
    virtual BOOL OnSubNegotiation(const BYTE_vector &params, HUB_MSG **ppMsg);

  protected:
    DWORD br;
    BOOL brSend;
    DWORD lc;
    DWORD lcSend;
    BYTE mst;
    BYTE mstMask;
    BYTE lsrMask;
    BYTE mcr;
    BYTE mcrMask;
    BYTE mcrSend;
    BOOL brk;
    BOOL brkSend;
};
///////////////////////////////////////////////////////////////

#endif  // _OPT_COMPORT_H
