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

#include "WindowDialog.h"

#include "guilib/GUIWindow.h"
#include "guilib/GUIWindowManager.h"
#include "ApplicationMessenger.h"

#define HACK_CUSTOM_ACTION_CLOSING -3
#define HACK_CUSTOM_ACTION_OPENING -4


namespace XBMCAddon
{
  namespace xbmcgui
  {

    WindowDialog::WindowDialog() throw(WindowException) :
      Window("WindowDialog")
    {
      CSingleLock lock(g_graphicsContext);
      setWindow(new Interceptor<CGUIWindow>("CGUIWindow",this,getNextAvailalbeWindowId()));
    }

    WindowDialog::~WindowDialog() { deallocating(); }

    bool WindowDialog::OnMessage(CGUIMessage& message)
    {
      TRACE;
      CLog::Log(LOGDEBUG,"NEWADDON WindowDialog::OnMessage Message %d", message.GetMessage());

      switch(message.GetMessage())
      {
      case GUI_MSG_WINDOW_INIT:
        {
          ref(window)->OnMessage(message);
          return true;
        }
        break;

      case GUI_MSG_CLICKED:
        {
          return Window::OnMessage(message);
        }
        break;
      }

      // we do not message CGUIPythonWindow here..
      return ref(window)->OnMessage(message);
    }

    // TODO: Move this into a Mixin since it's the same code as in WindowDialog
    // This used the hack:
    //   ThreadMessage tMsg = {TMSG_GUI_PYTHON_DIALOG, 1, 1};
    // but we're going to try and fix that here.
    void WindowDialog::show()
    {
      TRACE;
      DelayedCallGuard dcguard(languageHook);
//      // Previously the code did this in response to a 'show' when the 
//      //   Window was a Python dialog:
//      //ThreadMessage tMsg = {TMSG_GUI_PYTHON_DIALOG, 1, 1};
//      //tMsg.lpVoid = self->pWindow;
//      //CApplicationMessenger::Get().SendMessage(tMsg, true);
//      //
//      // however, the 2nd parameter to SendMessage is true, so the call
//      //  went straight to ProcessMessage in the ApplicationMessenger
//      //  after setting a 'wait' event.
//      //
//      // ProcessMessage then switched on the TMSG_GUI_PYTHON_DIALOG
//      //   and executes:
//      // 
//      // if (pMsg->dwParam1)
//      //  ((CGUIPythonWindowXMLDialog *)pMsg->lpVoid)->Show_Internal(pMsg->dwParam2 > 0);
//      // else
//      //  ((CGUIPythonWindowDialog *)pMsg->lpVoid)->Show_Internal(pMsg->dwParam2 > 0);
//      // 
//      // dwparam1 is '1' so it sends the message to the Show_Internal on a 
//      //   CGUIPythonWindowXMLDialog (even if the object is a CGUIPythonWindowDialog
//      //   - see bug 10425). Show_Internal does the same thing for either class.
//      // to send an INIT message directly to the OnMessage method after setting
//      //   RouteToWindow to 'this' .. .so that's all we're going to do
//      amRunning = true;
//      g_windowManager.RouteToWindow(interceptor);
//      // active this dialog...
//      CGUIMessage msg(GUI_MSG_WINDOW_INIT,0,0);
//      OnMessage(msg);

//      // Now we try this
//      ThreadMessage tMsg = {TMSG_GUI_SHOW, 0, 0};
//      tMsg.lpVoid = new DialogJumper(this,ref(window)->GetID() + 1,false);
//      CApplicationMessenger::Get().SendMessage(tMsg, true);

// Instead of the above we are going to create a custom action and 
      ThreadMessage tMsg = {TMSG_GUI_PYTHON_DIALOG, HACK_CUSTOM_ACTION_OPENING, 0};
      tMsg.lpVoid = window->get();
      CApplicationMessenger::Get().SendMessage(tMsg, true);
    }

    // TODO: Move this into a Mixin since it's the same code as in WindowDialog
    void WindowDialog::close()
    {
      TRACE;
      DelayedCallGuard dcguard(languageHook);
      bModal = false;
      PulseActionEvent();

//      // see the 'show' for a tedious explanation of problems with the
//      //  call to 'Show_Internal.' The exact same thing happens on this
//      //  side of the window's lifecycle.
//      CGUIMessage msg(GUI_MSG_WINDOW_DEINIT,0,0);
//      OnMessage(msg);
//
//      g_windowManager.RemoveDialog(ref(window)->GetID());
//      amRunning = false;

//      ThreadMessage tMsg = {TMSG_GUI_DIALOG_CLOSE, 1, 0};
//      tMsg.lpVoid = new DialogJumper(this,ref(window)->GetID() + 1,true);
//      CApplicationMessenger::Get().SendMessage(tMsg, true);

// Instead of the above we are going to create a custom action and 
      ThreadMessage tMsg = {TMSG_GUI_PYTHON_DIALOG, HACK_CUSTOM_ACTION_CLOSING, 0};
      tMsg.lpVoid = window->get();
      CApplicationMessenger::Get().SendMessage(tMsg, true);

    }

    bool WindowDialog::OnAction(const CAction &action)
    {
      TRACE;

      switch (action.GetID())
      {
      case HACK_CUSTOM_ACTION_OPENING:
        {
          // This is from the CGUIPythonWindowXMLDialog::Show_Internal
          g_windowManager.RouteToWindow(ref(window).get());
          // active this dialog...
          CGUIMessage msg(GUI_MSG_WINDOW_INIT,0,0);
          OnMessage(msg);
          // TODO: Figure out how to clean up the CAction
          return true;
        }
        break;
        
      case HACK_CUSTOM_ACTION_CLOSING:
        {
          // This is from the CGUIPythonWindowXMLDialog::Show_Internal
          close();
          // TODO: Figure out how to clean up the CAction
          return true;
        }
        break;
      }

      return Window::OnAction(action);
    }

    void WindowDialog::OnDeinitWindow(int nextWindowID)
    {
      g_windowManager.RemoveDialog(iWindowId);
      Window::OnDeinitWindow(nextWindowID);
    }

  }
}
