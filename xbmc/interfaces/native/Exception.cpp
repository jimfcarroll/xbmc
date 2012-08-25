/*
 *      Copyright (C) 2005-2012 Team XBMC
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
#include "Exception.h"

// I'm using this for the FormatV functionality
#include "utils/StdString.h"

namespace XBMCAddon
{
  // need a place to put the vtab
  Exception::~Exception(){}

  void Exception::logThrowMessage()
  {
    CLog::Log(LOGERROR,"NEWADDON EXCEPTION Thrown (%s) : %s", classname.c_str(), message.c_str());
  }

  void Exception::set(const char* fmt, va_list& argList) 
  {
    CStdString tmps;
    tmps.FormatV(fmt, argList);
    message = tmps;

    logThrowMessage();
  }

  void Exception::setMessage(const char* fmt, ...) 
  {
    // calls 'set'
    COPYVARARGS(fmt);
  }

}

