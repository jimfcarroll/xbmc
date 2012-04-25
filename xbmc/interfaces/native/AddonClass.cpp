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

#include "AddonClass.h"

#include "utils/log.h"

#include "LanguageHook.h"
#include "AddonUtils.h"

namespace XBMCAddon
{
  // need a place to put the vtab
  AddonClass::~AddonClass()
  {
    m_isDeallocating= true;

    if (languageHook != NULL)
      languageHook->Release();

#ifdef LOG_LIFECYCLE_EVENTS
    CLog::Log(LOGDEBUG, "NEWADDON LIFECYCLE destroying %s 0x%lx", classname.c_str(), (long)(((void*)this)));
#endif
  }

  AddonClass::AddonClass(const char* cname) : refs(0L), classname(cname), m_isDeallocating(false), 
                                              languageHook(NULL)
  {
#ifdef LOG_LIFECYCLE_EVENTS
    CLog::Log(LOGDEBUG, "NEWADDON LIFECYCLE constructing %s 0x%lx", classname.c_str(), (long)(((void*)this)));
#endif

    // check to see if we have a language hook that was prepared for this instantiation
    languageHook = LanguageHook::getLanguageHook();
    if (languageHook != NULL)
    {
      languageHook->Acquire();

      // here we assume the language hook was set for the single instantiation of
      //  this AddonClass (actually - it's subclass - but whatever). So we
      //  will now reset the Tls. This avoids issues if the constructor of the
      //  subclass throws an exception.
      LanguageHook::clearLanguageHook();
    }
  }

}

              
