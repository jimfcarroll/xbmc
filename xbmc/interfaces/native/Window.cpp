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

#include "Window.h"
#include "WindowInterceptor.h"
#include "guilib/GUIButtonControl.h"
#include "guilib/GUIEditControl.h"
#include "guilib/GUICheckMarkControl.h"
#include "guilib/GUIRadioButtonControl.h"
#include "guilib/GUIWindowManager.h"
#include "settings/Settings.h"
#include "Application.h"
#include "ApplicationMessenger.h"
#include "utils/Variant.h"

#define ACTIVE_WINDOW g_windowManager.GetActiveWindow()

namespace XBMCAddon
{
  namespace xbmcgui
  {
    /**
     * Explicit template instantiation
     */
    template class Interceptor<CGUIWindow>;

    /**
     * This interceptor is a simple, non-callbackable (is that a word?)
     *  Interceptor to satisfy the Window requirements for upcalling
     *  for the purposes of instantiating a Window instance from
     *  an already existing window.
     */
    class ProxyExistingWindowInterceptor : public InterceptorBase
    {
      CGUIWindow* cguiwindow;

    public:
      inline ProxyExistingWindowInterceptor(CGUIWindow* window) :
        cguiwindow(window) { }

      virtual CGUIWindow* get();
    };

    CGUIWindow* ProxyExistingWindowInterceptor::get() { return cguiwindow; }

    Window::Window(const char* classname) throw (WindowException): 
      AddonCallback(classname), windowCleared(false), window(NULL), iWindowId(-1),
      iOldWindowId(0), iCurrentControlId(3000), bModal(false), m_actionEvent(true),
      canPulse(true)
    {
      CSingleLock lock(g_graphicsContext);
    }

    /**
     * This just creates a default window.
     */
    Window::Window(int existingWindowId) throw (WindowException) : 
      AddonCallback("Window"), windowCleared(false), window(NULL), iWindowId(-1),
      iOldWindowId(0), iCurrentControlId(3000), bModal(false), m_actionEvent(true),
      canPulse(false), existingWindow(true), destroyAfterDeInit(false)
    {
      CSingleLock lock(g_graphicsContext);

      if (existingWindowId == -1)
      {
        // in this case just do the other constructor.
        canPulse = true;
        existingWindow = false;

        setWindow(new Interceptor<CGUIWindow>("CGUIWindow",this,getNextAvailalbeWindowId()));
      }
      else
      {
        // user specified window id, use this one if it exists
        // It is not possible to capture key presses or button presses
        CGUIWindow* pWindow = g_windowManager.GetWindow(existingWindowId);
        if (!pWindow)
          throw WindowException("Window id does not exist");

        setWindow(new ProxyExistingWindowInterceptor(pWindow));
      }
    }

    Window::~Window()
    {
      deallocating();

      CSingleLock lock(g_graphicsContext);

      // no callbacks are possible any longer
      //   - this will be handled by the parent constructor

      // first change to an existing window
      if (!existingWindow)
      {
        if (ACTIVE_WINDOW == iWindowId && !g_application.m_bStop)
        {
          if(g_windowManager.GetWindow(iOldWindowId))
          {
            g_windowManager.ActivateWindow(iOldWindowId);
          }
          // old window does not exist anymore, switch to home
          else g_windowManager.ActivateWindow(WINDOW_HOME);
        }

        if (window)
        {
          if (g_windowManager.IsWindowVisible(ref(window)->GetID()))
          {
            destroyAfterDeInit = true;
            close();
          }
          else
            g_windowManager.Delete(ref(window)->GetID());

        }
      }
    }

    void Window::deallocating()
    {
      // if the window has not been unhooked, then unhook it.
      {
        Synchronize lock(*this);
        if (window && !windowCleared)
        {
          window->clear();
          windowCleared = true;
        }
      }

      AddonCallback::deallocating();
    }

    void Window::setWindow(InterceptorBase* _window) 
    { 
      window = _window; 
      windowCleared = false; 
      iWindowId = _window->get()->GetID(); 

      if (!existingWindow)
        g_windowManager.Add(window->get());
    }

