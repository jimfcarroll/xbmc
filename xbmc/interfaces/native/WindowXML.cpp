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

#include "WindowXML.h"

#include "WindowInterceptor.h"
#include "guilib/GUIWindowManager.h"
#include "settings/GUISettings.h"
#include "addons/Skin.h"
#include "filesystem/File.h"
#include "ApplicationMessenger.h"
#include "utils/URIUtils.h"
#include "addons/Addon.h"

// These #defs are for WindowXML
#define CONTROL_BTNVIEWASICONS  2
#define CONTROL_BTNSORTBY       3
#define CONTROL_BTNSORTASC      4
#define CONTROL_LABELFILES      12

#define A(x) interceptor->x

#define HACK_CUSTOM_ACTION_CLOSING -3
#define HACK_CUSTOM_ACTION_OPENING -4

namespace XBMCAddon
{
  namespace xbmcgui
  {
    template class Interceptor<CGUIMediaWindow>;

    /**
     * This class extends the Interceptor<CGUIMediaWindow> in order to 
     *  add behavior for a few more virtual functions that were unneccessary
     *  in the Window or WindowDialog.
     */
#define checkedb(methcall) ( window ? xwin-> methcall : false )
#define checkedv(methcall) { if (window) xwin-> methcall ; }

    class WindowXMLInterceptor : public InterceptorDialog<CGUIMediaWindow>
    {
      WindowXML* xwin;
    public:
      WindowXMLInterceptor(WindowXML* _window, int windowid,const char* xmlfile) :
        InterceptorDialog<CGUIMediaWindow>("CGUIMediaWindow",_window,windowid,xmlfile), xwin(_window) 
      { }

      virtual void AllocResources(bool forceLoad = false)
      { TRACE; if(up()) CGUIMediaWindow::AllocResources(forceLoad); else checkedv(AllocResources(forceLoad)); }
      virtual  void FreeResources(bool forceUnLoad = false)
      { TRACE; if(up()) CGUIMediaWindow::FreeResources(forceUnLoad); else checkedv(FreeResources(forceUnLoad)); }
      virtual bool OnClick(int iItem) { TRACE; return up() ? CGUIMediaWindow::OnClick(iItem) : checkedb(OnClick(iItem)); }

      // this is a hack to SKIP the CGUIMediaWindow
      virtual bool OnAction(const CAction &action) 
      { TRACE; return up() ? CGUIWindow::OnAction(action) : checkedb(OnAction(action)); }

    protected:
      // CGUIWindow
      virtual bool LoadXML(const CStdString &strPath, const CStdString &strPathLower)
      { TRACE; return up() ? CGUIMediaWindow::LoadXML(strPath,strPathLower) : xwin->LoadXML(strPath,strPathLower); }

      // CGUIMediaWindow
      virtual void GetContextButtons(int itemNumber, CContextButtons &buttons)
      { TRACE; if (up()) CGUIMediaWindow::GetContextButtons(itemNumber,buttons); else xwin->GetContextButtons(itemNumber,buttons); }
      virtual bool Update(const CStdString &strPath)
      { TRACE; return up() ? CGUIMediaWindow::Update(strPath) : xwin->Update(strPath); }

      friend class WindowXML;
      friend class WindowXMLDialog;

    };

    WindowXML::WindowXML(const String& xmlFilename,
                         const String& scriptPath,
                         const String& defaultSkin,
                         const String& defaultRes) throw(WindowException) :
      Window("WindowXML")
    {
      initialize(xmlFilename,scriptPath,defaultSkin,defaultRes);
    }

    WindowXML::WindowXML(const char* classname, 
                         const String& xmlFilename,
                         const String& scriptPath,
                         const String& defaultSkin,
                         const String& defaultRes) throw(WindowException) :
      Window(classname)
    {
      initialize(xmlFilename,scriptPath,defaultSkin,defaultRes);
    }

    WindowXML::~WindowXML() { deallocating();  }

