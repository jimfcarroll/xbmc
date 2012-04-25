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

#include "ModuleXbmcgui.h"

#include "guilib/GraphicContext.h"
#include "guilib/GUIWindowManager.h"
#include "utils/log.h"

namespace XBMCAddon
{
  namespace xbmcgui
  {
    /**
     * 'xbmcgui.lock()' is depreciated and serves no purpose anymore,
     * it will be removed in future releases.
     *
     * lock() -- Lock the gui until xbmcgui.unlock() is called.
     * 
     * *Note, This will improve performance when doing a lot of gui manipulation at once.
     *        The main program (xbmc itself) will freeze until xbmcgui.unlock() is called.
     * 
     * example:
     *   - xbmcgui.lock()
     */
    void lock()
    {
      CLog::Log(LOGWARNING,"'xbmcgui.lock()' is depreciated and serves no purpose anymore, it will be removed in future releases");
    }

    // unlock() method
    /**
     * 'xbmcgui.unlock()' is depreciated and serves no purpose anymore,
     * it will be removed in future releases.
     *
     * unlock() -- Unlock the gui from a lock() call.
     * 
     * example:
     *   - xbmcgui.unlock()
     */
    void unlock()
    {
      CLog::Log(LOGWARNING,"'xbmcgui.unlock()' is depreciated and serves no purpose anymore, it will be removed in future releases");
    }

    /**
     * getCurrentWindowId() -- Returns the id for the current 'active' window as an integer.
     * 
     * example:
     *   - wid = xbmcgui.getCurrentWindowId()
     */
    long getCurrentWindowId()
    {
      lock();
      int id = g_windowManager.GetActiveWindow();
      unlock();
      return id;
    }

    // getCurrentWindowDialogId() method
    /**
     * getCurrentWindowDialogId() -- Returns the id for the current 'active' dialog as an integer.
     * 
     * example:
     *   - wid = xbmcgui.getCurrentWindowDialogId()
     */
    long getCurrentWindowDialogId()
    {
      lock();
      int id = g_windowManager.GetTopMostModalDialogID();
      unlock();
      return id;
    }
  }
}