    /**
     * This helper retrieves the next available id. It is assumed 
     *  that the global lock is already being held.
     */
    int Window::getNextAvailalbeWindowId() throw (WindowException)
    {
      TRACE;
      // window id's 13000 - 13100 are reserved for python
      // get first window id that is not in use
      int id = WINDOW_PYTHON_START;
      // if window 13099 is in use it means python can't create more windows
      if (g_windowManager.GetWindow(WINDOW_PYTHON_END))
        throw WindowException("maximum number of windows reached");

      while(id < WINDOW_PYTHON_END && g_windowManager.GetWindow(id) != NULL) id++;
      return id;
    }

    /**
     * This is a helper method since poping the
     *  previous window id is a common function
     */
    void Window::popActiveWindowId()
    {
      TRACE;
      if (iOldWindowId != iWindowId &&
          iWindowId != ACTIVE_WINDOW)
        iOldWindowId = ACTIVE_WINDOW;
    }

    // Internal helper method
    /* Searches for a control in Window->vecControls
     * If we can't find any but the window has the controlId (in case of a not python window)
     * we create a new control with basic functionality
     */
    Control* Window::GetControlById(int iControlId) throw (WindowException)
    {
      TRACE;
      // find in window vector first!!!
      // this saves us from creating a complete new control
      std::vector<AddonClass::Ref<Control> >::iterator it = vecControls.begin();
      while (it != vecControls.end())
      {
        AddonClass::Ref<Control> control = (*it);
        if (control->iControlId == iControlId)
        {
          return control.get();
        } else ++it;
      }

      // lock xbmc GUI before accessing data from it
      CSingleLock lock(g_graphicsContext);

      // check if control exists
      CGUIControl* pGUIControl = (CGUIControl*)ref(window)->GetControl(iControlId); 
      if (!pGUIControl)
      {
        // control does not exist.
        CStdString error;
        error.Format("Non-Existent Control %d",iControlId);
        throw WindowException(error.c_str());
      }

      // allocate a new control with a new reference
      CLabelInfo li;

      Control* pControl = NULL;

      // TODO: Yuck! Should probably be done with a Factory pattern
      switch(pGUIControl->GetControlType())
      {
      case CGUIControl::GUICONTROL_BUTTON:
        pControl = new ControlButton();

        li = ((CGUIButtonControl *)pGUIControl)->GetLabelInfo();

        // note: conversion from infocolors -> plain colors here
        ((ControlButton*)pControl)->disabledColor = li.disabledColor;
        ((ControlButton*)pControl)->focusedColor  = li.focusedColor;
        ((ControlButton*)pControl)->textColor  = li.textColor;
        ((ControlButton*)pControl)->shadowColor   = li.shadowColor;
        if (li.font) ((ControlButton*)pControl)->strFont = li.font->GetFontName();
        ((ControlButton*)pControl)->align = li.align;
        break;
      case CGUIControl::GUICONTROL_CHECKMARK:
        pControl = new ControlCheckMark();

        li = ((CGUICheckMarkControl *)pGUIControl)->GetLabelInfo();

        // note: conversion to plain colors from infocolors.
        ((ControlCheckMark*)pControl)->disabledColor = li.disabledColor;
        //((ControlCheckMark*)pControl)->shadowColor = li.shadowColor;
        ((ControlCheckMark*)pControl)->textColor  = li.textColor;
        if (li.font) ((ControlCheckMark*)pControl)->strFont = li.font->GetFontName();
        ((ControlCheckMark*)pControl)->align = li.align;
        break;
      case CGUIControl::GUICONTROL_LABEL:
        pControl = new ControlLabel();
        break;
      case CGUIControl::GUICONTROL_SPIN:
        pControl = new ControlSpin();
        break;
      case CGUIControl::GUICONTROL_FADELABEL:
        pControl = new ControlFadeLabel();
        break;
      case CGUIControl::GUICONTROL_TEXTBOX:
        pControl = new ControlTextBox();
        break;
      case CGUIControl::GUICONTROL_IMAGE:
        pControl = new ControlImage();
        break;
      case CGUIControl::GUICONTROL_PROGRESS:
        pControl = new ControlProgress();
        break;
      case CGUIControl::GUICONTROL_SLIDER:
        pControl = new ControlSlider();
        break;			
      case CGUIControl::GUICONTAINER_LIST:
      case CGUIControl::GUICONTAINER_WRAPLIST:
      case CGUIControl::GUICONTAINER_FIXEDLIST:
      case CGUIControl::GUICONTAINER_PANEL:
        pControl = new ControlList();
        // create a python spin control
        ((ControlList*)pControl)->pControlSpin = new ControlSpin();
        break;
      case CGUIControl::GUICONTROL_GROUP:
        pControl = new ControlGroup();
        break;
      case CGUIControl::GUICONTROL_RADIO:
        pControl = new ControlRadioButton();

        li = ((CGUIRadioButtonControl *)pGUIControl)->GetLabelInfo();

        // note: conversion from infocolors -> plain colors here
        ((ControlRadioButton*)pControl)->disabledColor = li.disabledColor;
        ((ControlRadioButton*)pControl)->focusedColor  = li.focusedColor;
        ((ControlRadioButton*)pControl)->textColor  = li.textColor;
        ((ControlRadioButton*)pControl)->shadowColor   = li.shadowColor;
        if (li.font) ((ControlRadioButton*)pControl)->strFont = li.font->GetFontName();
        ((ControlRadioButton*)pControl)->align = li.align;
        break;
      case CGUIControl::GUICONTROL_EDIT:
        pControl = new ControlEdit();

        li = ((CGUIEditControl *)pGUIControl)->GetLabelInfo();

        // note: conversion from infocolors -> plain colors here
        ((ControlEdit*)pControl)->disabledColor = li.disabledColor;
        ((ControlEdit*)pControl)->textColor  = li.textColor;
        if (li.font) ((ControlEdit*)pControl)->strFont = li.font->GetFontName();
        ((ControlButton*)pControl)->align = li.align;
        break;
      default:
        break;
      }

      if (!pControl)
        // throw an exeption
        throw WindowException("Unknown control type for python");

      // we have a valid control here, fill in all the 'Control' data
      pControl->pGUIControl = pGUIControl;
      pControl->iControlId = pGUIControl->GetID();
      pControl->iParentId = iWindowId;
      pControl->dwHeight = (int)pGUIControl->GetHeight();
      pControl->dwWidth = (int)pGUIControl->GetWidth();
      pControl->dwPosX = (int)pGUIControl->GetXPosition();
      pControl->dwPosY = (int)pGUIControl->GetYPosition();
      pControl->iControlUp = pGUIControl->GetControlIdUp();
      pControl->iControlDown = pGUIControl->GetControlIdDown();
      pControl->iControlLeft = pGUIControl->GetControlIdLeft();
      pControl->iControlRight = pGUIControl->GetControlIdRight();

      // It got this far so means the control isn't actually in the vector of controls
      // so lets add it to save doing all that next time
      vecControls.push_back(AddonClass::Ref<Control>(pControl));

      // return the control with increased reference (+1)
      return pControl;
    }