    void WindowXML::initialize(const String& xmlFilename,
                         const String& scriptPath,
                         const String& defaultSkin,
                         const String& defaultRes)
    {
      RESOLUTION_INFO res;
      CStdString strSkinPath = g_SkinInfo->GetSkinPath(xmlFilename, &res);

      CLog::Log(LOGDEBUG,"NEWADDON Skin Path1:%s",strSkinPath.c_str());

      if (!XFILE::CFile::Exists(strSkinPath))
      {
        CStdString str("none");
        ADDON::AddonProps props(str, ADDON::ADDON_SKIN, "", "");
        ADDON::CSkinInfo::TranslateResolution(defaultRes, res);

        // Check for the matching folder for the skin in the fallback skins folder
        CStdString fallbackPath = URIUtils::AddFileToFolder(scriptPath, "resources");
        fallbackPath = URIUtils::AddFileToFolder(fallbackPath, "skins");
        CStdString basePath = URIUtils::AddFileToFolder(fallbackPath, g_SkinInfo->ID());

        strSkinPath = g_SkinInfo->GetSkinPath(xmlFilename, &res, basePath);

        // Check for the matching folder for the skin in the fallback skins folder (if it exists)
        if (XFILE::CFile::Exists(basePath))
        {
          props.path = basePath;
          ADDON::CSkinInfo skinInfo(props, res);
          skinInfo.Start();
          strSkinPath = skinInfo.GetSkinPath(xmlFilename, &res);
        }

        if (!XFILE::CFile::Exists(strSkinPath))
        {
          // Finally fallback to the DefaultSkin as it didn't exist in either the XBMC Skin folder or the fallback skin folder
          props.path = URIUtils::AddFileToFolder(fallbackPath, defaultSkin);
          ADDON::CSkinInfo skinInfo(props, res);

          skinInfo.Start();
          strSkinPath = skinInfo.GetSkinPath(xmlFilename, &res);
          if (!XFILE::CFile::Exists(strSkinPath))
            throw WindowException("XML File for Window is missing");
        }
      }

      m_scriptPath = scriptPath;
//      sXMLFileName = strSkinPath;

      interceptor = new WindowXMLInterceptor(this, lockingGetNextAvailalbeWindowId(),strSkinPath.c_str());
      setWindow(interceptor);
      interceptor->SetCoordsRes(res);
    }

    int WindowXML::lockingGetNextAvailalbeWindowId() throw (WindowException)
    {
      TRACE;
      CSingleLock lock(g_graphicsContext);
      return getNextAvailalbeWindowId();
    }

    void WindowXML::addItem(const String& item, int pos)
    {
      TRACE;
      AddonClass::Ref<ListItem> ritem(ListItem::fromString(item));
      addListItem(ritem.get(),pos);
    }

    void WindowXML::addListItem(ListItem* item, int pos)
    {
      TRACE;
      // item could be deleted if the reference count is 0.
      //   so I MAY need to check prior to using a Ref just in
      //   case this object is managed by Python. I'm not sure
      //   though.
      AddonClass::Ref<ListItem> ritem(item);

      // Tells the window to add the item to FileItem vector
      {
        LOCKGUI;

        //----------------------------------------------------
        // Former AddItem call
        //AddItem(ritem->item, pos);
        {
          CFileItemPtr& fileItem = ritem->item;
          if (pos == INT_MAX || pos > A(m_vecItems)->Size())
          {
            A(m_vecItems)->Add(fileItem);
          }
          else if (pos <  -1 &&  !(pos*-1 < A(m_vecItems)->Size()))
          {
            A(m_vecItems)->AddFront(fileItem,0);
          }
          else
          {
            A(m_vecItems)->AddFront(fileItem,pos);
          }
          A(m_viewControl).SetItems(*(A(m_vecItems)));
          A(UpdateButtons());
        }
        //----------------------------------------------------
      }
    }

    void WindowXML::removeItem(int position)
    {
      TRACE;
      // Tells the window to remove the item at the specified position from the FileItem vector
      LOCKGUI;
      A(m_vecItems)->Remove(position);
      A(m_viewControl).SetItems(*(A(m_vecItems)));
      A(UpdateButtons());
    }

    int WindowXML::getCurrentListPosition()
    {
      TRACE;
      LOCKGUI;
      int listPos = A(m_viewControl).GetSelectedItem();
      return listPos;
    }

    void WindowXML::setCurrentListPosition(int position)
    {
      TRACE;
      LOCKGUI;
      A(m_viewControl).SetSelectedItem(position);
    }

