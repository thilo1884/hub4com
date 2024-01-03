/*
 * $Id: comport.cpp,v 1.27 2011/07/27 17:08:33 vfrolov Exp $
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
 * $Log: comport.cpp,v $
 * Revision 1.27  2011/07/27 17:08:33  vfrolov
 * Implemented serial port share mode
 *
 * Revision 1.26  2011/05/19 16:46:59  vfrolov
 * Fixed unexpected assertion
 * Added human readable printing output options set
 *
 * Revision 1.25  2010/09/14 16:31:50  vfrolov
 * Implemented --write-limit=0 to disable writing to the port
 *
 * Revision 1.24  2008/12/22 09:40:46  vfrolov
 * Optimized message switching
 *
 * Revision 1.23  2008/12/18 16:50:52  vfrolov
 * Extended the number of possible IN options
 *
 * Revision 1.22  2008/12/17 11:52:35  vfrolov
 * Replaced ComIo::dcb by serialBaudRate, serialLineControl,
 * serialHandFlow and serialChars
 * Replaced ComPort::filterX by ComIo::FilterX()
 * Replaced SetManual*() by PinStateControlMask()
 *
 * Revision 1.21  2008/12/11 13:07:54  vfrolov
 * Added PURGE_TX
 *
 * Revision 1.20  2008/12/01 17:06:29  vfrolov
 * Improved write buffering
 *
 * Revision 1.19  2008/11/27 16:25:08  vfrolov
 * Improved write buffering
 *
 * Revision 1.18  2008/11/27 13:44:52  vfrolov
 * Added --write-limit option
 *
 * Revision 1.17  2008/11/24 16:30:56  vfrolov
 * Removed pOnXoffXon
 *
 * Revision 1.16  2008/11/24 12:37:00  vfrolov
 * Changed plugin API
 *
 * Revision 1.15  2008/11/13 07:35:10  vfrolov
 * Changed for staticaly linking
 *
 * Revision 1.14  2008/10/22 08:27:26  vfrolov
 * Added ability to set bytesize, parity and stopbits separately
 *
 * Revision 1.13  2008/10/16 16:04:39  vfrolov
 * Added LBR_STATUS and LLC_STATUS
 *
 * Revision 1.12  2008/09/30 08:28:32  vfrolov
 * Added ability to control OUT1 and OUT2 pins
 * Added ability to get remote baud rate and line control settings
 * Added ability to set baud rate and line control
 * Added fallback to non escape mode
 *
 * Revision 1.11  2008/08/29 13:02:37  vfrolov
 * Added ESC_OPTS_MAP_EO2GO() and ESC_OPTS_MAP_GO2EO()
 *
 * Revision 1.10  2008/08/28 16:07:09  vfrolov
 * Tracing of HUB_MSG_TYPE_SET_PIN_STATE moved to the trace filter
 *
 * Revision 1.9  2008/08/22 16:57:12  vfrolov
 * Added
 *   HUB_MSG_TYPE_GET_ESC_OPTS
 *   HUB_MSG_TYPE_FAIL_ESC_OPTS
 *   HUB_MSG_TYPE_BREAK_STATUS
 *
 * Revision 1.8  2008/08/22 12:45:34  vfrolov
 * Added masking to HUB_MSG_TYPE_MODEM_STATUS and HUB_MSG_TYPE_LINE_STATUS
 *
 * Revision 1.7  2008/08/20 14:30:19  vfrolov
 * Redesigned serial port options
 *
 * Revision 1.6  2008/08/15 12:44:59  vfrolov
 * Added fake read filter method to ports
 *
 * Revision 1.5  2008/08/13 15:14:02  vfrolov
 * Print bit values in readable form
 *
 * Revision 1.4  2008/08/11 07:15:34  vfrolov
 * Replaced
 *   HUB_MSG_TYPE_COM_FUNCTION
 *   HUB_MSG_TYPE_INIT_LSR_MASK
 *   HUB_MSG_TYPE_INIT_MST_MASK
 * by
 *   HUB_MSG_TYPE_SET_PIN_STATE
 *   HUB_MSG_TYPE_GET_OPTIONS
 *   HUB_MSG_TYPE_SET_OPTIONS
 *
 * Revision 1.3  2008/04/11 14:48:42  vfrolov
 * Replaced SET_RT_EVENTS by INIT_LSR_MASK and INIT_MST_MASK
 * Replaced COM_ERRORS by LINE_STATUS
 *
 * Revision 1.2  2008/04/07 12:28:03  vfrolov
 * Replaced --rt-events option by SET_RT_EVENTS message
 *
 * Revision 1.1  2008/03/26 08:44:34  vfrolov
 * Redesigned for using plugins
 *
 * Revision 1.4  2007/04/16 07:33:38  vfrolov
 * Fixed LostReport()
 *
 * Revision 1.3  2007/02/06 11:53:33  vfrolov
 * Added options --odsr, --ox, --ix and --idsr
 * Added communications error reporting
 *
 * Revision 1.2  2007/02/05 09:33:20  vfrolov
 * Implemented internal flow control
 *
 * Revision 1.1  2007/01/23 09:13:10  vfrolov
 * Initial revision
 *
 */