    void Window::PulseActionEvent()
    {
      TRACE;
      if (canPulse)
        m_actionEvent.Set();
    }

    void Window::WaitForActionEvent()
    {
      TRACE;
      DelayedCallGuard dcguard(languageHook);
      if (languageHook)
        languageHook->waitForEvent(m_actionEvent);
      m_actionEvent.Reset();
    }

    /**
     * onAction(self, Action action) -- onAction method.
     * 
     * This method will recieve all actions that the main program will send
     * to this window.
     * By default, only the PREVIOUS_MENU and NAV_BACK actions are handled.
     * Overwrite this method to let your script handle all actions.
     * Don't forget to capture ACTION_PREVIOUS_MENU or ACTION_NAV_BACK, else the user can't close this window.
     */
    bool Window::OnAction(const CAction &action)
    {
      TRACE;
      // do the base class window first, and the call to python after this
      bool ret = ref(window)->OnAction(action);

      // workaround - for scripts which try to access the active control (focused) when there is none.
      // for example - the case when the mouse enters the screen.
      CGUIControl *pControl = ref(window)->GetFocusedControl();
      if (action.IsMouse() && !pControl)
        return ret;

      AddonClass::Ref<Action> inf(new Action(action));
      handleCallback(new VoidCallbackFunction1Ref<Window,Action>(this,&Window::onAction,inf.get()));
      PulseActionEvent();

      return ret;
    }