    ListItem* WindowXML::getListItem(int position) throw (WindowException)
    {
      LOCKGUI;
      //CFileItemPtr fi = pwx->GetListItem(listPos);
      CFileItemPtr fi;
      {
        if (position < 0 || position >= A(m_vecItems)->Size()) 
          return new ListItem();
        fi = A(m_vecItems)->Get(position);
      }

      if (fi == NULL)
      {
        XBMCAddonUtils::guiUnlock();
        throw WindowException("Index out of range (%i)",position);
      }

      ListItem* sListItem = new ListItem();
      sListItem->item = fi;

      // let's hope someone reference counts this.
      return sListItem;
    }

    int WindowXML::getListSize()
    {
      TRACE;
      return A(m_vecItems)->Size();
    }

    void WindowXML::clearList()
    {
      TRACE;
      A(ClearFileItems());

      A(m_viewControl).SetItems(*(A(m_vecItems)));
      A(UpdateButtons());
    }

    void WindowXML::setProperty(const String& key, const String& value)
    {
      TRACE;
      A(m_vecItems)->SetProperty(key, value);
    }

    bool WindowXML::OnAction(const CAction &action)
    {
      TRACE;
      // do the base class window first, and the call to python after this
      bool ret = ref(window)->OnAction(action);  // we don't currently want the mediawindow actions here
                                                 //  look at the WindowXMLInterceptor onAction, it skips
                                                 //  the CGUIMediaWindow::OnAction and calls directly to
                                                 //  CGUIWindow::OnAction
      AddonClass::Ref<Action> inf(new Action(action));
      handleCallback(new CallbackFunction<WindowXML,AddonClass::Ref<Action> >(this,&WindowXML::onAction,inf.get()));
      PulseActionEvent();
      return ret;
    }

    bool WindowXML::OnMessage(CGUIMessage& message)
    {
      TRACE;

      // TODO: We shouldn't be dropping down to CGUIWindow in any of this ideally.
      //       We have to make up our minds about what python should be doing and
      //       what this side of things should be doing
      switch (message.GetMessage())
      {
      case GUI_MSG_WINDOW_DEINIT:
        {
          return ref(window)->OnMessage(message);
        }
        break;

      case GUI_MSG_WINDOW_INIT:
        {
          ref(window)->OnMessage(message);
          handleCallback(new CallbackFunction<WindowXML>(this,&WindowXML::onInit));
          return true;
        }
        break;

      case GUI_MSG_FOCUSED:
        {
          if (A(m_viewControl).HasControl(message.GetControlId()) && 
              A(m_viewControl).GetCurrentControl() != (int)message.GetControlId())
          {
            A(m_viewControl).SetFocused();
            return true;
          }
          // check if our focused control is one of our category buttons
          int iControl=message.GetControlId();

          AddonClass::Ref<Control> inf(GetControlById(iControl));
          if (inf.isNotNull())
          {
            handleCallback(new CallbackFunction<WindowXML,int>(this,&WindowXML::onFocus,iControl));
            PulseActionEvent();
          }
        }
        break;

      case GUI_MSG_CLICKED:
        {
          int iControl=message.GetSenderId();
          // Handle Sort/View internally. Scripters shouldn't use ID 2, 3 or 4.
          if (iControl == CONTROL_BTNSORTASC) // sort asc
          {
            CLog::Log(LOGINFO, "WindowXML: Internal asc/dsc button not implemented");
            /*if (m_guiState.get())
              m_guiState->SetNextSortOrder();
              UpdateFileList();*/
            return true;
          }
          else if (iControl == CONTROL_BTNSORTBY) // sort by
          {
            CLog::Log(LOGINFO, "WindowXML: Internal sort button not implemented");
            /*if (m_guiState.get())
              m_guiState->SetNextSortMethod();
              UpdateFileList();*/
            return true;
          }

          if(iControl && iControl != (int)interceptor->GetID()) // pCallbackWindow &&  != this->GetID())
          {
            CGUIControl* controlClicked = (CGUIControl*)interceptor->GetControl(iControl);

            // The old python way used to check list AND SELECITEM method 
            //   or if its a button, checkmark.
            // Its done this way for now to allow other controls without a 
            //  python version like togglebutton to still raise a onAction event
            if (controlClicked) // Will get problems if we the id is not on the window 
                                //   and we try to do GetControlType on it. So check to make sure it exists
            {
              if ((controlClicked->IsContainer() && (message.GetParam1() == ACTION_SELECT_ITEM || message.GetParam1() == ACTION_MOUSE_LEFT_CLICK)) || !controlClicked->IsContainer())
              {
                AddonClass::Ref<Control> inf(GetControlById(iControl));
                if (inf.isNotNull())
                {
                  handleCallback(new CallbackFunction<WindowXML,int>(this,&WindowXML::onClick,iControl));
                  PulseActionEvent();
                }
                return true;
              }
              else if (controlClicked->IsContainer() && message.GetParam1() == ACTION_MOUSE_RIGHT_CLICK)
              {
                AddonClass::Ref<Action> inf(new Action(CAction(ACTION_CONTEXT_MENU)));
                handleCallback(new CallbackFunction<WindowXML,AddonClass::Ref<Action> >(this,&WindowXML::onAction,inf.get()));
                PulseActionEvent();
                return true;
              }
            }
          }
        }
        break;
      }

      return ref(window)->OnMessage(message);
    }

