/*
 * $Id: filter.cpp,v 1.13 2011/05/19 16:34:47 vfrolov Exp $
 *
 * Copyright (c) 2008-2011 Vyacheslav Frolov
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
 * Revision 1.13  2011/05/19 16:34:47  vfrolov
 * Fixed typo
 *
 * Revision 1.12  2009/03/06 07:56:28  vfrolov
 * Fixed assertion with non ascii chars
 *
 * Revision 1.11  2009/02/02 15:21:42  vfrolov
 * Optimized filter's API
 *
 * Revision 1.10  2008/12/22 09:40:45  vfrolov
 * Optimized message switching
 *
 * Revision 1.9  2008/12/18 16:50:51  vfrolov
 * Extended the number of possible IN options
 *
 * Revision 1.8  2008/11/25 16:40:40  vfrolov
 * Added assert for port handle
 *
 * Revision 1.7  2008/11/24 12:37:00  vfrolov
 * Changed plugin API
 *
 * Revision 1.6  2008/11/13 07:48:35  vfrolov
 * Changed for staticaly linking
 *
 * Revision 1.5  2008/10/22 08:27:26  vfrolov
 * Added ability to set bytesize, parity and stopbits separately
 *
 * Revision 1.4  2008/10/16 16:06:30  vfrolov
 * Added --br and --lc options
 *
 * Revision 1.3  2008/10/16 09:24:23  vfrolov
 * Changed return type of ROUTINE_MSG_REPLACE_*() to BOOL
 *
 * Revision 1.2  2008/10/16 06:34:27  vfrolov
 * Fixed typo
 *
 * Revision 1.1  2008/09/30 08:34:38  vfrolov
 * Initial revision
 *
 */

#include "precomp.h"
#include "../plugins_api.h"
///////////////////////////////////////////////////////////////
namespace FilterLineCtl {
///////////////////////////////////////////////////////////////
#ifndef _DEBUG
  #define DEBUG_PARAM(par)
#else   /* _DEBUG */
  #define DEBUG_PARAM(par) par
#endif  /* _DEBUG */
///////////////////////////////////////////////////////////////
static ROUTINE_MSG_INSERT_VAL *pMsgInsertVal;
static ROUTINE_MSG_REPLACE_NONE *pMsgReplaceNone;
static ROUTINE_PORT_NAME_A *pPortName;
static ROUTINE_FILTER_NAME_A *pFilterName;
static ROUTINE_FILTERPORT *pFilterPort;
///////////////////////////////////////////////////////////////
const char *GetParam(const char *pArg, const char *pPattern)
{
  size_t lenPattern = strlen(pPattern);

  if (_strnicmp(pArg, pPattern, lenPattern) != 0)
    return NULL;

  return pArg + lenPattern;
}
///////////////////////////////////////////////////////////////
class Valid {
  public:
    Valid() : isValid(TRUE) {}
    void Invalidate() { isValid = FALSE; }
    BOOL IsValid() const { return isValid; }
  private:
    BOOL isValid;
};
///////////////////////////////////////////////////////////////
class Filter : public Valid {
  public:
    Filter(const char *pName, int argc, const char *const argv[]);

    const char *FilterName() const { return pName; }

    DWORD soOutMask;
    DWORD goInMask[2];

  private:
    const char *pName;

