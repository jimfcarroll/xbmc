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

#pragma once

#include <limits.h>

#include "WindowException.h"
#include "AddonCallback.h"
#include "Exception.h"
#include "Control.h"
#include "AddonString.h"

#include "swighelper.h"

#include "guilib/GUIWindow.h"

namespace XBMCAddon
{
  namespace xbmcgui
  {
    // Forward declare the interceptor as the AddonWindowInterceptor.h 
    // file needs to include the Window class because of the template
    class InterceptorBase;

    class Action : public AddonClass
    {
    public:
      Action() : AddonClass("Action"), id(-1), fAmount1(0.0f), fAmount2(0.0f), 
                 fRepeat(0.0f), buttonCode(0), strAction("")
      { }

#ifndef SWIG
      Action(const CAction& caction) : AddonClass("Action") { setFromCAction(caction); }

      void setFromCAction(const CAction& caction);

      long id;
      float fAmount1;
      float fAmount2;
      float fRepeat;
      unsigned long buttonCode;
      std::string strAction;

      // Not sure if this is correct but it's here for now.
      AddonClass::Ref<Control> control; // previously pObject
#endif

      long getId() { TRACE; return id; }
      long getButtonCode() { TRACE; return buttonCode; }
      float getAmount1() { TRACE; return fAmount1; }
      float getAmount2() { TRACE; return fAmount2; }
    };

    /**
     * This is the main class for the xbmcgui.Window functionality. It is tied
     *  into the main XBMC windowing system via the Interceptor
     */
    class Window : public AddonCallback
    {
    protected:
#ifndef SWIG
      bool windowCleared;
      InterceptorBase* window;
      int iWindowId;

      std::vector<AddonClass::Ref<Control> > vecControls;
      int iOldWindowId;
      int iCurrentControlId;
      bool bModal; 
      CEvent m_actionEvent;

      bool canPulse;

      // I REALLY hate this ... but it's the simplest fix right now.
      bool existingWindow;
      bool destroyAfterDeInit;

      Window(const char* classname) throw (WindowException);

      virtual void deallocating();

      /**
       * This helper retrieves the next available id. It is assumed 
       *  that the global lock is already being held.
       */
      static int getNextAvailalbeWindowId() throw (WindowException);

      /**
       * Child classes MUST call this in their constructors. It should
       *  be an instance of Interceptor<P extends CGUIWindow>. Control
       *  of memory managment for this class is then given to the
       *  Window
       */
      void setWindow(InterceptorBase* _window);

      /**
       * This is a helper method since poping the
       *  previous window id is a common function
       */
      void popActiveWindowId();

      /**
       * This is a helper method since getting
       *  a control by it's id is a common function
       */
      Control* GetControlById(int iControlId) throw (WindowException);

      SWIGHIDDENVIRTUAL void PulseActionEvent();
      SWIGHIDDENVIRTUAL void WaitForActionEvent();

#endif

    public:
      Window(int existingWindowId = -1) throw (WindowException);

      virtual ~Window();

#ifndef SWIG
      SWIGHIDDENVIRTUAL bool    OnMessage(CGUIMessage& message);
      SWIGHIDDENVIRTUAL bool    OnAction(const CAction &action);
      SWIGHIDDENVIRTUAL bool    OnBack(int actionId);
      SWIGHIDDENVIRTUAL void    OnDeinitWindow(int nextWindowID);

      SWIGHIDDENVIRTUAL bool    IsDialog() const { TRACE; return false; };
      SWIGHIDDENVIRTUAL bool    IsModalDialog() const { TRACE; return false; };
      SWIGHIDDENVIRTUAL bool    IsMediaWindow() const { TRACE; return false; };
#endif

      // callback takes a parameter
      virtual void onAction(Action* action);
      // on control is not actually on Window in the api but is called
      //  into Python anyway. This must result in a problem when 
      virtual void onControl(Control* control);
      virtual void onClick(int controlId);
      virtual void onFocus(int controlId);
      virtual void onInit();

      SWIGHIDDENVIRTUAL void show();
      SWIGHIDDENVIRTUAL void setFocus(Control* pControl) throw (WindowException);
      SWIGHIDDENVIRTUAL void setFocusId(int iControlId);
      SWIGHIDDENVIRTUAL Control* getFocus() throw (WindowException);
      SWIGHIDDENVIRTUAL long getFocusId() throw (WindowException);
      SWIGHIDDENVIRTUAL void removeControl(Control* pControl) throw (WindowException);
      SWIGHIDDENVIRTUAL void removeControls(std::vector<Control*> pControls) throw (WindowException);
      SWIGHIDDENVIRTUAL long getHeight();
      SWIGHIDDENVIRTUAL long getWidth();
      SWIGHIDDENVIRTUAL long getResolution();
      SWIGHIDDENVIRTUAL void setCoordinateResolution(long res) throw (WindowException);
      SWIGHIDDENVIRTUAL void setProperty(const char* key, const String& value);
      SWIGHIDDENVIRTUAL String getProperty(const char* key);
      SWIGHIDDENVIRTUAL void clearProperty(const char* key);
      SWIGHIDDENVIRTUAL void clearProperties();
      SWIGHIDDENVIRTUAL void close();
      SWIGHIDDENVIRTUAL void doModal();
      SWIGHIDDENVIRTUAL void addControl(Control* pControl) throw (WindowException);
      SWIGHIDDENVIRTUAL void addControls(std::vector<Control*> pControls) throw (WindowException);
      SWIGHIDDENVIRTUAL Control* getControl(int iControlId) throw (WindowException);
    };
  }
}