    void WindowXML::AllocResources(bool forceLoad /*= FALSE */)
    {
      TRACE;
      CStdString tmpDir;
      URIUtils::GetDirectory(ref(window)->GetProperty("xmlfile").asString(), tmpDir);
      CStdString fallbackMediaPath;
      URIUtils::GetParentPath(tmpDir, fallbackMediaPath);
      URIUtils::RemoveSlashAtEnd(fallbackMediaPath);
      m_mediaDir = fallbackMediaPath;

      //CLog::Log(LOGDEBUG, "CGUIPythonWindowXML::AllocResources called: %s", fallbackMediaPath.c_str());
      g_TextureManager.AddTexturePath(m_mediaDir);
      ref(window)->AllocResources(forceLoad);
      g_TextureManager.RemoveTexturePath(m_mediaDir);
    }

    void WindowXML::FreeResources(bool forceUnLoad /*= FALSE */)
    {
      TRACE;
      // Unload temporary language strings
      ClearScriptStrings();

      ref(window)->FreeResources(forceUnLoad);
    }

    void WindowXML::Process(unsigned int currentTime, CDirtyRegionList &regions)
    {
      TRACE;
      g_TextureManager.AddTexturePath(m_mediaDir);
      ref(window)->Process(currentTime, regions);
      g_TextureManager.RemoveTexturePath(m_mediaDir);
    }

    bool WindowXML::OnClick(int iItem) 
    {
      TRACE;
      // Hook Over calling  CGUIMediaWindow::OnClick(iItem) results in it trying to PLAY the file item
      // which if its not media is BAD and 99 out of 100 times undesireable.
      return false;
    }

    void WindowXML::GetContextButtons(int itemNumber, CContextButtons &buttons)
    {
      TRACE;
      // maybe on day we can make an easy way to do this context menu
      // with out this method overriding the MediaWindow version, it will display 'Add to Favorites'
    }

    bool WindowXML::LoadXML(const String &strPath, const String &strLowerPath)
    {
      TRACE;
      // load our window
      XFILE::CFile file;
      if (!file.Open(strPath) && !file.Open(CStdString(strPath).ToLower()) && !file.Open(strLowerPath))
      {
        // fail - can't load the file
        CLog::Log(LOGERROR, "%s: Unable to load skin file %s", __FUNCTION__, strPath.c_str());
        return false;
      }
      // load the strings in
      unsigned int offset = LoadScriptStrings();

      CStdString xml;
      char *buffer = new char[(unsigned int)file.GetLength()+1];
      if(buffer == NULL)
        return false;
      int size = file.Read(buffer, file.GetLength());
      if (size > 0)
      {
        buffer[size] = 0;
        xml = buffer;
        if (offset)
        {
          // replace the occurences of SCRIPT### with offset+###
          // not particularly efficient, but it works
          int pos = xml.Find("SCRIPT");
          while (pos != (int)CStdString::npos)
          {
            CStdString num = xml.Mid(pos + 6, 4);
            int number = atol(num.c_str());
            CStdString oldNumber, newNumber;
            oldNumber.Format("SCRIPT%d", number);
            newNumber.Format("%lu", offset + number);
            xml.Replace(oldNumber, newNumber);
            pos = xml.Find("SCRIPT", pos + 6);
          }
        }
      }
      delete[] buffer;

      CXBMCTinyXML xmlDoc;
      xmlDoc.Parse(xml.c_str());

      if (xmlDoc.Error())
        return false;

      return interceptor->Load(xmlDoc);
    }