    bool Window::OnBack(int actionID)
    {
      // if we have a callback window then python handles the closing
      return ref(window)->OnAction(actionID);
    }

    void Window::OnDeinitWindow(int nextWindowID /*= 0*/)
    {
      ref(window)->OnDeinitWindow(nextWindowID);
      if (destroyAfterDeInit)
        g_windowManager.Delete(ref(window)->GetID());
    }

    void Window::onAction(Action* action)
    {
      TRACE;
      // default onAction behavior
      if(action->id == ACTION_PREVIOUS_MENU || action->id == ACTION_NAV_BACK)
        close();
    }

    bool Window::OnMessage(CGUIMessage& message)
    {
      TRACE;
      switch (message.GetMessage())
      {
      case GUI_MSG_WINDOW_DEINIT:
        {
          g_windowManager.ShowOverlay(ref(window)->OVERLAY_STATE_SHOWN);
        }
        break;

      case GUI_MSG_WINDOW_INIT:
        {
          ref(window)->OnMessage(message);
          g_windowManager.ShowOverlay(ref(window)->OVERLAY_STATE_HIDDEN);
          return true;
        }
        break;

      case GUI_MSG_CLICKED:
        {
          int iControl=message.GetSenderId();
          AddonClass::Ref<Control> inf;
          // find python control object with same iControl
          std::vector<AddonClass::Ref<Control> >::iterator it = vecControls.begin();
          while (it != vecControls.end())
          {
            AddonClass::Ref<Control> pControl = (*it);
            if (pControl->iControlId == iControl)
            {
              inf = pControl.get();
              break;
            }
            ++it;
          }

          // did we find our control?
          if (inf.isNotNull())
          {
            // currently we only accept messages from a button or controllist with a select action
            if (inf->canAcceptMessages(message.GetParam1()))
            {
              handleCallback(new VoidCallbackFunction1Ref<Window,Control>(this,&Window::onControl,inf.get()));
              PulseActionEvent();

              // return true here as we are handling the event
              return true;
            }
          }
          // if we get here, we didn't add the action
        }
        break;
      }

      return ref(window)->OnMessage(message);
    }

    void Window::onControl(Control* action) { TRACE; /* do nothing by default */ }
    void Window::onClick(int controlId) { TRACE; /* do nothing by default */ }
    void Window::onFocus(int controlId) { TRACE; /* do nothing by default */ }
    void Window::onInit() { TRACE; /* do nothing by default */ }

    /**
     * show(self) -- Show this window.
     * 
     * Shows this window by activating it, calling close() after it wil activate the
     * current window again.
     * Note, if your script ends this window will be closed to. To show it forever, 
     * make a loop at the end of your script ar use doModal() instead
     */
    void Window::show()
    {
      TRACE;
      DelayedCallGuard dcguard(languageHook);
      popActiveWindowId();

      std::vector<CStdString> params;
      CApplicationMessenger::Get().ActivateWindow(iWindowId, params, false);
    }

    /**
     * setFocus(self, Control) -- Give the supplied control focus.
     * Throws: TypeError, if supplied argument is not a Control type
     *         SystemError, on Internal error
     *         RuntimeError, if control is not added to a window
     */
    void Window::setFocus(Control* pControl) throw (WindowException)
    {
      TRACE;
      if(pControl == NULL)
        throw WindowException("Object should be of type Control");

      CGUIMessage msg = CGUIMessage(GUI_MSG_SETFOCUS,pControl->iParentId, pControl->iControlId);
      g_windowManager.SendThreadMessage(msg, pControl->iParentId);
    }

    /**
     * setFocusId(self, int) -- Gives the control with the supplied focus.
     * Throws: 
     *         SystemError, on Internal error
     *         RuntimeError, if control is not added to a window
     */
    void Window::setFocusId(int iControlId)
    {
      TRACE;
      CGUIMessage msg = CGUIMessage(GUI_MSG_SETFOCUS,iWindowId,iControlId);
      g_windowManager.SendThreadMessage(msg, iWindowId);
    }

