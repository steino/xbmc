/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "Win32Exception.h"
#include <eh.h>

#define LOG if(logger) logger->Log

void win32_exception::install_handler()
{
    _set_se_translator(win32_exception::translate);
}

void win32_exception::translate(unsigned code, EXCEPTION_POINTERS* info)
{
    // Windows guarantees that *(info->ExceptionRecord) is valid
    switch (code) {
    case EXCEPTION_ACCESS_VIOLATION:
        throw access_violation(*(info->ExceptionRecord));
        break;
    default:
        throw win32_exception(*(info->ExceptionRecord));
    }
}

win32_exception::win32_exception(const EXCEPTION_RECORD& info, const char* classname) : 
  XbmcCommons::Exception(classname ? classname : "win32_exception"),
mWhat("Win32 exception"), mWhere(info.ExceptionAddress), mCode(info.ExceptionCode)
{
    switch (info.ExceptionCode) {
    case EXCEPTION_ACCESS_VIOLATION:
        mWhat = "Access violation";
        break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        mWhat = "Division by zero";
        break;
    }
}

void win32_exception::LogThrowMessage(const char *prefix)  const
{
  if( prefix )
    LOG(LOGERROR, "%s : %s (code:0x%08x) at 0x%08x", prefix, (unsigned int) what(), code(), where());
  else
    LOG(LOGERROR, "%s (code:0x%08x) at 0x%08x", what(), code(), where());
}

access_violation::access_violation(const EXCEPTION_RECORD& info)
: win32_exception(info,"access_voilation"), mAccessType(Invalid), mBadAddress(0)
{
    switch(info.ExceptionInformation[0])
    {
    case 0:
      mAccessType = Read;
      break;
    case 1:
      mAccessType = Write;
      break;
    case 8:
      mAccessType = DEP;
      break;
    }
    mBadAddress = reinterpret_cast<win32_exception ::Address>(info.ExceptionInformation[1]);
}

void access_violation::LogThrowMessage(const char *prefix) const
{
  if( prefix )
    if( mAccessType == Write)
      LOG(LOGERROR, "%s : %s at 0x%08x: Writing location 0x%08x", prefix, what(), where(), address());
    else if( mAccessType == Read)
      LOG(LOGERROR, "%s : %s at 0x%08x: Reading location 0x%08x", prefix, what(), where(), address());
    else if( mAccessType == DEP)
      LOG(LOGERROR, "%s : %s at 0x%08x: DEP violation, location 0x%08x", prefix, what(), where(), address());
    else
      LOG(LOGERROR, "%s : %s at 0x%08x: unknown access type, location 0x%08x", prefix, what(), where(), address());
  else
    if( mAccessType == Write)
      LOG(LOGERROR, "%s at 0x%08x: Writing location 0x%08x", what(), where(), address());
    else if( mAccessType == Read)
      LOG(LOGERROR, "%s at 0x%08x: Reading location 0x%08x", what(), where(), address());
    else if( mAccessType == DEP)
      LOG(LOGERROR, "%s at 0x%08x: DEP violation, location 0x%08x", what(), where(), address());
    else
      LOG(LOGERROR, "%s at 0x%08x: unknown access type, location 0x%08x", what(), where(), address());
}