    unsigned int WindowXML::LoadScriptStrings()
    {
      TRACE;
      // Path where the language strings reside
      CStdString pathToLanguageFile = m_scriptPath;
      URIUtils::AddFileToFolder(pathToLanguageFile, "resources", pathToLanguageFile);
      URIUtils::AddFileToFolder(pathToLanguageFile, "language", pathToLanguageFile);
      URIUtils::AddSlashAtEnd(pathToLanguageFile);

      // allocate a bunch of strings
      return g_localizeStrings.LoadBlock(m_scriptPath, pathToLanguageFile, g_guiSettings.GetString("locale.language"));
    }

    void WindowXML::ClearScriptStrings()
    {
      TRACE;
      // Unload temporary language strings
      g_localizeStrings.ClearBlock(m_scriptPath);
    }

    bool WindowXML::Update(const String &strPath)
    {
      TRACE;
      return true;
    }

    bool WindowXML::IsDialogRunning() const
    { 
      return window->isActive();
    }
//    // SetupShares();
//    /*
//      CGUIMediaWindow::OnWindowLoaded() calls SetupShares() so override it
//      and just call UpdateButtons();
//    */
//    void WindowXML::SetupShares()
//    {
//      TRACE;
//      UpdateButtons();
//    }

    WindowXMLDialog::WindowXMLDialog(const String& xmlFilename, const String& scriptPath,
                                     const String& defaultSkin,
                                     const String& defaultRes) throw(WindowException) :
      WindowXML("WindowXMLDialog",xmlFilename, scriptPath, defaultSkin, defaultRes)
    {
      interceptor->m_loadOnDemand = false;
    }

    WindowXMLDialog::~WindowXMLDialog() { deallocating(); }