#include "precomp.h"
#include "../plugins_api.h"
///////////////////////////////////////////////////////////////
namespace PortSerial {
///////////////////////////////////////////////////////////////
#include "comport.h"
#include "comio.h"
#include "comparams.h"
#include "import.h"
///////////////////////////////////////////////////////////////
struct FIELD2NAME {
  DWORD field;
  DWORD mask;
  const char *name;
};
#define TOFIELD2NAME2(p, s) { (ULONG)p##s, (ULONG)p##s, #s }

static BOOL PrintFields(
    ostream &tout,
    const FIELD2NAME *pTable,
    DWORD fields,
    BOOL delimitNext = FALSE,
    const char *pUnknownPrefix = "",
    const char *pDelimiter = "|")
{
  if (pTable) {
    while (pTable->name) {
      DWORD field = (fields & pTable->mask);

      if (field == pTable->field) {
        fields &= ~pTable->mask;
        if (delimitNext)
          tout << pDelimiter;
        else
          delimitNext = TRUE;

        tout << pTable->name;
      }
      pTable++;
    }
  }

  if (fields) {
    if (delimitNext)
      tout << pDelimiter;
    else
      delimitNext = TRUE;

    tout << pUnknownPrefix << "0x" << hex << fields << dec;
  }

  return delimitNext;
}
///////////////////////////////////////////////////////////////
static const FIELD2NAME fieldNameTableModemStatus[] = {
  TOFIELD2NAME2(MODEM_STATUS_, DCTS),
  TOFIELD2NAME2(MODEM_STATUS_, DDSR),
  TOFIELD2NAME2(MODEM_STATUS_, TERI),
  TOFIELD2NAME2(MODEM_STATUS_, DDCD),
  TOFIELD2NAME2(MODEM_STATUS_, CTS),
  TOFIELD2NAME2(MODEM_STATUS_, DSR),
  TOFIELD2NAME2(MODEM_STATUS_, RI),
  TOFIELD2NAME2(MODEM_STATUS_, DCD),
  {0, 0, NULL}
};
///////////////////////////////////////////////////////////////
static const FIELD2NAME fieldNameTableLineStatus[] = {
  TOFIELD2NAME2(LINE_STATUS_, DR),
  TOFIELD2NAME2(LINE_STATUS_, OE),
  TOFIELD2NAME2(LINE_STATUS_, PE),
  TOFIELD2NAME2(LINE_STATUS_, FE),
  TOFIELD2NAME2(LINE_STATUS_, BI),
  TOFIELD2NAME2(LINE_STATUS_, THRE),
  TOFIELD2NAME2(LINE_STATUS_, TEMT),
  TOFIELD2NAME2(LINE_STATUS_, FIFOERR),
  {0, 0, NULL}
};
///////////////////////////////////////////////////////////////
static const FIELD2NAME fieldNameTableGo0Options[] = {
  TOFIELD2NAME2(GO0_, LBR_STATUS),
  TOFIELD2NAME2(GO0_, LLC_STATUS),
  TOFIELD2NAME2(GO0_, ESCAPE_MODE),
  {0, 0, NULL}
};
///////////////////////////////////////////////////////////////
static const FIELD2NAME fieldNameTableGo1Options[] = {
  TOFIELD2NAME2(GO1_, RBR_STATUS),
  TOFIELD2NAME2(GO1_, RLC_STATUS),
  TOFIELD2NAME2(GO1_, BREAK_STATUS),
  {0, 0, NULL}
};
///////////////////////////////////////////////////////////////
static const FIELD2NAME fieldNameTableSetPinState[] = {
  TOFIELD2NAME2(PIN_STATE_, RTS),
  TOFIELD2NAME2(PIN_STATE_, DTR),
  TOFIELD2NAME2(PIN_STATE_, OUT1),
  TOFIELD2NAME2(PIN_STATE_, OUT2),
  TOFIELD2NAME2(PIN_STATE_, CTS),
  TOFIELD2NAME2(PIN_STATE_, DSR),
  TOFIELD2NAME2(PIN_STATE_, RI),
  TOFIELD2NAME2(PIN_STATE_, DCD),
  TOFIELD2NAME2(PIN_STATE_, BREAK),
  {0, 0, NULL}
};
///////////////////////////////////////////////////////////////
static const FIELD2NAME fieldNameTableSoOptions[] = {
  TOFIELD2NAME2(SO_, SET_BR),
  TOFIELD2NAME2(SO_, SET_LC),
  {0, 0, NULL}
};
///////////////////////////////////////////////////////////////
static const FIELD2NAME fieldNameTableComEvents[] = {
  TOFIELD2NAME2(EV_, CTS),
  TOFIELD2NAME2(EV_, DSR),
  TOFIELD2NAME2(EV_, RLSD),
  TOFIELD2NAME2(EV_, RING),
  {0, 0, NULL}
};
///////////////////////////////////////////////////////////////
static const FIELD2NAME fieldNameTableCommErrors[] = {
  TOFIELD2NAME2(CE_, RXOVER),
  TOFIELD2NAME2(CE_, OVERRUN),
  TOFIELD2NAME2(CE_, RXPARITY),
  TOFIELD2NAME2(CE_, FRAME),
  TOFIELD2NAME2(CE_, BREAK),
  TOFIELD2NAME2(CE_, TXFULL),
  {0, 0, NULL}
};
///////////////////////////////////////////////////////////////
static BOOL PrintGoOptions(
    ostream &tout,
    DWORD fields,
    BOOL delimitNext = FALSE)
{
  int iGo = GO_O2I(fields);

  fields &= ~GO_I2O(-1);

  const FIELD2NAME *pTable;

  switch (iGo) {
    case 0:
      pTable = fieldNameTableGo0Options;
      break;
    case 1:
      delimitNext = PrintFields(tout, fieldNameTableModemStatus, GO1_O2V_MODEM_STATUS(fields), delimitNext, "MST_");
      delimitNext = PrintFields(tout, fieldNameTableLineStatus, GO1_O2V_LINE_STATUS(fields), delimitNext, "LSR_");
      fields &= ~(GO1_V2O_MODEM_STATUS(-1) | GO1_V2O_LINE_STATUS(-1));
      pTable = fieldNameTableGo1Options;
      break;
    default:
      pTable = NULL;
  }

  stringstream buf;

  buf << "GO" << iGo << "_";

  delimitNext = PrintFields(tout, pTable, fields, delimitNext, buf.str().c_str());

  return delimitNext;
}
///////////////////////////////////////////////////////////////
static BOOL PrintEscOptions(
    ostream &tout,
    DWORD fields,
    BOOL delimitNext = FALSE)
{
  PrintGoOptions(tout, ESC_OPTS_MAP_EO_2_GO1(fields) | GO_I2O(1), delimitNext);
  PrintFields(tout, NULL, fields & ~ESC_OPTS_MAP_GO1_2_EO(-1), delimitNext);

  return delimitNext;
}
///////////////////////////////////////////////////////////////
ComPort::ComPort(
    const ComParams &comParams,
    const char *pPath)
  : hMasterPort(NULL)
  , countReadOverlapped(0)
  , countWaitCommEventOverlapped(0)
  , countXoff(0)
  , escapeOptions(0)
  , outOptions(0)
#ifdef _DEBUG
  , outOptionsRequested(0)
#endif
  , connectionCounter(0)
  , connectSent(FALSE)
  , permanent(comParams.ShareMode() == 0)
  , writeQueueLimit(comParams.WriteQueueLimit())
  , writeQueueLimitSendXoff((writeQueueLimit*2)/3)
  , writeQueueLimitSendXon(writeQueueLimit/3)
  , writeQueued(0)
  , writeSuspended(FALSE)
  , writeLost(0)
  , writeLostTotal(0)
  , errors(0)
  , pWriteBuf(NULL)
  , lenWriteBuf(0)
{
  pComIo = new ComIo(*this, pPath);

  if (!pComIo) {
    cerr << "No enough memory." << endl;
    exit(2);
  }

  name = pComIo->Path().substr(pComIo->Path().rfind('\\') + 1);

  for (int iO = 0 ; iO < 2 ; iO++) {
    intercepted_options[iO] = 0;
    inOptions[iO] = 0;
  }

  for (int i = 0 ; i < 3 ; i++) {
    _ASSERTE(pComIo != NULL);

    WriteOverlapped *pOverlapped = new WriteOverlapped(*pComIo);

    if (!pOverlapped) {
      cerr << "No enough memory." << endl;
      exit(2);
    }

    writeOverlappedBuf.push(pOverlapped);
  }

  pComIo->Open(comParams);
}

BOOL ComPort::Init(HMASTERPORT _hMasterPort)
{
  _ASSERTE(pComIo != NULL);

  if (pComIo->Handle() == INVALID_HANDLE_VALUE) {
    cerr << "ComPort::Init(): Invalid handle" << endl;
    return FALSE;
  }

  hMasterPort = _hMasterPort;

  return TRUE;
}

void ComPort::Update()
{
  if ((permanent || connectionCounter > 0) && pComIo->Handle() == INVALID_HANDLE_VALUE) {
    if (pComIo->Open())
      Start(FALSE);

    return;
  }

  if (!permanent && connectionCounter <= 0) {
    pComIo->Close();

    if (connectSent) {
      connectSent = FALSE;

      HUB_MSG msg;

      msg.type = HUB_MSG_TYPE_CONNECT;
      msg.u.val = FALSE;
      pOnRead(hMasterPort, &msg);
    }
  }
}

BOOL ComPort::Start()
{
  return Start(true);
}

BOOL ComPort::Start(BOOL first)
{
  //cout << name << " Start " << ::GetCurrentThreadId() << endl;

  _ASSERTE(hMasterPort != NULL);
  _ASSERTE(pComIo != NULL);

  if (!first && !permanent && connectionCounter <= 0) {
    pComIo->Close();
    return TRUE;
  }

  _ASSERTE(pComIo->Handle() != INVALID_HANDLE_VALUE || (!first && !permanent));

  if (pComIo->Handle() == INVALID_HANDLE_VALUE) {
    cerr << name << " ComPort::Start(" << first << "): Invalid handle" << endl;
    return FALSE;
  }

  BYTE *pBuf = NULL;
  DWORD done = 0;
  HUB_MSG msg;

  if (!first) {
    UpdateOutOptions(outOptions);

    _ASSERTE((inOptions[0] & GO0_ESCAPE_MODE) == 0 || (escapeOptions & ~ESC_OPTS_V2O_ESCCHAR(-1)) != 0);

    if (inOptions[0] & GO0_ESCAPE_MODE) {
      DWORD failEscapeOptions = pComIo->SetEscMode(escapeOptions, &pBuf, &done);

      if (failEscapeOptions & ESC_OPTS_V2O_ESCCHAR(-1)) {
        cerr << name << " "
                "ComPort::Start(" << first << "): "
                "The escape mode rejected by driver" << endl;

        pComIo->Close();
        pBufFree(pBuf);
        return FALSE;
      }

      if (failEscapeOptions & ~ESC_OPTS_V2O_ESCCHAR(-1)) {
        cerr << name << " "
                "ComPort::Start(" << first << "): "
                "requested escape mode input option(s) [";
                PrintEscOptions(cerr, failEscapeOptions & ~ESC_OPTS_V2O_ESCCHAR(-1));
        cerr << "] will be ignored by driver (possible serial driver was replaced)" << endl;
      }
    }
  }
  else
  if (intercepted_options[0] & GO0_ESCAPE_MODE) {
    _ASSERTE(escapeOptions == 0);
    _ASSERTE((inOptions[0] & GO0_ESCAPE_MODE) == 0);

    msg.type = HUB_MSG_TYPE_GET_ESC_OPTS;
    msg.u.pv.pVal = &escapeOptions;
    msg.u.pv.val = 0;
    pOnRead(hMasterPort, &msg);

    DWORD failEscapeOptions = pComIo->SetEscMode(escapeOptions, &pBuf, &done);

    if ((failEscapeOptions & ESC_OPTS_V2O_ESCCHAR(-1)) == 0) {
      inOptions[0] |= GO0_ESCAPE_MODE;
      escapeOptions &= ~failEscapeOptions;

      _ASSERTE((escapeOptions & ~ESC_OPTS_V2O_ESCCHAR(-1)) != 0);
    }

    if (failEscapeOptions & ~ESC_OPTS_V2O_ESCCHAR(-1)) {
      DWORD options_GO1 = 0;

      msg.type = HUB_MSG_TYPE_FAIL_ESC_OPTS;
      msg.u.pv.pVal = &options_GO1;
      msg.u.pv.val = failEscapeOptions;
      pOnRead(hMasterPort, &msg);

      intercepted_options[1] |= (options_GO1 & ~GO_I2O(-1));

      cerr << name << " "
              "WARNING: "
              "The escape mode " << (((failEscapeOptions & ESC_OPTS_V2O_ESCCHAR(-1)) == 0) ? "enabled but " : "disabled and ")
           << "requested escape mode input option(s) [";
              PrintEscOptions(cerr, failEscapeOptions & ~ESC_OPTS_V2O_ESCCHAR(-1));
      cerr << "] rejected by driver";

      if (options_GO1 & ~GO_I2O(-1)) {
        cerr << ". "
              "Instead them requested non escape mode input option(s) [";
              PrintGoOptions(cerr, (options_GO1 & ~GO_I2O(-1)) | GO_I2O(1));
        cerr << "]";
      }

      cerr << endl;
    }
  }

  if (first) {
    for (int iO = 0 ; iO < 2 ; iO++) {
      inOptions[iO] |= (
        intercepted_options[iO] & (
          iO == 0 ? (
            GO0_LBR_STATUS|GO0_LLC_STATUS
          ) : (
            GO1_RBR_STATUS|GO1_RLC_STATUS |
            GO1_V2O_MODEM_STATUS(
              MODEM_STATUS_CTS |
              MODEM_STATUS_DSR |
              MODEM_STATUS_DCD |
              MODEM_STATUS_RI
            )
          )
        )
      );

      DWORD fail_options = (intercepted_options[iO] & ~inOptions[iO]);

      _ASSERTE((fail_options & GO_I2O(-1)) == 0);

      if (fail_options) {
        cerr << name << " WARNING: Requested input option(s) [";
        PrintGoOptions(cerr, (fail_options & ~GO_I2O(-1)) | GO_I2O(iO == 0 ? 0 : 1));
        cerr << "] will be ignored by driver" << endl;
      }

      msg.type = HUB_MSG_TYPE_FAIL_IN_OPTS;
      msg.u.val = (fail_options & ~GO_I2O(-1)) | GO_I2O(iO == 0 ? 0 : 1);
      pOnRead(hMasterPort, &msg);
    }

    if (inOptions[1] & GO1_RBR_STATUS)
      cerr << name << " WARNING: Suppose remote baud rate is equal local settings" << endl;

    if (inOptions[1] & GO1_RLC_STATUS)
      cerr << name << " WARNING: Suppose remote byte size, parity and stop bits are equal local settings" << endl;
  }

  if (inOptions[1] & GO1_V2O_MODEM_STATUS(
        MODEM_STATUS_CTS |
        MODEM_STATUS_DSR |
        MODEM_STATUS_DCD |
        MODEM_STATUS_RI))
  {
    DWORD events = 0;

    if (inOptions[1] & GO1_V2O_MODEM_STATUS(MODEM_STATUS_CTS))
      events |= EV_CTS;

    if (inOptions[1] & GO1_V2O_MODEM_STATUS(MODEM_STATUS_DSR))
      events |= EV_DSR;

    if (inOptions[1] & GO1_V2O_MODEM_STATUS(MODEM_STATUS_DCD))
      events |= EV_RLSD;

    if (inOptions[1] & GO1_V2O_MODEM_STATUS(MODEM_STATUS_RI))
      events |= EV_RING;

    if (!first) {
      if (!pComIo->SetComEvents(&events) || !StartWaitCommEvent()) {
        cerr << name << " WARNING: Event(s) [";
        PrintFields(cerr, fieldNameTableComEvents, events);
        cerr << "] will be ignored by driver" << endl;
      }
    } else {
      if (!pComIo->SetComEvents(&events) || !StartWaitCommEvent()) {
        pComIo->Close();
        pBufFree(pBuf);
        return FALSE;
      }

      cout << name << " Event(s) [";
      PrintFields(cout, fieldNameTableComEvents, events);
      cout << "] will be monitored" << endl;
    }
  }

  if (!permanent && connectionCounter <= 0) {
    pComIo->Close();
    pBufFree(pBuf);
    return TRUE;
  }

  if (countXoff <= 0) {
    if (!StartRead()) {
      pComIo->Close();
      pBufFree(pBuf);
      return FALSE;
    }
  }

  if (!connectSent) {
    connectSent = TRUE;

    msg.type = HUB_MSG_TYPE_CONNECT;
    msg.u.val = TRUE;
    pOnRead(hMasterPort, &msg);
  }

  if (inOptions[0] & GO0_LBR_STATUS) {
    msg.type = HUB_MSG_TYPE_LBR_STATUS;
    msg.u.val = pComIo->BaudRate();
    pOnRead(hMasterPort, &msg);
  }

  if (inOptions[0] & GO0_LLC_STATUS) {
    msg.type = HUB_MSG_TYPE_LLC_STATUS;
    msg.u.val = pComIo->LineControl();
    pOnRead(hMasterPort, &msg);
  }

  if (inOptions[1] & GO1_RBR_STATUS) {
    msg.type = HUB_MSG_TYPE_RBR_STATUS;
    msg.u.val = pComIo->BaudRate();
    pOnRead(hMasterPort, &msg);
  }

  if (inOptions[1] & GO1_RLC_STATUS) {
    msg.type = HUB_MSG_TYPE_RLC_STATUS;
    msg.u.val = pComIo->LineControl();
    pOnRead(hMasterPort, &msg);
  }

  if (pBuf) {
    msg.type = HUB_MSG_TYPE_LINE_DATA;
    msg.u.buf.pBuf = pBuf;
    msg.u.buf.size = done;
    pOnRead(hMasterPort, &msg);
  }

  CheckComEvents(DWORD(-1));

  return TRUE;
}

BOOL ComPort::StartRead()
{
  if (countReadOverlapped)
    return TRUE;

  _ASSERTE(pComIo != NULL);
  _ASSERTE(pComIo->Handle() != INVALID_HANDLE_VALUE);

  ReadOverlapped *pOverlapped;

  pOverlapped = new ReadOverlapped(*pComIo);

  if (!pOverlapped)
    return FALSE;

  if (!pOverlapped->StartRead()) {
    delete pOverlapped;
    return FALSE;
  }

  countReadOverlapped++;

  //cout << name << " Started Read " << countReadOverlapped << endl;

  return TRUE;
}

BOOL ComPort::FakeReadFilter(HUB_MSG *pInMsg)
{
  _ASSERTE(pInMsg != NULL);

  switch (HUB_MSG_T2N(pInMsg->type)) {
    case HUB_MSG_T2N(HUB_MSG_TYPE_GET_IN_OPTS): {
      int iGo = GO_O2I(pInMsg->u.pv.val);

      if (iGo != 0 && iGo != 1)
        break;

      // get interceptable options from subsequent filters separately

      DWORD interceptable_options = (
        pInMsg->u.pv.val & (
          iGo == 0 ? (
            GO0_ESCAPE_MODE|GO0_LBR_STATUS|GO0_LLC_STATUS
          ) : (
            GO1_RBR_STATUS|GO1_RLC_STATUS|GO1_BREAK_STATUS |
            GO1_V2O_MODEM_STATUS(-1) |
            GO1_V2O_LINE_STATUS(-1)
          )
        )
      );

      pInMsg->u.pv.val &= ~interceptable_options;

      pInMsg = pMsgInsertNone(pInMsg, HUB_MSG_TYPE_EMPTY);

      if (pInMsg) {
        pInMsg->type = HUB_MSG_TYPE_GET_IN_OPTS;
        pInMsg->u.pv.pVal = &intercepted_options[iGo == 0 ? 0 : 1];
        pInMsg->u.pv.val = (interceptable_options & ~GO_I2O(-1)) | GO_I2O(iGo);
      }
      break;
    }
  }

  return pInMsg != NULL;
}

void ComPort::FlowControlUpdate()
{
  if (writeSuspended) {
    if (writeQueued <= writeQueueLimitSendXon) {
      writeSuspended = FALSE;

      HUB_MSG msg;

      msg.type = HUB_MSG_TYPE_ADD_XOFF_XON;
      msg.u.val = FALSE;

      pOnRead(hMasterPort, &msg);
    }
  } else {
    if (writeQueued > writeQueueLimitSendXoff) {
      writeSuspended = TRUE;

      HUB_MSG msg;

      msg.type = HUB_MSG_TYPE_ADD_XOFF_XON;
      msg.u.val = TRUE;

      pOnRead(hMasterPort, &msg);
    }
  }
}

void ComPort::PurgeWrite(BOOL withLost)
{
  _ASSERTE(pComIo != NULL);

  if (pComIo->Handle() != INVALID_HANDLE_VALUE)
    pComIo->PurgeWrite();

  if (lenWriteBuf) {
    _ASSERTE(pWriteBuf != NULL);

    if (withLost)
      writeLost += lenWriteBuf;

    _ASSERTE(writeQueued >= lenWriteBuf);
    writeQueued -= lenWriteBuf;

    lenWriteBuf = 0;
    pBufFree(pWriteBuf);
    pWriteBuf = NULL;
  }

  if (!withLost) {
    _ASSERTE(writeLost >= writeQueued);
    writeLost -= writeQueued;  // compensate
  }
}

void ComPort::FilterX(BYTE *pBuf, DWORD &len)
{
  _ASSERTE(pComIo != NULL);
  _ASSERTE(pComIo->Handle() != INVALID_HANDLE_VALUE);

  BYTE xOn;
  BYTE xOff;

  if (pComIo->FilterX(xOn, xOff)) {
    BYTE *pSrc = pBuf;
    BYTE *pDst = pBuf;

    for (DWORD i = 0 ; i < len ; i++) {
      if (*pSrc == xOn || *pSrc == xOff) {
        pSrc++;
        writeLost++;
      } else {
        *pDst++ = *pSrc++;
      }
    }

    len = DWORD(pDst - pBuf);
  }
}

void ComPort::UpdateOutOptions(DWORD options)
{
  WORD controlMask = pComIo->PinStateControlMask();

  if (options & SO_V2O_PIN_STATE(PIN_STATE_RTS)) {
    if (controlMask & PIN_STATE_RTS)
      outOptions |= SO_V2O_PIN_STATE(PIN_STATE_RTS);
    else
      outOptions &= ~SO_V2O_PIN_STATE(PIN_STATE_RTS);
  }
  if (options & SO_V2O_PIN_STATE(PIN_STATE_DTR)) {
    if (controlMask & PIN_STATE_DTR)
      outOptions |= SO_V2O_PIN_STATE(PIN_STATE_DTR);
    else
      outOptions &= ~SO_V2O_PIN_STATE(PIN_STATE_DTR);
  }
  if (options & SO_V2O_PIN_STATE(PIN_STATE_OUT1)) {
    if (controlMask & PIN_STATE_OUT1)
      outOptions |= SO_V2O_PIN_STATE(PIN_STATE_OUT1);
    else
      outOptions &= ~SO_V2O_PIN_STATE(PIN_STATE_OUT1);
  }
  if (options & SO_V2O_PIN_STATE(PIN_STATE_OUT2)) {
    if (controlMask & PIN_STATE_OUT2)
      outOptions |= SO_V2O_PIN_STATE(PIN_STATE_OUT2);
    else
      outOptions &= ~SO_V2O_PIN_STATE(PIN_STATE_OUT2);
  }

  outOptions |= options & (
      SO_V2O_PIN_STATE(PIN_STATE_BREAK) |
      SO_SET_BR |
      SO_SET_LC |
      SO_PURGE_TX);

  options &= ~outOptions;

  if (options) {
    cerr << name << " WARNING: Requested output option(s) [";
    BOOL delimitNext = FALSE;
    delimitNext = PrintFields(cerr, fieldNameTableSetPinState, SO_O2V_PIN_STATE(options), delimitNext, "SET_");
    PrintFields(cerr, fieldNameTableSoOptions, options & ~SO_V2O_PIN_STATE(-1), delimitNext);
    cerr << "] will be ignored by driver" << endl;
  }
}

BOOL ComPort::Write(HUB_MSG *pMsg)
{
  _ASSERTE(pMsg != NULL);

  switch (HUB_MSG_T2N(pMsg->type)) {
  case HUB_MSG_T2N(HUB_MSG_TYPE_LINE_DATA): {
    if (!writeQueueLimit)
      return TRUE;

    DWORD len = pMsg->u.buf.size;

    if (!len)
      return TRUE;

    BYTE *pBuf = pMsg->u.buf.pBuf;

    if (!pBuf) {
      writeLost += len;
      return FALSE;
    }

    if (writeQueued > writeQueueLimit)
      PurgeWrite(TRUE);

    _ASSERTE(pComIo != NULL);

    if (pComIo->Handle() != INVALID_HANDLE_VALUE && writeOverlappedBuf.size()) {
      _ASSERTE(pWriteBuf == NULL);
      _ASSERTE(lenWriteBuf == 0);

      FilterX(pBuf, len);

      if (!len)
        return TRUE;

      WriteOverlapped *pOverlapped = writeOverlappedBuf.front();

      _ASSERTE(pOverlapped != NULL);

      if (!pOverlapped->StartWrite(pBuf, len)) {
        writeLost += len;
        FlowControlUpdate();
        return FALSE;
      }

      writeOverlappedBuf.pop();

      pMsg->type = HUB_MSG_TYPE_EMPTY;  // detach pBuf
    } else {
      _ASSERTE((pWriteBuf == NULL && lenWriteBuf == 0) || (pWriteBuf != NULL && lenWriteBuf != 0));

      pBufAppend(&pWriteBuf, lenWriteBuf, pBuf, len);
      lenWriteBuf += len;
    }

    writeQueued += len;
    FlowControlUpdate();

    //cout << name << " Started Write " << len << " " << writeQueued << endl;
    break;
  }
  case HUB_MSG_T2N(HUB_MSG_TYPE_CONNECT): {
    if (pMsg->u.val) {
      connectionCounter++;

      _ASSERTE(connectionCounter > 0);

      Update();
    } else {
      _ASSERTE(connectionCounter > 0);

      connectionCounter--;

      Update();
    }
    break;
  }
  case HUB_MSG_T2N(HUB_MSG_TYPE_SET_PIN_STATE):
    _ASSERTE((~SO_O2V_PIN_STATE(outOptionsRequested) & MASK2VAL(pMsg->u.val)) == 0);
    _ASSERTE(pComIo != NULL);

    pComIo->SetPinState((WORD)pMsg->u.val, SO_O2V_PIN_STATE(outOptions) & MASK2VAL(pMsg->u.val));
    break;
  case HUB_MSG_T2N(HUB_MSG_TYPE_SET_BR): {
    _ASSERTE(outOptionsRequested & SO_SET_BR);
    _ASSERTE(pComIo != NULL);

    if ((outOptions & SO_SET_BR) == 0)
      break;

    DWORD oldVal = pComIo->BaudRate();
    DWORD curVal = pComIo->SetBaudRate(pMsg->u.val);

    if (curVal != pMsg->u.val) {
      cerr << name << " WARNING: can't change"
           << " baud rate " << oldVal
           << " to " << pMsg->u.val
           << " (current=" << curVal << ")"
           << endl;
    }

    if (oldVal != curVal) {
      if (inOptions[0] & GO0_LBR_STATUS) {
        HUB_MSG msg;

        msg.type = HUB_MSG_TYPE_LBR_STATUS;
        msg.u.val = curVal;
        pOnRead(hMasterPort, &msg);
      }

      if (inOptions[1] & GO1_RBR_STATUS) {
        HUB_MSG msg;

        msg.type = HUB_MSG_TYPE_RBR_STATUS;
        msg.u.val = curVal;                 // suppose remote equal local
        pOnRead(hMasterPort, &msg);
      }
    }
    break;
  }
  case HUB_MSG_T2N(HUB_MSG_TYPE_SET_LC): {
    _ASSERTE(outOptionsRequested & SO_SET_LC);
    _ASSERTE(pComIo != NULL);

    if ((outOptions & SO_SET_LC) == 0)
      break;

    DWORD oldVal = pComIo->LineControl();
    DWORD curVal = pComIo->SetLineControl(pMsg->u.val);

    DWORD mask = (pMsg->u.val & (LC_MASK_BYTESIZE|LC_MASK_PARITY|LC_MASK_STOPBITS));

    if (mask & LC_MASK_BYTESIZE)
      mask |= VAL2LC_BYTESIZE(-1);

    if (mask & LC_MASK_PARITY)
      mask |= VAL2LC_PARITY(-1);

    if (mask & LC_MASK_STOPBITS)
      mask |= VAL2LC_STOPBITS(-1);

    if ((curVal & mask) != (pMsg->u.val & mask)) {
      cerr << name << " WARNING: can't change"
           << hex
           << " line control 0x" << oldVal
           << " to 0x" << (pMsg->u.val | (oldVal & ~mask))
           << " (current=0x" << curVal << ")"
           << dec
           << endl;
    }

    DWORD changes = (oldVal ^ curVal);

    if (changes) {
      if ((changes & (VAL2LC_BYTESIZE(-1)|LC_MASK_BYTESIZE)) == 0)
        curVal &= ~(VAL2LC_BYTESIZE(-1)|LC_MASK_BYTESIZE);

      if ((changes & (VAL2LC_PARITY(-1)|LC_MASK_PARITY)) == 0)
        curVal &= ~(VAL2LC_PARITY(-1)|LC_MASK_PARITY);

      if ((changes & (VAL2LC_STOPBITS(-1)|LC_MASK_STOPBITS)) == 0)
        curVal &= ~(VAL2LC_STOPBITS(-1)|LC_MASK_STOPBITS);

      if (inOptions[0] & GO0_LLC_STATUS) {
        HUB_MSG msg;

        msg.type = HUB_MSG_TYPE_LLC_STATUS;
        msg.u.val = curVal;
        pOnRead(hMasterPort, &msg);
      }

      if (inOptions[1] & GO1_RLC_STATUS) {
        HUB_MSG msg;

        msg.type = HUB_MSG_TYPE_RLC_STATUS;
        msg.u.val = curVal;                 // suppose remote equal local
        pOnRead(hMasterPort, &msg);
      }
    }
    break;
  }
  case HUB_MSG_T2N(HUB_MSG_TYPE_PURGE_TX):
    _ASSERTE(outOptionsRequested & SO_PURGE_TX);
    _ASSERTE(pComIo != NULL);

    if ((outOptions & SO_PURGE_TX) == 0)
      break;

    PurgeWrite(FALSE);
    FlowControlUpdate();
    break;
  case HUB_MSG_T2N(HUB_MSG_TYPE_SET_OUT_OPTS): {
    _ASSERTE(pComIo != NULL);

#ifdef _DEBUG
    outOptionsRequested |= pMsg->u.val;
#endif

    pMsg->u.val &= ~outOptions;

    UpdateOutOptions(pMsg->u.val);

    pMsg->u.val &= ~outOptions;

    break;
  }
  case HUB_MSG_T2N(HUB_MSG_TYPE_ADD_XOFF_XON):
    if (pMsg->u.val) {
      countXoff++;
    } else {
      if (--countXoff == 0) {
        _ASSERTE(pComIo != NULL);

        if (pComIo->Handle() != INVALID_HANDLE_VALUE)
          StartRead();
      }
    }
    break;
  }

  return TRUE;
}

BOOL ComPort::StartWaitCommEvent()
{
  if (countWaitCommEventOverlapped)
    return TRUE;

  _ASSERTE(pComIo != NULL);

  if (pComIo->Handle() == INVALID_HANDLE_VALUE)
    return FALSE;

  WaitCommEventOverlapped *pOverlapped;

  pOverlapped = new WaitCommEventOverlapped(*pComIo);

  if (!pOverlapped)
    return FALSE;

  if (!pOverlapped->StartWaitCommEvent()) {
    pOverlapped->Delete();
    return FALSE;
  }

  countWaitCommEventOverlapped++;

  return TRUE;
}

void ComPort::OnWrite(WriteOverlapped *pOverlapped, DWORD len, DWORD done)
{
  //cout << name << " OnWrite " << ::GetCurrentThreadId() << " len=" << len << " done=" << done << " queued=" << writeQueued << endl;

  if (len > done)
    writeLost += len - done;

  _ASSERTE(writeQueued >= len);
  writeQueued -= len;

  _ASSERTE(pComIo != NULL);
  _ASSERTE(pWriteBuf != NULL || lenWriteBuf == 0);
  _ASSERTE(pWriteBuf == NULL || lenWriteBuf != 0);

  if (pComIo->Handle() != INVALID_HANDLE_VALUE && lenWriteBuf) {
    _ASSERTE(writeQueued >= lenWriteBuf);
    writeQueued -= lenWriteBuf;

    FilterX(pWriteBuf, lenWriteBuf);

    if (!lenWriteBuf || !pOverlapped->StartWrite(pWriteBuf, lenWriteBuf)) {
      writeOverlappedBuf.push(pOverlapped);

      writeLost += lenWriteBuf;
      pBufFree(pWriteBuf);
    } else {
      writeQueued += lenWriteBuf;
    }

    lenWriteBuf = 0;
    pWriteBuf = NULL;
  } else {
    writeOverlappedBuf.push(pOverlapped);
  }

  FlowControlUpdate();
}

void ComPort::OnRead(ReadOverlapped *pOverlapped, BYTE *pBuf, DWORD done)
{
  //cout << name << " OnRead " << ::GetCurrentThreadId() << endl;

  HUB_MSG msg;

  msg.type = HUB_MSG_TYPE_LINE_DATA;
  msg.u.buf.pBuf = pBuf;
  msg.u.buf.size = done;

  pOnRead(hMasterPort, &msg);

  if (pComIo->Handle() == INVALID_HANDLE_VALUE || countXoff > 0 || !pOverlapped->StartRead()) {
    delete pOverlapped;

    countReadOverlapped--;
    _ASSERTE(countReadOverlapped >= 0);

    if (countXoff <= 0)
      cout << name << " Stopped Read " << countReadOverlapped << endl;
  }
}

void ComPort::OnCommEvent(WaitCommEventOverlapped *pOverlapped, DWORD eMask)
{
  cout << name << " OnCommEvent " << ::GetCurrentThreadId() << " [";
  PrintFields(cout, fieldNameTableComEvents, eMask);
  cout << "]" << endl;

  CheckComEvents(eMask);

  if (pComIo->Handle() == INVALID_HANDLE_VALUE || !pOverlapped->StartWaitCommEvent()) {
    pOverlapped->Delete();

    countWaitCommEventOverlapped--;
    _ASSERTE(countWaitCommEventOverlapped >= 0);

    cout << name << " Stopped WaitCommEvent " << countWaitCommEventOverlapped << endl;
  }
}

void ComPort::CheckComEvents(DWORD eMask)
{
  _ASSERTE(pComIo != NULL);

  if (GO1_O2V_MODEM_STATUS(inOptions[1]) && (eMask & (EV_CTS|EV_DSR|EV_RLSD|EV_RING)) != 0) {
    DWORD stat = 0;

    if (pComIo->Handle() == INVALID_HANDLE_VALUE || ::GetCommModemStatus(pComIo->Handle(), &stat)) {
      HUB_MSG msg;

      msg.type = HUB_MSG_TYPE_MODEM_STATUS;
      msg.u.val = ((DWORD)(BYTE)stat | VAL2MASK(GO1_O2V_MODEM_STATUS(inOptions[1])));

      pOnRead(hMasterPort, &msg);
    }
  }

  if (pComIo->Handle() != INVALID_HANDLE_VALUE && (eMask & (EV_BREAK|EV_ERR)) != 0) {
    DWORD errs;

    if (::ClearCommError(pComIo->Handle(), &errs, NULL))
      errors |= errs;
  }
}

void ComPort::LostReport()
{
  if (writeLost) {
    writeLostTotal += writeLost;
    cout << "Write lost " << name << ": " << writeLost << ", total " << writeLostTotal << endl;
    writeLost = 0;
  }

  CheckComEvents(EV_BREAK|EV_ERR);

  if (errors) {
    cout << "Error " << name << ": ";
    PrintFields(cout, fieldNameTableCommErrors, errors, FALSE, "", " ");
    errors = 0;

    _ASSERTE(pComIo != NULL);

    if (pComIo->Handle() != INVALID_HANDLE_VALUE) {
      #define IOCTL_SERIAL_GET_STATS CTL_CODE(FILE_DEVICE_SERIAL_PORT,35,METHOD_BUFFERED,FILE_ANY_ACCESS)

      typedef struct _SERIALPERF_STATS {
        ULONG ReceivedCount;
        ULONG TransmittedCount;
        ULONG FrameErrorCount;
        ULONG SerialOverrunErrorCount;
        ULONG BufferOverrunErrorCount;
        ULONG ParityErrorCount;
      } SERIALPERF_STATS, *PSERIALPERF_STATS;

      SERIALPERF_STATS stats;
      DWORD size;

      if (DeviceIoControl(pComIo->Handle(), IOCTL_SERIAL_GET_STATS, NULL, 0, &stats, sizeof(stats), &size, NULL)) {
        cout << ", total"
          << " RXOVER=" << stats.BufferOverrunErrorCount
          << " OVERRUN=" << stats.SerialOverrunErrorCount
          << " RXPARITY=" << stats.ParityErrorCount
          << " FRAME=" << stats.FrameErrorCount;
      }
    }

    cout << endl;
  }
}
///////////////////////////////////////////////////////////////
} // end namespace
///////////////////////////////////////////////////////////////