    /**
     * getFocus(self, Control) -- returns the control which is focused.
     * Throws: SystemError, on Internal error
     *         RuntimeError, if no control has focus
     */
    Control* Window::getFocus() throw (WindowException)
    {
      TRACE;
      CSingleLock lock(g_graphicsContext);

      int iControlId = ref(window)->GetFocusedControlID();
      if(iControlId == -1)
        throw WindowException("No control in this window has focus");
      lock.Leave();
      return GetControlById(iControlId);
    }

    /**
     * getFocusId(self, int) -- returns the id of the control which is focused.
     * Throws: SystemError, on Internal error
     *         RuntimeError, if no control has focus
     */
    long Window::getFocusId() throw (WindowException)
    {
      TRACE;
      CSingleLock lock(g_graphicsContext);
      int iControlId = ref(window)->GetFocusedControlID();
      if(iControlId == -1)
        throw WindowException("No control in this window has focus");
      return (long)iControlId;
    }

    /**
     * removeControl(self, Control) -- Removes the control from this window.
     * 
     * Throws: TypeError, if supplied argument is not a Control type
     *         RuntimeError, if control is not added to this window
     * 
     * This will not delete the control. It is only removed from the window.
     */
    void Window::removeControl(Control* pControl) throw (WindowException)
    {
      TRACE;
      // type checking, object should be of type Control
      if(pControl == NULL)
        throw WindowException("Object should be of type Control");

      CSingleLock lock(g_graphicsContext);
      if(!ref(window)->GetControl(pControl->iControlId))
        throw WindowException("Control does not exist in window");

      // delete control from vecControls in window object
      std::vector<AddonClass::Ref<Control> >::iterator it = vecControls.begin();
      while (it != vecControls.end())
      {
        AddonClass::Ref<Control> control = (*it);
        if (control->iControlId == pControl->iControlId)
        {
          it = vecControls.erase(it);
        } else ++it;
      }

      CGUIMessage msg(GUI_MSG_REMOVE_CONTROL, 0, 0);
      msg.SetPointer(pControl->pGUIControl);
      CApplicationMessenger::Get().SendGUIMessage(msg, iWindowId, true);

      // initialize control to zero
      pControl->pGUIControl = NULL;
      pControl->iControlId = 0;
      pControl->iParentId = 0;
    }

    /**
     * removeControls(self, List) -- Removes a list of controls from this window.
     *
     * Throws: TypeError, if supplied argument is not a Control type
     *        RuntimeError, if control is not added to this window
     *
     * This will not delete the controls. They are only removed from the window.
     */
    void Window::removeControls(std::vector<Control*> pControls) throw (WindowException)
    {
      for (std::vector<Control*>::iterator iter = pControls.begin(); iter != pControls.end(); iter++)
        removeControl(*iter);
    }

    /**
     * getHeight(self) -- Returns the height of this screen.
     */
    long Window::getHeight()
    {
      TRACE;
      return g_graphicsContext.GetHeight();
    }

    /**
     * getWidth(self) -- Returns the width of this screen.
     */
    long Window::getWidth()
    {
      TRACE;
      return g_graphicsContext.GetWidth();
    }

    /**
     * getResolution(self) -- Returns the resolution of the scree
     *  The returned value is one of the following:
     *    0 - 1080i      (1920x1080)
     *    1 - 720p       (1280x720)
     *    2 - 480p 4:3   (720x480)
     *    3 - 480p 16:9  (720x480)
     *    4 - NTSC 4:3   (720x480)
     *    5 - NTSC 16:9  (720x480)
     *    6 - PAL 4:3    (720x576)
     *    7 - PAL 16:9   (720x576)
     *    8 - PAL60 4:3  (720x480)
     *    9 - PAL60 16:9 (720x480)\n
     */
    long Window::getResolution()
    {
      TRACE;
      return (long)g_graphicsContext.GetVideoResolution();
    }