    bool WindowXMLDialog::OnMessage(CGUIMessage &message)
    {
      TRACE;
      if (message.GetMessage() == GUI_MSG_WINDOW_DEINIT)
      {
        CGUIWindow *pWindow = g_windowManager.GetWindow(g_windowManager.GetActiveWindow());
        if (pWindow)
          g_windowManager.ShowOverlay(pWindow->GetOverlayState());
        return interceptor->skipLevelOnMessage(message);
      }
      return WindowXML::OnMessage(message);
    }

//    void WindowXMLDialog::Show(bool show /* = true */)
//    {
//      TRACE;
//      int count = ExitCriticalSection(g_graphicsContext);
//// This code sends a message which in turn simply calls show internal.
////   Let's see if I can just put the "Show_Internal" code here.
////      ThreadMessage tMsg = {TMSG_GUI_PYTHON_DIALOG, 1, show ? 1 : 0};
////      tMsg.lpVoid = this;
////      CApplicationMessenger::Get().SendMessage(tMsg, true);
////-----------------------------------------------------
//// 'Show_Internal' code from CGUIPythonWindowXMLDialog
////-----------------------------------------------------
//      if (show)
//      {
//        g_windowManager.RouteToWindow(interceptor);
//
//        // active this dialog...
//        CGUIMessage msg(GUI_MSG_WINDOW_INIT,0,0);
//        OnMessage(msg);
//        amRunning = true;
//      }
//      else // hide
//      {
//        CGUIMessage msg(GUI_MSG_WINDOW_DEINIT,0,0);
//        OnMessage(msg);
//
//        g_windowManager.RemoveDialog(interceptor->GetID());
//        amRunning = false;
//      }
////-----------------------------------------------------
//
//      RestoreCriticalSection(g_graphicsContext, count);
//    }

//    /**
//     * I'm not sure why but my original attempt to handle a WindowXMLDialog
//     *  without recourse to a GUIDialog and without running the 'show' in 
//     *  another thread failed. So we are going to have Show_Internal invoked
//     *  the way it was originally designed (that is, from the main processing
//     *  thread) but without using the stupid hack TMSG_GUI_PYTHON_DIALOG.
//     *  This, however, requires another hack (enter, the DialogJumper) which 
//     *  jumpers calls to Show_Internal and Close_Internal over to the 
//     *  WindowXMLDialog.
//     *
//     * Since Show_Internal and Close_Internal are not virtual, we need
//     *  to intercept these calls at the OnMessage level.
//     *
//     * Note: This method of jumpering Show_Internal/Close_Internal may be 
//     *  brittle with respect to changes in the GUIDialog class as it 
//     *  assumes certain things about the internals.
//     */
//    class DialogJumper : public CGUIDialog
//    {
//      WindowXMLDialog* window;
//    public:
//      DialogJumper(WindowXMLDialog* _window, int windowid, bool closing) : 
//        CGUIDialog(windowid,""), window(_window) 
//      {
//#ifdef LOG_LIFECYCLE_EVENTS
//        CLog::Log(LOGDEBUG, "NEWADDON LIFECYCLE constructing DialogJumper 0x%lx", (long)(((void*)this)));
//#endif
//        CLog::Log(LOGINFO,"DialogJumper window id is %d\n",(int)GetID());
//
//        m_bModal = false;
//        m_enableSound = false;
//        if (closing)
//          m_bRunning = true; // make it think it's running or it wont close
//      }
//      virtual ~DialogJumper()
//      {
//#ifdef LOG_LIFECYCLE_EVENTS
//        CLog::Log(LOGDEBUG, "NEWADDON LIFECYCLE destroying DialogJumper 0x%lx", (long)(((void*)this)));
//#endif
//      }
//      virtual bool OnMessage(CGUIMessage& message);
//    };
//
//    bool DialogJumper::OnMessage(CGUIMessage& message)
//    {
//      TRACE;
//      switch (message.GetMessage())
//      {
//      case GUI_MSG_WINDOW_INIT:
//        // This indicates that the Show_Internal was invoked
//        //  on the dialog.
//        window->Show_Internal();
//        break;
//      case GUI_MSG_WINDOW_DEINIT:
//        // This indicates the Close_Internal was invoked
//        //  on the dialog
//        window->Close_Internal();
//        break;
//      }
////      g_windowManager.Remove(GetID());
//      g_windowManager.RemoveDialog(GetID());
//      // TODO: see if the following works
//      // delete this;
//      return true;
//    };
//
//    void WindowXMLDialog::Show_Internal()
//    {
//      TRACE;
//      g_windowManager.RouteToWindow(interceptor);
//
//      // active this dialog...
//      CGUIMessage msg(GUI_MSG_WINDOW_INIT,0,0);
//      OnMessage(msg);
//      amRunning = true;
//    }
//
//    void WindowXMLDialog::Close_Internal(bool forceClose)
//    {
//      TRACE;
//      CGUIMessage msg(GUI_MSG_WINDOW_DEINIT,0,0);
//      OnMessage(msg);
//
//      g_windowManager.RemoveDialog(interceptor->GetID());
//      amRunning = false;
//    }

    // TODO: Move this into a Mixin since it's the same code as in WindowDialog
    // This used the hack:
    //   ThreadMessage tMsg = {TMSG_GUI_PYTHON_DIALOG, 1, 1};
    // but we're going to try and fix that here.
    void WindowXMLDialog::show()
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
    void WindowXMLDialog::close()
    {
      TRACE;
      DelayedCallGuard dcguard(languageHook);
      bModal = false;
      PulseActionEvent();

//      // see the 'show' for a tediout explanation of problems with the
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

    bool WindowXMLDialog::OnAction(const CAction &action)
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
          window->setActive(true);
          return true;
        }
        break;
        
      case HACK_CUSTOM_ACTION_CLOSING:
        {
          // This is from the CGUIPythonWindowXMLDialog::Show_Internal
          CGUIMessage msg(GUI_MSG_WINDOW_DEINIT,0,0);
          OnMessage(msg);
          g_windowManager.RemoveDialog(ref(window)->GetID());
          // TODO: Figure out how to clean up the CAction
          return true;
        }
        break;
      }

      return WindowXML::OnAction(action);
    }
    
  }
}
