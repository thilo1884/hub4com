/*
 * $Id: comport.h,v 1.11 2012/06/02 19:37:20 vfrolov Exp $
 *
 * Copyright (c) 2008-2012 Vyacheslav Frolov
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
 * $Log: comport.h,v $
 * Revision 1.11  2012/06/02 19:37:20  vfrolov
 * Added ability to divide incoming TCP sessions
 *
 * Revision 1.10  2009/08/04 11:36:49  vfrolov
 * Implemented priority and reject modifiers for <listen port>
 *
 * Revision 1.9  2009/01/23 16:55:05  vfrolov
 * Utilized timer routines
 *
 * Revision 1.8  2008/12/01 17:09:34  vfrolov
 * Improved write buffering
 *
 * Revision 1.7  2008/11/24 16:30:56  vfrolov
 * Removed pOnXoffXon
 *
 * Revision 1.6  2008/11/24 12:37:00  vfrolov
 * Changed plugin API
 *
 * Revision 1.5  2008/11/17 16:44:57  vfrolov
 * Fixed race conditions
 *
 * Revision 1.4  2008/11/13 07:41:09  vfrolov
 * Changed for staticaly linking
 *
 * Revision 1.3  2008/10/06 12:15:14  vfrolov
 * Added --reconnect option
 *
 * Revision 1.2  2008/03/28 16:00:19  vfrolov
 * Added connectionCounter
 *
 * Revision 1.1  2008/03/27 17:18:27  vfrolov
 * Initial revision
 *
 */

#ifndef _COMPORT_H
#define _COMPORT_H

///////////////////////////////////////////////////////////////
class ComParams;
class WriteOverlapped;
class ReadOverlapped;
class WaitEventOverlapped;
class ListenOverlapped;
class ComPort;
///////////////////////////////////////////////////////////////
class ComPortPtr
{
  public:
    ComPortPtr(ComPort *_pPort = NULL) : pPort(_pPort) {}
    ComPort *Ptr() const { return pPort; }
    bool ComPortPtr::operator<(const ComPortPtr &p) const;

  private:
    ComPort *pPort;
};
///////////////////////////////////////////////////////////////
class Listener
{
  public:
    Listener(const struct sockaddr_in &_snLocal);

    BOOL IsEqual(const struct sockaddr_in &_snLocal) const {
      return memcmp(&snLocal, &_snLocal, sizeof(snLocal)) == 0;
    }

    void Push(ComPort *pPort);
    BOOL Start();
    BOOL OnEvent(ListenOverlapped *pOverlapped, long e, int err);
    void OnDisconnect(ComPort *pPort);
    SOCKET Accept(const ComPort &port, int cmd);

  private:
    struct sockaddr_in snLocal;
    SOCKET hSockListen;
    priority_queue<ComPortPtr> ports;
};
///////////////////////////////////////////////////////////////
class ComPort
{
  public:
    ComPort(
      vector<Listener *> &listeners,
      const ComParams &comParams,
      const char *pPath);

    bool operator<(const ComPort &p) const { return priority < p.priority; }

    BOOL Init(HMASTERPORT _hMasterPort);
    BOOL Start();
    BOOL FakeReadFilter(HUB_MSG *pInMsg);
    BOOL Write(HUB_MSG *pMsg);
    void OnWrite(WriteOverlapped *pOverlapped, DWORD len, DWORD done);
    void OnRead(ReadOverlapped *pOverlapped, BYTE *pBuf, DWORD done);
    BOOL OnEvent(WaitEventOverlapped *pOverlapped, long e);
    void LostReport();
    BOOL Accept();

    const string &Name() const { return name; }
    void Name(const char *pName) { name = pName; }
    HANDLE Handle() const { return (HANDLE)hSock; }

  private:
    void FlowControlUpdate();
    BOOL CanConnect() const { return (permanent || connectionCounter > 0); }
    void StartConnect();
    BOOL StartRead();
    BOOL StartWaitEvent(SOCKET hSockWait);
    void OnConnect();
    void OnDisconnect();

    struct sockaddr_in snLocal;
    struct sockaddr_in snRemote;
    Listener *pListener;
    BOOL rejectZeroConnectionCounter;
    BOOL busyTillZeroConnectionCounter;
    int priority;

    BOOL isValid;

    SOCKET hSock;
    BOOL isConnected;
    BOOL isDisconnected;
    BOOL pendingListenerOnDisconnect;
    int connectionCounter;
    BOOL permanent;

    int reconnectTime;
    HMASTERTIMER hReconnectTimer;

    string name;
    HMASTERPORT hMasterPort;

    int countReadOverlapped;
    int countXoff;

    DWORD writeQueueLimit;
    DWORD writeQueueLimitSendXoff;
    DWORD writeQueueLimitSendXon;
    DWORD writeQueued;
    BOOL writeSuspended;
    DWORD writeLost;
    DWORD writeLostTotal;

    queue<WriteOverlapped *> writeOverlappedBuf;
    BYTE *pWriteBuf;
    DWORD lenWriteBuf;
};
///////////////////////////////////////////////////////////////
inline bool ComPortPtr::operator<(const ComPortPtr &p) const
{
  return *pPort < *p.pPort;
}
///////////////////////////////////////////////////////////////

#endif  // _COMPORT_H
