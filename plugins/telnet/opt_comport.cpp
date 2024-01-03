/*
 * $Id: opt_comport.cpp,v 1.7 2008/12/18 16:50:52 vfrolov Exp $
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
 * $Log: opt_comport.cpp,v $
 * Revision 1.7  2008/12/18 16:50:52  vfrolov
 * Extended the number of possible IN options
 *
 * Revision 1.6  2008/12/12 08:28:34  vfrolov
 * Partialy implemented handling SET-CONTROL commands 0-3 and 13-19
 *
 * Revision 1.5  2008/12/11 13:13:40  vfrolov
 * Implemented PURGE-DATA (RFC 2217)
 *
 * Revision 1.4  2008/12/05 14:10:33  vfrolov
 * Fixed return values of TelnetOptionComPortClient::OnSubNegotiation()
 *
 * Revision 1.3  2008/11/24 16:39:58  vfrolov
 * Implemented FLOWCONTROL-SUSPEND and FLOWCONTROL-RESUME commands (RFC 2217)
 *
 * Revision 1.2  2008/11/13 07:44:12  vfrolov
 * Changed for staticaly linking
 *
 * Revision 1.1  2008/10/24 06:51:23  vfrolov
 * Initial revision
 *
 */

#include "precomp.h"
#include "../plugins_api.h"
///////////////////////////////////////////////////////////////
namespace FilterTelnet {
///////////////////////////////////////////////////////////////
#include "import.h"
#include "opt_comport.h"
///////////////////////////////////////////////////////////////
enum {
  cpcSignature          = 0,
  cpcSetBaudRate        = 1,
  cpcSetDataSize        = 2,
  cpcSetParity          = 3,
  cpcSetStopSize        = 4,
  cpcSetControl         = 5,
  cpcNotifyLineState    = 6,
  cpcNotifyModemState   = 7,
  cpcFlowControlSuspend = 8,
  cpcFlowControlResume  = 9,
  cpcSetLineStateMask   = 10,
  cpcSetModemStateMask  = 11,
  cpcPurgeData          = 12,

  cpcServerBase         = 100,
};
///////////////////////////////////////////////////////////////
enum {
  scReqFCO              = 0,
  scUseNoFCO            = 1,
  scUseXonXoffFCO       = 2,
  scUseHardwareFCO      = 3,

  scReqBreakState       = 4,
  scSetBreakOn          = 5,
  scSetBreakOff         = 6,

  scReqDtrState         = 7,
  scSetDtrOn            = 8,
  scSetDtrOff           = 9,

  scReqRtsState         = 10,
  scSetRtsOn            = 11,
  scSetRtsOff           = 12,

  scReqFCI              = 13,
  scUseNoFCI            = 14,
  scUseXonXoffFCI       = 15,
  scUseHardwareFCI      = 16,

