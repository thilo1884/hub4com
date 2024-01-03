/*
 * $Id: comio.h,v 1.6 2009/08/04 11:36:49 vfrolov Exp $
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
 * $Log: comio.h,v $
 * Revision 1.6  2009/08/04 11:36:49  vfrolov
 * Implemented priority and reject modifiers for <listen port>
 *
 * Revision 1.5  2008/12/01 17:09:34  vfrolov
 * Improved write buffering
 *
 * Revision 1.4  2008/11/17 16:44:57  vfrolov
 * Fixed race conditions
 *
 * Revision 1.3  2008/10/22 15:31:38  vfrolov
 * Fixed race condition
 *
 * Revision 1.2  2008/08/26 14:07:01  vfrolov
 * Execute OnEvent() in main thread context
 *
 * Revision 1.1  2008/03/27 17:17:27  vfrolov
 * Initial revision
 *
 */

#ifndef _COMIO_H
#define _COMIO_H

///////////////////////////////////////////////////////////////
class ComPort;
class Listener;
///////////////////////////////////////////////////////////////
extern BOOL SetAddr(struct sockaddr_in &sn, const char *pAddr, const char *pPort);
extern SOCKET Socket(const struct sockaddr_in &sn);
extern BOOL Connect(const char *pName, SOCKET hSock, const struct sockaddr_in &snRemote);
extern BOOL Listen(SOCKET hSock);
extern SOCKET Accept(const char *pName, SOCKET hSockListen, int cmd);
extern void Disconnect(const char *pName, SOCKET hSock);
extern void Close(const char *pName, SOCKET hSock);
///////////////////////////////////////////////////////////////
class ReadOverlapped : private OVERLAPPED
{
  public:
    ReadOverlapped(ComPort &_port);
    ~ReadOverlapped();
    BOOL StartRead();

  private:
    static VOID CALLBACK OnRead(
        DWORD err,
        DWORD done,
        LPOVERLAPPED pOverlapped);

    ComPort &port;
    BYTE *pBuf;
};
///////////////////////////////////////////////////////////////
class WriteOverlapped : private OVERLAPPED
{
  public:
    WriteOverlapped(ComPort &_port) : port(_port) {
#ifdef _DEBUG
      pBuf = NULL;
#endif
    }
#ifdef _DEBUG
    ~WriteOverlapped() {
      _ASSERTE(pBuf == NULL);
    }
#endif

    BOOL StartWrite(BYTE *_pBuf, DWORD _len);

  private:
    static VOID CALLBACK OnWrite(
      DWORD err,
      DWORD done,
      LPOVERLAPPED pOverlapped);
    void BufFree();

    ComPort &port;
    BYTE *pBuf;
    DWORD len;
};
///////////////////////////////////////////////////////////////
class SafeDelete
{
  protected:
    SafeDelete() : deleted(FALSE), locked(0) {}

    virtual ~SafeDelete() {}

    void Delete() {
      _ASSERTE(locked >= 0);

      if (locked <= 0)
        delete this;
      else
        deleted = TRUE;
    }

    void LockDelete() { locked++; }
    BOOL UnockDelete() {
      locked--;

      _ASSERTE(locked >= 0);

      if (deleted) {
        if (locked <= 0)
          delete this;

        return FALSE;
      }

      return TRUE;
    }

  private:
    BOOL deleted;
    int locked;
};
///////////////////////////////////////////////////////////////
class WaitEventOverlapped : public SafeDelete
{
  public:
    WaitEventOverlapped(ComPort &_port, SOCKET hSockWait);
    void Delete();
    BOOL StartWaitEvent();

    SOCKET Sock() { return hSock; }

  private:
    static VOID CALLBACK OnEvent(
      PVOID pParameter,
      BOOLEAN timerOrWaitFired);
    static VOID CALLBACK OnEvent(ULONG_PTR pOverlapped);

    ComPort &port;
    SOCKET hSock;
    HANDLE hWait;
    HANDLE hEvent;

#ifdef _DEBUG
  private:
    ~WaitEventOverlapped() {}
#endif  /* _DEBUG */
};
///////////////////////////////////////////////////////////////
class ListenOverlapped : public SafeDelete
{
  public:
    ListenOverlapped(Listener &_listener, SOCKET hSockWait);
    void Delete();
    BOOL StartWaitEvent();

    SOCKET Sock() { return hSock; }

  private:
    static VOID CALLBACK OnEvent(
      PVOID pParameter,
      BOOLEAN timerOrWaitFired);
    static VOID CALLBACK OnEvent(ULONG_PTR pOverlapped);

    Listener &listener;
    SOCKET hSock;
    HANDLE hWait;
    HANDLE hEvent;

#ifdef _DEBUG
  private:
    ~ListenOverlapped() {}
#endif  /* _DEBUG */
};
///////////////////////////////////////////////////////////////

#endif  // _COMIO_H