    void Parse(const char *pArg);
};

Filter::Filter(const char *_pName, int argc, const char *const argv[])
  : pName(_pName),
    soOutMask(0)
{
  goInMask[0] = goInMask[1] = 0;

  for (const char *const *pArgs = &argv[1] ; argc > 1 ; pArgs++, argc--) {
    const char *pArg = GetParam(*pArgs, "--");

    if (!pArg) {
      cerr << "Unknown option " << *pArgs << endl;
      Invalidate();
      continue;
    }

    Parse(pArg);
  }

  if (!soOutMask) {
    Parse("br=remote");
    Parse("lc=remote");
  }
}

void Filter::Parse(const char *pArg)
{
  const char *pParam;

  if ((pParam = GetParam(pArg, "br=")) != NULL) {
    soOutMask |= SO_SET_BR;

    switch (tolower((unsigned char)*pParam)) {
      case 'l':
        goInMask[0] |= GO0_LBR_STATUS;
        break;
      case 'r':
        goInMask[1] |= GO1_RBR_STATUS;
        break;
      default:
        cerr << "Invalid option " << pArg << endl;
        Invalidate();
    }
  }
  else
  if ((pParam = GetParam(pArg, "lc=")) != NULL) {
    soOutMask |= SO_SET_LC;

    switch (tolower((unsigned char)*pParam)) {
      case 'l':
        goInMask[0] |= GO0_LLC_STATUS;
        break;
      case 'r':
        goInMask[1] |= GO1_RLC_STATUS;
        break;
      default:
        cerr << "Invalid option " << pArg << endl;
        Invalidate();
    }
  }
  else {
    cerr << "Unknown option " << pArg << endl;
    Invalidate();
  }
}
///////////////////////////////////////////////////////////////
static PLUGIN_TYPE CALLBACK GetPluginType()
{
  return PLUGIN_TYPE_FILTER;
}
///////////////////////////////////////////////////////////////
static const PLUGIN_ABOUT_A about = {
  sizeof(PLUGIN_ABOUT_A),
  "linectl",
  "Copyright (c) 2008-2011 Vyacheslav Frolov",
  "GNU General Public License",
  "Baudrate and line control mapping filter",
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
  << "  " << pProgPath << " ... --create-filter=" << GetPluginAbout()->pName << "[,<FID>][:<options>] ... --add-filters=<ports>:[...,]<FID>[,...] ..." << endl
  << endl
  << "Wire options:" << endl
  << "  --br=<s>              - wire input state of <s> baudrate to output one." << endl
  << "  --lc=<s>              - wire input state of <s> line control to output one." << endl
  << endl
  << "  The possible values of <s> above can be l[ocal] or r[emote]. If no any wire" << endl
  << "  option specified, then the options --br=remote --lc=remote are used." << endl
  << endl
  << "Examples:" << endl
  << "  " << pProgPath << " --load=,,_END_" << endl
  << "      --create-filter=escparse" << endl
  << "      --add-filters=0,1:escparse" << endl
  << "      --create-filter=linectl" << endl
  << "      --add-filters=0,1:linectl" << endl
  << "      \\\\.\\CNCB0" << endl
  << "      \\\\.\\CNCB1" << endl
  << "      _END_" << endl
  << "    - transfer data, baudrate and line control between CNCB0 and CNCB1." << endl
  ;
}
///////////////////////////////////////////////////////////////
static HFILTER CALLBACK Create(
    HMASTERFILTER hMasterFilter,
    HCONFIG /*hConfig*/,
    int argc,
    const char *const argv[])
{
  _ASSERTE(hMasterFilter != NULL);

  Filter *pFilter = new Filter(pFilterName(hMasterFilter), argc, argv);

  if (!pFilter) {
    cerr << "No enough memory." << endl;
    exit(2);
  }

  if (!pFilter->IsValid()) {
    delete pFilter;
    return NULL;
  }

  return (HFILTER)pFilter;
}
///////////////////////////////////////////////////////////////
static void CALLBACK Delete(
    HFILTER hFilter)
{
  _ASSERTE(hFilter != NULL);

  delete (Filter *)hFilter;
}
///////////////////////////////////////////////////////////////
static HFILTERINSTANCE CALLBACK CreateInstance(
    HMASTERFILTERINSTANCE hMasterFilterInstance)
{
  _ASSERTE(hMasterFilterInstance != NULL);

  HMASTERPORT hMasterPort = pFilterPort(hMasterFilterInstance);

  _ASSERTE(hMasterPort != NULL);

  return (HFILTERINSTANCE)pPortName(hMasterPort);
}
///////////////////////////////////////////////////////////////
static void CALLBACK DeleteInstance(
    HFILTERINSTANCE DEBUG_PARAM(hFilterInstance))
{
  _ASSERTE(hFilterInstance != NULL);
}
///////////////////////////////////////////////////////////////
static BOOL CALLBACK OutMethod(
    HFILTER hFilter,
    HFILTERINSTANCE hFilterInstance,
    HMASTERPORT hFromPort,
    HUB_MSG *pOutMsg)
{
  _ASSERTE(hFilter != NULL);
  _ASSERTE(hFilterInstance != NULL);
  _ASSERTE(hFromPort != NULL);
  _ASSERTE(pOutMsg != NULL);

  switch (HUB_MSG_T2N(pOutMsg->type)) {
    case HUB_MSG_T2N(HUB_MSG_TYPE_SET_OUT_OPTS): {
      // or'e with the required mask to set
      pOutMsg->u.val |= ((Filter *)hFilter)->soOutMask;
      break;
    }
    case HUB_MSG_T2N(HUB_MSG_TYPE_GET_IN_OPTS): {
      _ASSERTE(pOutMsg->u.pv.pVal != NULL);

      int iGo = GO_O2I(pOutMsg->u.pv.val);

      if (iGo != 0 && iGo != 1)
        break;

      // or'e with the required mask to get baudrate and line control
      *pOutMsg->u.pv.pVal |= (((Filter *)hFilter)->goInMask[iGo == 0 ? 0 : 1] & pOutMsg->u.pv.val);
      break;
    }
    case HUB_MSG_T2N(HUB_MSG_TYPE_FAIL_IN_OPTS): {
      int iGo = GO_O2I(pOutMsg->u.pv.val);

      if (iGo != 0 && iGo != 1)
        break;

      DWORD fail_options = (pOutMsg->u.val & ((Filter *)hFilter)->goInMask[iGo == 0 ? 0 : 1]);

      if (fail_options) {
        cerr << (const char *)hFilterInstance
             << " WARNING: Requested by filter " << ((Filter *)hFilter)->FilterName()
             << " for port " << pPortName(hFromPort)
             << " option(s) GO" << iGo << "_0x" << hex << fail_options << dec
             << " not accepted" << endl;
      }
      break;
    }
    case HUB_MSG_T2N(HUB_MSG_TYPE_SET_BR):
      // discard if controlled by this filter
      if (((Filter *)hFilter)->soOutMask & SO_SET_BR) {
        if (!pMsgReplaceNone(pOutMsg, HUB_MSG_TYPE_EMPTY))
          return FALSE;
      }
      break;
    case HUB_MSG_T2N(HUB_MSG_TYPE_SET_LC):
      // discard if controlled by this filter
      if (((Filter *)hFilter)->soOutMask & SO_SET_LC) {
        if (!pMsgReplaceNone(pOutMsg, HUB_MSG_TYPE_EMPTY))
          return FALSE;
      }
      break;
    case HUB_MSG_T2N(HUB_MSG_TYPE_LBR_STATUS):
      if ((((Filter *)hFilter)->goInMask[0] & GO0_LBR_STATUS) == 0)
        break;

      if ((((Filter *)hFilter)->soOutMask & SO_SET_BR) == 0)
        break;

      pOutMsg = pMsgInsertVal(pOutMsg, HUB_MSG_TYPE_SET_BR, pOutMsg->u.val);
      break;
    case HUB_MSG_T2N(HUB_MSG_TYPE_RBR_STATUS):
      if ((((Filter *)hFilter)->goInMask[1] & GO1_RBR_STATUS) == 0)
        break;

      if ((((Filter *)hFilter)->soOutMask & SO_SET_BR) == 0)
        break;

      pOutMsg = pMsgInsertVal(pOutMsg, HUB_MSG_TYPE_SET_BR, pOutMsg->u.val);
      break;
    case HUB_MSG_T2N(HUB_MSG_TYPE_LLC_STATUS):
      if ((((Filter *)hFilter)->goInMask[0] & GO0_LLC_STATUS) == 0)
        break;

      if ((((Filter *)hFilter)->soOutMask & SO_SET_LC) == 0)
        break;

      pOutMsg = pMsgInsertVal(pOutMsg, HUB_MSG_TYPE_SET_LC, pOutMsg->u.val);
      break;
    case HUB_MSG_T2N(HUB_MSG_TYPE_RLC_STATUS):
      if ((((Filter *)hFilter)->goInMask[1] & GO1_RLC_STATUS) == 0)
        break;

      if ((((Filter *)hFilter)->soOutMask & SO_SET_LC) == 0)
        break;

      pOutMsg = pMsgInsertVal(pOutMsg, HUB_MSG_TYPE_SET_LC, pOutMsg->u.val);
      break;
  }

  return pOutMsg != NULL;
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
  Create,
  Delete,
  CreateInstance,
  DeleteInstance,
  NULL,           // InMethod
  OutMethod,
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
  if (!ROUTINE_IS_VALID(pHubRoutines, pMsgInsertVal) ||
      !ROUTINE_IS_VALID(pHubRoutines, pMsgReplaceNone) ||
      !ROUTINE_IS_VALID(pHubRoutines, pPortName) ||
      !ROUTINE_IS_VALID(pHubRoutines, pFilterName) ||
      !ROUTINE_IS_VALID(pHubRoutines, pFilterPort))
  {
    return NULL;
  }

  pMsgInsertVal = pHubRoutines->pMsgInsertVal;
  pMsgReplaceNone = pHubRoutines->pMsgReplaceNone;
  pPortName = pHubRoutines->pPortName;
  pFilterName = pHubRoutines->pFilterName;
  pFilterPort = pHubRoutines->pFilterPort;

  return plugins;
}
///////////////////////////////////////////////////////////////
} // end namespace
///////////////////////////////////////////////////////////////