  scUseDcdFCO           = 17,
  scUseDtrFCI           = 18,
  scUseDsrFCO           = 19,
};
///////////////////////////////////////////////////////////////
enum {
  pdRxBuf               = 1,
  pdTxBuf               = 2,
  pdRxTxBuf             = 3,
};
///////////////////////////////////////////////////////////////
inline BYTE p2v_dataSize(BYTE par)
{
  return par;
}

inline BYTE v2p_dataSize(BYTE val)
{
  return val;
}

inline BYTE p2v_parity(BYTE par)
{
  switch (par) {
    case 1: return NOPARITY;
    case 2: return ODDPARITY;
    case 3: return EVENPARITY;
    case 4: return MARKPARITY;
    case 5: return SPACEPARITY;
  }

  return NOPARITY;
}

inline BYTE v2p_parity(BYTE val)
{
  switch (val) {
    case NOPARITY:    return 1;
    case ODDPARITY:   return 2;
    case EVENPARITY:  return 3;
    case MARKPARITY:  return 4;
    case SPACEPARITY: return 5;
  }

  return 1;
}

inline BYTE p2v_stopSize(BYTE par)
{
  switch (par) {
    case 1: return ONESTOPBIT;
    case 2: return TWOSTOPBITS;
    case 3: return ONE5STOPBITS;
  }

  return ONESTOPBIT;
}

inline BYTE v2p_stopSize(BYTE val)
{
  switch (val) {
    case ONESTOPBIT:   return 1;
    case TWOSTOPBITS:  return 2;
    case ONE5STOPBITS: return 3;
  }

  return 1;
}
///////////////////////////////////////////////////////////////
TelnetOptionComPort::TelnetOptionComPort(TelnetProtocol &_telnet, BOOL _isClient, DWORD (&_goMask)[2], DWORD &_soMask)
  : TelnetOption(_telnet, 44 /*COM-PORT-OPTION*/),
    isClient(_isClient),
    goMask(_goMask),
    soMask(_soMask),
    countXoff(0),
    suspended(FALSE)
{
}

void TelnetOptionComPort::AddXoffXon(BOOL xoff)
{
  if (xoff) {
    if (countXoff++ == 0) {
      BYTE_vector params;

      params.push_back((BYTE)(cpcFlowControlSuspend + (isClient ? 0 : cpcServerBase)));
      SendSubNegotiation(params);
    }
  } else {
    if (--countXoff == 0) {
      BYTE_vector params;

      params.push_back((BYTE)(cpcFlowControlResume + (isClient ? 0 : cpcServerBase)));
      SendSubNegotiation(params);
    }
  }
}

void TelnetOptionComPort::OnSuspendResume(BOOL suspend, HUB_MSG **ppMsg)
{
  if (suspended != suspend) {
    suspended = suspend;

    *ppMsg = FlushDecodedStream(*ppMsg);

    if (!*ppMsg)
      return;

    *ppMsg = pMsgInsertVal(*ppMsg, HUB_MSG_TYPE_ADD_XOFF_XON, suspended);
  }
}
///////////////////////////////////////////////////////////////
TelnetOptionComPortClient::TelnetOptionComPortClient(TelnetProtocol &_telnet, DWORD (&_goMask)[2], DWORD &_soMask)
  : TelnetOptionComPort(_telnet, TRUE, _goMask, _soMask)
{
}

void TelnetOptionComPortClient::SetBR(DWORD br)
{
  BYTE_vector params;

  params.push_back((BYTE)(cpcSetBaudRate));
  params.push_back((BYTE)(br >> 24));
  params.push_back((BYTE)(br >> 16));
  params.push_back((BYTE)(br >> 8));
  params.push_back((BYTE)(br));
  SendSubNegotiation(params);
}

void TelnetOptionComPortClient::SetLC(DWORD lc)
{
  if (lc & LC_MASK_BYTESIZE) {
    BYTE_vector params;

    params.push_back((BYTE)(cpcSetDataSize));
    params.push_back(v2p_dataSize(LC2VAL_BYTESIZE(lc)));
    SendSubNegotiation(params);
  }

  if (lc & LC_MASK_PARITY) {
    BYTE_vector params;

    params.push_back((BYTE)(cpcSetParity));
    params.push_back(v2p_parity(LC2VAL_PARITY(lc)));
    SendSubNegotiation(params);
  }

  if (lc & LC_MASK_STOPBITS) {
    BYTE_vector params;

    params.push_back((BYTE)(cpcSetStopSize));
    params.push_back(v2p_stopSize(LC2VAL_STOPBITS(lc)));
    SendSubNegotiation(params);
  }
}

void TelnetOptionComPortClient::SetMCR(BYTE mcr, BYTE mask)
{
  if (mask & SPS_P2V_MCR(PIN_STATE_DTR)) {
    BYTE_vector params;

    params.push_back((BYTE)(cpcSetControl));
    params.push_back((BYTE)((mcr & SPS_P2V_MCR(PIN_STATE_DTR)) ?  scSetDtrOn : scSetDtrOff));
    SendSubNegotiation(params);
  }

  if (mask & SPS_P2V_MCR(PIN_STATE_RTS)) {
    BYTE_vector params;

    params.push_back((BYTE)(cpcSetControl));
    params.push_back((BYTE)((mcr & SPS_P2V_MCR(PIN_STATE_RTS)) ?  scSetRtsOn : scSetRtsOff));
    SendSubNegotiation(params);
  }
}

void TelnetOptionComPortClient::SetBreak(BOOL on)
{
  BYTE_vector params;

  params.push_back((BYTE)(cpcSetControl));
  params.push_back((BYTE)(on ? scSetBreakOn : scSetBreakOff));
  SendSubNegotiation(params);
}

void TelnetOptionComPortClient::PurgeTx()
{
  BYTE_vector params;

  params.push_back((BYTE)(cpcPurgeData));
  params.push_back(pdTxBuf);
  SendSubNegotiation(params);
}

BOOL TelnetOptionComPortClient::OnSubNegotiation(const BYTE_vector &params, HUB_MSG **ppMsg)
{
  if (params.empty())
    return FALSE;

  switch (params[0]) {
    case cpcSignature + cpcServerBase:
      return FALSE;
      break;
    case cpcSetBaudRate + cpcServerBase: {
      if (params.size() != (1 + 4))
        return FALSE;

      DWORD par = ((DWORD)params[1] << 24)
                | ((DWORD)params[2] << 16)
                | ((DWORD)params[3] << 8)
                | ((DWORD)params[4]);

      if (par == 0)
        return FALSE;

      if ((goMask[0] & GO0_LBR_STATUS) == 0)
        break;

      *ppMsg = FlushDecodedStream(*ppMsg);

      if (!*ppMsg)
        return FALSE;

      *ppMsg = pMsgInsertVal(*ppMsg, HUB_MSG_TYPE_LBR_STATUS, par);

      break;
    }
    case cpcSetDataSize + cpcServerBase: {
      if (params.size() != (1 + 1))
        return FALSE;

      BYTE par = params[1];

      if (par == 0)
        return FALSE;

      if ((goMask[0] & GO0_LLC_STATUS) == 0)
        break;

      DWORD lcVal = (VAL2LC_BYTESIZE(p2v_dataSize(par))|LC_MASK_BYTESIZE);

      *ppMsg = FlushDecodedStream(*ppMsg);

      if (!*ppMsg)
        return FALSE;

      *ppMsg = pMsgInsertVal(*ppMsg, HUB_MSG_TYPE_LLC_STATUS, lcVal);

      break;
    }
    case cpcSetParity + cpcServerBase: {
      if (params.size() != (1 + 1))
        return FALSE;

      BYTE par = params[1];

      if (par == 0)
        return FALSE;

      if ((goMask[0] & GO0_LLC_STATUS) == 0)
        break;

      DWORD lcVal = (VAL2LC_PARITY(p2v_parity(par))|LC_MASK_PARITY);

      *ppMsg = FlushDecodedStream(*ppMsg);

      if (!*ppMsg)
        return FALSE;

      *ppMsg = pMsgInsertVal(*ppMsg, HUB_MSG_TYPE_LLC_STATUS, lcVal);

      break;
    }
    case cpcSetStopSize + cpcServerBase: {
      if (params.size() != (1 + 1))
        return FALSE;

      BYTE par = params[1];

      if (par == 0)
        return FALSE;

      if ((goMask[0] & GO0_LLC_STATUS) == 0)
        break;

      DWORD lcVal = (VAL2LC_STOPBITS(p2v_stopSize(par))|LC_MASK_STOPBITS);

      *ppMsg = FlushDecodedStream(*ppMsg);

      if (!*ppMsg)
        return FALSE;

      *ppMsg = pMsgInsertVal(*ppMsg, HUB_MSG_TYPE_LLC_STATUS, lcVal);

      break;
    }
    case cpcSetControl + cpcServerBase:
      if (params.size() != (1 + 1))
        return FALSE;

      switch (params[1]) {
        //case scUseNoFCO:
        //case scUseXonXoffFCO:
        //case scUseHardwareFCO:
        case scSetBreakOn:
        case scSetBreakOff:
        case scSetDtrOn:
        case scSetDtrOff:
        case scSetRtsOn:
        case scSetRtsOff:
        //case scUseNoFCI:
        //case scUseXonXoffFCI:
        //case scUseHardwareFCI:
        //case scUseDcdFCO:
        //case scUseDtrFCI:
        //case scUseDsrFCO:
          break;
        default:
          return FALSE;
      }
      break;
    case cpcNotifyLineState + cpcServerBase: {
      if (params.size() != (1 + 1))
        return FALSE;

      BYTE mask = GO1_O2V_LINE_STATUS(goMask[1]);

      if (mask == 0)
        return FALSE;

      BYTE par = params[1];

      if ((par & mask) == 0)
        break;

      *ppMsg = FlushDecodedStream(*ppMsg);

      if (!*ppMsg)
        return FALSE;

      *ppMsg = pMsgInsertVal(*ppMsg, HUB_MSG_TYPE_LINE_STATUS, par | VAL2MASK(mask));

      break;
    }
    case cpcNotifyModemState + cpcServerBase: {
      if (params.size() != (1 + 1))
        return FALSE;

      BYTE mask = GO1_O2V_MODEM_STATUS(goMask[1]);

      if (mask == 0)
        return FALSE;

      *ppMsg = FlushDecodedStream(*ppMsg);

      if (!*ppMsg)
        return FALSE;

      *ppMsg = pMsgInsertVal(*ppMsg, HUB_MSG_TYPE_MODEM_STATUS, params[1] | VAL2MASK(mask));

      break;
    }
    case cpcFlowControlSuspend + cpcServerBase: {
      if (params.size() != (1 + 0))
        return FALSE;

      OnSuspendResume(TRUE, ppMsg);
      break;
    }
    case cpcFlowControlResume + cpcServerBase: {
      if (params.size() != (1 + 0))
        return FALSE;

      OnSuspendResume(FALSE, ppMsg);
      break;
    }
    case cpcSetLineStateMask + cpcServerBase:
      break;
    case cpcSetModemStateMask + cpcServerBase:
      break;
    case cpcPurgeData + cpcServerBase:
      break;
    default:
      return FALSE;
  }

  return TRUE;
}
///////////////////////////////////////////////////////////////
TelnetOptionComPortServer::TelnetOptionComPortServer(TelnetProtocol &_telnet, DWORD (&_goMask)[2], DWORD &_soMask)
  : TelnetOptionComPort(_telnet, FALSE, _goMask, _soMask),
    br(0),
    brSend(FALSE),
    lc(0),
    lcSend(0),
    mst(0),
    mstMask(255 & SPS_P2V_MST(SO_O2V_PIN_STATE(soMask))),
    lsrMask(0),
    mcr(0),
    mcrMask(0),
    mcrSend(0),
    brk(0),
    brkSend(0)

{
}

void TelnetOptionComPortServer::SetBR(DWORD _br)
{
  br = _br;

  if (br != 0 && brSend) {
    BYTE_vector params;

    params.push_back((BYTE)(cpcSetBaudRate + cpcServerBase));
    params.push_back((BYTE)(br >> 24));
    params.push_back((BYTE)(br >> 16));
    params.push_back((BYTE)(br >> 8));
    params.push_back((BYTE)(br));
    SendSubNegotiation(params);

    brSend = FALSE;
  }
}

void TelnetOptionComPortServer::SetLC(DWORD _lc)
{
  lc = _lc;

  if (lc & lcSend & LC_MASK_BYTESIZE) {
    BYTE_vector params;

    params.push_back((BYTE)(cpcSetDataSize + cpcServerBase));
    params.push_back(v2p_dataSize(LC2VAL_BYTESIZE(lc)));
    SendSubNegotiation(params);

    lcSend &= ~LC_MASK_BYTESIZE;
  }

  if (lc & lcSend & LC_MASK_PARITY) {
    BYTE_vector params;

    params.push_back((BYTE)(cpcSetParity + cpcServerBase));
    params.push_back(v2p_parity(LC2VAL_PARITY(lc)));
    SendSubNegotiation(params);

    lcSend &= ~LC_MASK_PARITY;
  }

  if (lc & lcSend & LC_MASK_STOPBITS) {
    BYTE_vector params;

    params.push_back((BYTE)(cpcSetStopSize + cpcServerBase));
    params.push_back(v2p_stopSize(LC2VAL_STOPBITS(lc)));
    SendSubNegotiation(params);

    lcSend &= ~LC_MASK_STOPBITS;
  }
}

void TelnetOptionComPortServer::NotifyMST(BYTE _mst)
{
  if ((mst ^ _mst) & mstMask) {
    mst = (_mst & mstMask);

    BYTE_vector params;

    params.push_back((BYTE)(cpcNotifyModemState + cpcServerBase));
    params.push_back(mst);
    SendSubNegotiation(params);
  }
}

void TelnetOptionComPortServer::NotifyLSR(BYTE lsr)
{
  lsr &= lsrMask;

  if (lsr) {
    BYTE_vector params;

    params.push_back((BYTE)(cpcNotifyLineState + cpcServerBase));
    params.push_back(lsr);
    SendSubNegotiation(params);
  }
}

void TelnetOptionComPortServer::SetMCR(BYTE _mcr, BYTE mask)
{
  mcr = ((mcr & ~mask) | (_mcr & mask));
  mcrMask |= mask;

  if (mcrMask & mcrSend & SPS_P2V_MCR(PIN_STATE_DTR)) {
    BYTE_vector params;

    params.push_back((BYTE)(cpcSetControl + cpcServerBase));
    params.push_back((BYTE)((mcr & SPS_P2V_MCR(PIN_STATE_DTR)) ?  scSetDtrOn : scSetDtrOff));
    SendSubNegotiation(params);

    mcrSend &= ~SPS_P2V_MCR(PIN_STATE_DTR);
  }

  if (mcrMask & mcrSend & SPS_P2V_MCR(PIN_STATE_RTS)) {
    BYTE_vector params;

    params.push_back((BYTE)(cpcSetControl + cpcServerBase));
    params.push_back((BYTE)((mcr & SPS_P2V_MCR(PIN_STATE_RTS)) ?  scSetRtsOn : scSetRtsOff));
    SendSubNegotiation(params);

    mcrSend &= ~SPS_P2V_MCR(PIN_STATE_RTS);
  }
}

void TelnetOptionComPortServer::SetBreak(BOOL on)
{
  brk = on;

  if (brkSend) {
    BYTE_vector params;

    params.push_back((BYTE)(cpcSetControl + cpcServerBase));
    params.push_back((BYTE)(brk ? scSetBreakOn : scSetBreakOff));
    SendSubNegotiation(params);

    brkSend = FALSE;
  }
}

BOOL TelnetOptionComPortServer::OnSubNegotiation(const BYTE_vector &params, HUB_MSG **ppMsg)
{
  if (params.empty())
    return FALSE;

  switch (params[0]) {
    case cpcSignature:
      return FALSE;
      break;
    case cpcSetBaudRate: {
      if (params.size() != (1 + 4))
        return FALSE;

      DWORD par = ((DWORD)params[1] << 24)
                | ((DWORD)params[2] << 16)
                | ((DWORD)params[3] << 8)
                | ((DWORD)params[4]);

      brSend = TRUE;

      if (par != 0 && br != par) {
        if (goMask[1] & GO1_RBR_STATUS) {
          br = 0;

          *ppMsg = FlushDecodedStream(*ppMsg);

          if (!*ppMsg)
            return FALSE;

          *ppMsg = pMsgInsertVal(*ppMsg, HUB_MSG_TYPE_RBR_STATUS, par);

          if (!*ppMsg || (soMask & SO_SET_BR) == 0) {
            SetBR(par);
            return FALSE;
          }
        } else {
          SetBR(par);
          return FALSE;
        }
      }
      else {
        SetBR(br);
      }

      break;
    }
    case cpcSetDataSize: {
      if (params.size() != (1 + 1))
        return FALSE;

      BYTE par = params[1];
      DWORD lcVal = VAL2LC_BYTESIZE(p2v_dataSize(par))|LC_MASK_BYTESIZE;

      lcSend |= LC_MASK_BYTESIZE;

      if (par != 0 && (lc & (VAL2LC_BYTESIZE(-1)|LC_MASK_BYTESIZE)) != lcVal) {
        lc &= ~(VAL2LC_BYTESIZE(-1)|LC_MASK_BYTESIZE);

        if (goMask[1] & GO1_RLC_STATUS) {
          *ppMsg = FlushDecodedStream(*ppMsg);

          if (!*ppMsg)
            return FALSE;

          *ppMsg = pMsgInsertVal(*ppMsg, HUB_MSG_TYPE_RLC_STATUS, lcVal);

          if (!*ppMsg || (soMask & SO_SET_LC) == 0) {
            SetLC(lcVal);
            return FALSE;
          }
        } else {
          SetLC(lcVal);
          return FALSE;
        }
      }
      else {
        SetLC(lc);
      }

      break;
    }
    case cpcSetParity: {
      if (params.size() != (1 + 1))
        return FALSE;

      BYTE par = params[1];
      DWORD lcVal = VAL2LC_PARITY(p2v_parity(par))|LC_MASK_PARITY;

      lcSend |= LC_MASK_PARITY;

      if (par != 0 && (lc & (VAL2LC_PARITY(-1)|LC_MASK_PARITY)) != lcVal) {
        lc &= ~(VAL2LC_PARITY(-1)|LC_MASK_PARITY);

        if (goMask[1] & GO1_RLC_STATUS) {
          *ppMsg = FlushDecodedStream(*ppMsg);

          if (!*ppMsg)
            return FALSE;

          *ppMsg = pMsgInsertVal(*ppMsg, HUB_MSG_TYPE_RLC_STATUS, lcVal);

          if (!*ppMsg || (soMask & SO_SET_LC) == 0) {
            SetLC(lcVal);
            return FALSE;
          }
        } else {
          SetLC(lcVal);
          return FALSE;
        }
      }
      else {
        SetLC(lc);
      }

      break;
    }
    case cpcSetStopSize: {
      if (params.size() != (1 + 1))
        return FALSE;

      BYTE par = params[1];
      DWORD lcVal = VAL2LC_STOPBITS(p2v_stopSize(par))|LC_MASK_STOPBITS;

      lcSend |= LC_MASK_STOPBITS;

      if (par != 0 && (lc & (VAL2LC_STOPBITS(-1)|LC_MASK_STOPBITS)) != lcVal) {
        lc &= ~(VAL2LC_STOPBITS(-1)|LC_MASK_STOPBITS);

        if (goMask[1] & GO1_RLC_STATUS) {
          *ppMsg = FlushDecodedStream(*ppMsg);

          if (!*ppMsg)
            return FALSE;

          *ppMsg = pMsgInsertVal(*ppMsg, HUB_MSG_TYPE_RLC_STATUS, lcVal);

          if (!*ppMsg || (soMask & SO_SET_LC) == 0) {
            SetLC(lcVal);
            return FALSE;
          }
        } else {
          SetLC(lcVal);
          return FALSE;
        }
      }
      else {
        SetLC(lc);
      }

      break;
    }
    case cpcSetControl: {
      if (params.size() != (1 + 1))
        return FALSE;

      BYTE val = params[1];

      switch (val) {
        case scReqFCO:
        case scUseNoFCO:
        case scUseXonXoffFCO:
        case scUseHardwareFCO:
        case scUseDcdFCO:
        case scUseDsrFCO: {
          BYTE_vector answer;

          answer.push_back((BYTE)(cpcSetControl + cpcServerBase));
          answer.push_back(scUseNoFCO);
          SendSubNegotiation(answer);

          if (val != scReqFCO)
            return FALSE;

          break;
        }
        case scReqFCI:
        case scUseNoFCI:
        case scUseXonXoffFCI:
        case scUseHardwareFCI:
        case scUseDtrFCI: {
          BYTE_vector answer;

          answer.push_back((BYTE)(cpcSetControl + cpcServerBase));
          answer.push_back(scUseNoFCI);
          SendSubNegotiation(answer);

          if (val != scReqFCI)
            return FALSE;

          break;
        }
        case scReqBreakState:
          brkSend = TRUE;
          SetBreak(brk);
          break;
        case scSetBreakOn:
        case scSetBreakOff:
          brkSend = TRUE;
          SetBreak(val == scSetBreakOn);

          if ((goMask[1] & GO1_BREAK_STATUS) == 0)
            return FALSE;

          *ppMsg = FlushDecodedStream(*ppMsg);

          if (!*ppMsg)
            return FALSE;

          *ppMsg = pMsgInsertVal(*ppMsg, HUB_MSG_TYPE_BREAK_STATUS, val == scSetBreakOn);

          break;
        case scReqDtrState:
          mcrSend |= SPS_P2V_MCR(PIN_STATE_DTR);
          SetMCR(mcr, mcrMask);
          break;
        case scSetDtrOn:
        case scSetDtrOff:
          mcrSend |= SPS_P2V_MCR(PIN_STATE_DTR);
          SetMCR(val == scSetDtrOn ? SPS_P2V_MCR(PIN_STATE_DTR) : 0, SPS_P2V_MCR(PIN_STATE_DTR));

          if ((goMask[1] & GO1_V2O_MODEM_STATUS(MODEM_STATUS_DSR)) == 0)
            return FALSE;

          *ppMsg = FlushDecodedStream(*ppMsg);

          if (!*ppMsg)
            return FALSE;

          *ppMsg = pMsgInsertVal(*ppMsg, HUB_MSG_TYPE_MODEM_STATUS,
               (val == scSetDtrOn ? MODEM_STATUS_DSR : 0) | VAL2MASK(MODEM_STATUS_DSR));

          break;
        case scReqRtsState:
          mcrSend |= SPS_P2V_MCR(PIN_STATE_RTS);
          SetMCR(mcr, mcrMask);
          break;
        case scSetRtsOn:
        case scSetRtsOff:
          mcrSend |= SPS_P2V_MCR(PIN_STATE_RTS);
          SetMCR(val == scSetRtsOn ? SPS_P2V_MCR(PIN_STATE_RTS) : 0, SPS_P2V_MCR(PIN_STATE_RTS));

          if ((goMask[1] & GO1_V2O_MODEM_STATUS(MODEM_STATUS_CTS)) == 0)
            return FALSE;

          *ppMsg = FlushDecodedStream(*ppMsg);

          if (!*ppMsg)
            return FALSE;

          *ppMsg = pMsgInsertVal(*ppMsg, HUB_MSG_TYPE_MODEM_STATUS,
               (val == scSetRtsOn ? MODEM_STATUS_CTS : 0) | VAL2MASK(MODEM_STATUS_CTS));

          break;
        default:
          return FALSE;
      }
      break;
    }
    case cpcFlowControlSuspend: {
      if (params.size() != (1 + 0))
        return FALSE;

      OnSuspendResume(TRUE, ppMsg);
      break;
    }
    case cpcFlowControlResume: {
      if (params.size() != (1 + 0))
        return FALSE;

      OnSuspendResume(FALSE, ppMsg);
      break;
    }
    case cpcSetLineStateMask: {
      if (params.size() != (1 + 1))
        return FALSE;

      lsrMask = (params[1] & SO_O2V_LINE_STATUS(soMask));

      BYTE_vector answer;

      answer.push_back((BYTE)(cpcSetLineStateMask + cpcServerBase));
      answer.push_back(lsrMask);
      SendSubNegotiation(answer);
      break;
    }
    case cpcSetModemStateMask: {
      if (params.size() != (1 + 1))
        return FALSE;

      mstMask = (params[1] & SPS_P2V_MST(SO_O2V_PIN_STATE(soMask)));

      BYTE_vector answer;

      answer.push_back((BYTE)(cpcSetModemStateMask + cpcServerBase));
      answer.push_back(mstMask);
      SendSubNegotiation(answer);
      break;
    }
    case cpcPurgeData: {
      if (params.size() != (1 + 1))
        return FALSE;

      BYTE val = params[1];

      switch (val) {
        case pdRxBuf:
        case pdTxBuf:
        case pdRxTxBuf:
          break;
        default:
          return FALSE;
      }

      BYTE_vector answer;

      answer.push_back((BYTE)(cpcPurgeData + cpcServerBase));
      answer.push_back(val);
      SendSubNegotiation(answer);

      if ((goMask[1] & GO1_PURGE_TX_IN) == 0)
        return FALSE;

      if (val == pdRxBuf)
        return FALSE;

      *ppMsg = FlushDecodedStream(*ppMsg);

      if (!*ppMsg)
        return FALSE;

      *ppMsg = pMsgInsertNone(*ppMsg, HUB_MSG_TYPE_PURGE_TX_IN);

      break;
    }
    default:
      return FALSE;
  }

  return TRUE;
}
///////////////////////////////////////////////////////////////
} // end namespace
///////////////////////////////////////////////////////////////