    /**
     * setCoordinateResolution(self, int resolution) -- Sets the resolution
     * that the coordinates of all controls are defined in.  Allows XBMC
     * to scale control positions and width/heights to whatever resolution
     * XBMC is currently using.
     *  resolution is one of the following:
     *    0 - 1080i      (1920x1080)
     *    1 - 720p       (1280x720)
     *    2 - 480p 4:3   (720x480)
     *    3 - 480p 16:9  (720x480)
     *    4 - NTSC 4:3   (720x480)
     *    5 - NTSC 16:9  (720x480)
     *    6 - PAL 4:3    (720x576)
     *    7 - PAL 16:9   (720x576)
     *    8 - PAL60 4:3  (720x480)
     *    9 - PAL60 16:9 (720x480)\n
     */
    void Window::setCoordinateResolution(long res) throw (WindowException)
    {
      TRACE;
      if (res < RES_HDTV_1080i || res > RES_AUTORES)
        throw WindowException("Invalid resolution.");

      CSingleLock lock(g_graphicsContext);
      ref(window)->SetCoordsRes(g_settings.m_ResInfo[res]);
    }

    // setProperty() method
    /**
     * setProperty(key, value) -- Sets a window property, similar to an infolabel.
     * 
     * key            : string - property name.
     * value          : string or unicode - value of property.
     * 
     * *Note, key is NOT case sensitive. Setting value to an empty string is equivalent to clearProperty(key)
     *        You can use the above as keywords for arguments and skip certain optional arguments.
     *        Once you use a keyword, all following arguments require the keyword.
     * 
     * example:
     *   - win = xbmcgui.Window(xbmcgui.getCurrentWindowId())
     *   - win.setProperty('Category', 'Newest')\n
     */
    void Window::setProperty(const char* key, const String& value)
    {
      TRACE;
      CSingleLock lock(g_graphicsContext);
      CStdString lowerKey = key;

      ref(window)->SetProperty(lowerKey.ToLower(), value);
    }

    /**
     * getProperty(key) -- Returns a window property as a string, similar to an infolabel.
     * 
     * key            : string - property name.
     * 
     * *Note, key is NOT case sensitive.
     *        You can use the above as keywords for arguments and skip certain optional arguments.
     *        Once you use a keyword, all following arguments require the keyword.
     * 
     * example:
     *   - win = xbmcgui.Window(xbmcgui.getCurrentWindowId())
     *   - category = win.getProperty('Category')
     */
    String Window::getProperty(const char* key)
    {
      TRACE;
      CSingleLock lock(g_graphicsContext);
      CStdString lowerKey = key;
      std::string value = ref(window)->GetProperty(lowerKey.ToLower()).asString();
      return value.c_str();
    }

    /**
     * clearProperty(key) -- Clears the specific window property.
     * 
     * key            : string - property name.
     * 
     * *Note, key is NOT case sensitive. Equivalent to setProperty(key,'')
     *        You can use the above as keywords for arguments and skip certain optional arguments.
     *        Once you use a keyword, all following arguments require the keyword.
     * 
     * example:
     *   - win = xbmcgui.Window(xbmcgui.getCurrentWindowId())
     *   - win.clearProperty('Category')\n
     */
    void Window::clearProperty(const char* key)
    {
      TRACE;
      if (!key) return;
      CSingleLock lock(g_graphicsContext);

      CStdString lowerKey = key;
      ref(window)->SetProperty(lowerKey.ToLower(), "");
    }

    /**
     * clearProperties() -- Clears all window properties.
     * 
     * example:
     *   - win = xbmcgui.Window(xbmcgui.getCurrentWindowId())
     *   - win.clearProperties()\n
     */
    void Window::clearProperties()
    {
      TRACE;
      CSingleLock lock(g_graphicsContext);
      ref(window)->ClearProperties();
    }

    /**
     * close(self) -- Closes this window.
     * 
     * Closes this window by activating the old window.
     * The window is not deleted with this method.
     */
    //Py_BEGIN_ALLOW_THREADS -delayed
    void Window::close()
    {
      TRACE;
      bModal = false;

      PulseActionEvent();

      std::vector<CStdString> params;
      CApplicationMessenger::Get().ActivateWindow(iOldWindowId, params, false);

      iOldWindowId = 0;
    }

