/*
 * $Id: utils.h,v 1.7 2009/02/20 18:32:35 vfrolov Exp $
 *
 * Copyright (c) 2006-2009 Vyacheslav Frolov
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
 * $Log: utils.h,v $
 * Revision 1.7  2009/02/20 18:32:35  vfrolov
 * Added info about location of options
 *
 * Revision 1.6  2009/02/04 12:26:54  vfrolov
 * Implemented --load option for filters
 *
 * Revision 1.5  2008/10/02 07:52:38  vfrolov
 * Added removing macroses for undefined parameters of --load option
 *
 * Revision 1.4  2008/08/28 15:53:13  vfrolov
 * Added ability to load arguments from standard input and
 * to select fragment for loading
 *
 * Revision 1.3  2008/04/16 14:07:12  vfrolov
 * Extended STRQTOK_R()
 *
 * Revision 1.2  2008/03/26 08:14:09  vfrolov
 * Added
 *   - class Args
 *   - STRQTOK_R()
 *   - CreateArgsVector()/FreeArgsVector()
 *
 * Revision 1.1  2007/01/23 09:13:10  vfrolov
 * Initial revision
 *
 */

#ifndef _UTILS_H_
#define _UTILS_H_

///////////////////////////////////////////////////////////////
class Arg : protected ARG_INFO_A
{
  public:
    Arg(const char *_pArg,
        const char *_pFile = NULL,
        int _iLine = -1,
        const char *_pReference = NULL)
    {
      Init(_pArg, _pFile, _iLine, _pReference);
    }

    Arg(const Arg &arg) {
      Init(arg.c_str(), arg.pFile, arg.iLine, arg.pReference);
    }

    Arg::~Arg();

    const char *c_str() const { return (const char *)pBuf; }
    ostream &Arg::OutReference(ostream &out, const string &prefix, const string &suffix) const;

    static Arg *Arg::GetArg(const char *_pArg);
    static const ARG_INFO_A *Arg::GetArgInfo(const char *_pArg) { return GetArg(_pArg); }

  protected:
    void Init(
        const char *_pArg,
        const char *_pFile,
        int _iLine,
        const char *_pReference);

    BYTE *pBuf;
};
///////////////////////////////////////////////////////////////
class Args : public vector<Arg>
{
  public:
    Args() : num_recursive(0) {}
    Args(int argc, const char *const argv[]);

    void Add(const vector<Arg> &args);

    static const char *LoadPrefix() { return "--load="; }
    static int RecursiveMax() { return 256; }

  private:
    void Add(const Arg &arg);

    int num_recursive;
};
///////////////////////////////////////////////////////////////
char *STRTOK_R(char *pStr, const char *pDelims, char **ppSave, BOOL skipLeadingDelims = FALSE);
char *STRQTOK_R(
    char *pStr,
    const char *pDelims,
    char **ppSave,
    const char *pQuotes = "\"\"",
    BOOL discardQuotes = TRUE,
    BOOL skipLeadingDelims = FALSE);
BOOL StrToInt(const char *pStr, int *pNum);
const char *GetParam(const char *pArg, const char *pPattern);
void CreateArgsVector(
    const char *pName,
    const char *pArgs,
    Args &args,
    int *pArgc,
    const char ***pArgv);
void FreeArgsVector(
    const char **argv);
///////////////////////////////////////////////////////////////

#endif /* _UTILS_H_ */