    /**
     * doModal(self) -- Display this window until close() is called.
     */
    void Window::doModal()
    {
      TRACE;
      if (!existingWindow)
      {
        bModal = true;

        if(iWindowId != ACTIVE_WINDOW) 
          show();

        while (bModal && !g_application.m_bStop)
        {
// TODO: garbear added this code to the pythin window.cpp class and
//  commented in XBPyThread.cpp. I'm not sure how to handle this 
//  in this native implementation.
//          // Check if XBPyThread::stop() raised a SystemExit exception
//          if (PyThreadState_Get()->async_exc == PyExc_SystemExit)
//          {
//            CLog::Log(LOGDEBUG, "PYTHON: doModal() encountered a SystemExit exception, closing window and returning");
//            Window_Close(self, NULL);
//            break;
//          }
          WaitForActionEvent();
        }
      }
    }

    /**
     * addControl(self, Control) -- Add a Control to this window.
     * 
     * Throws: TypeError, if supplied argument is not a Control type
     *         ReferenceError, if control is already used in another window
     *         RuntimeError, should not happen :-)
     * 
     * The next controls can be added to a window atm
     * 
     *   -ControlLabel
     *   -ControlFadeLabel
     *   -ControlTextBox
     *   -ControlButton
     *   -ControlCheckMark
     *   -ControlList
     *   -ControlGroup
     *   -ControlImage
     *   -ControlRadioButton
     *   -ControlProgress\n
     */
    void Window::addControl(Control* pControl) throw (WindowException)
    {
      TRACE;
      if(pControl == NULL)
        throw new WindowException("NULL Control passed to WindowBase::addControl");

      if(pControl->iControlId != 0)
        throw new WindowException("Control is already used");

      // lock xbmc GUI before accessing data from it
      CSingleLock lock(g_graphicsContext);
      pControl->iParentId = iWindowId;
      // assign control id, if id is already in use, try next id
      // TODO: This is not thread safe
      do pControl->iControlId = ++iCurrentControlId;
      while (ref(window)->GetControl(pControl->iControlId));

      pControl->Create();

      // set default navigation for control
      pControl->iControlUp = pControl->iControlId;
      pControl->iControlDown = pControl->iControlId;
      pControl->iControlLeft = pControl->iControlId;
      pControl->iControlRight = pControl->iControlId;

      pControl->pGUIControl->SetNavigation(pControl->iControlUp,
          pControl->iControlDown, pControl->iControlLeft, pControl->iControlRight);

      // add control to list and allocate recources for the control
      vecControls.push_back(AddonClass::Ref<Control>(pControl));
      pControl->pGUIControl->AllocResources();

      // This calls the CGUIWindow parent class to do the final add
      CGUIMessage msg(GUI_MSG_ADD_CONTROL, 0, 0);
      msg.SetPointer(pControl->pGUIControl);
      CApplicationMessenger::Get().SendGUIMessage(msg, iWindowId, true);
    }

    /**
     * addControls(self, List) -- Add a list of Controls to this window.
     *
     *Throws: TypeError, if supplied argument is not of List type, or a control is not of Control type
     *        ReferenceError, if control is already used in another window
     *        RuntimeError, should not happen :-)
     */
    void Window::addControls(std::vector<Control*> pControls) throw (WindowException)
    {
      for (std::vector<Control*>::iterator iter = pControls.begin(); iter != pControls.end(); iter++)
        addControl(*iter);
    }

    /**
     * getControl(self, int controlId) -- Get's the control from this window.
     * 
     * Throws: Exception, if Control doesn't exist
     * 
     * controlId doesn't have to be a python control, it can be a control id
     * from a xbmc window too (you can find id's in the xml files
     * 
     * Note, not python controls are not completely usable yet
     * You can only use the Control functions
     */
    Control* Window::getControl(int iControlId) throw (WindowException)
    {
      TRACE;
      return GetControlById(iControlId);
    }


    void Action::setFromCAction(const CAction& action)
    {
      TRACE;
      id = action.GetID();
      buttonCode = action.GetButtonCode();
      fAmount1 = action.GetAmount(0);
      fAmount2 = action.GetAmount(1);
      fRepeat = action.GetRepeat();
      strAction = action.GetName();
    }

  }
}
