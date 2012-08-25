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

#include "Keyboard.h"
#include "LanguageHook.h"

#include "guilib/GUIWindowManager.h"
#include "dialogs/GUIDialogKeyboardGeneric.h"
#include "ApplicationMessenger.h"

namespace XBMCAddon
{
  namespace xbmc
  {
    Keyboard::Keyboard(const String& line /* = nullString*/, const String& heading/* = nullString*/, bool hidden/* = false*/) 
      : AddonClass("Keyboard"), strDefault(line), strHeading(heading), bHidden(hidden), dlg(NULL) 
    {
      dlg = (CGUIDialogKeyboardGeneric*)g_windowManager.GetWindow(WINDOW_DIALOG_KEYBOARD);
    }

    Keyboard::~Keyboard() {}

    /**
     * doModal([autoclose]) -- Show keyboard and wait for user action.
     * 
     * autoclose      : [opt] integer - milliseconds to autoclose dialog. (default=do not autoclose)
     * 
     * example:
     *   - kb.doModal(30000)
     */
    void Keyboard::doModal(int autoclose) throw (KeyboardException)
    {
      DelayedCallGuard dg(languageHook);
      CGUIDialogKeyboardGeneric *pKeyboard = dlg;
      if(!pKeyboard)
        throw KeyboardException("Unable to load virtual keyboard");

      pKeyboard->Initialize();
      pKeyboard->SetHeading(strHeading);
      pKeyboard->SetText(strDefault);
      pKeyboard->SetHiddenInput(bHidden);
      if (autoclose > 0)
        pKeyboard->SetAutoClose(autoclose);

      // do modal of dialog
      ThreadMessage tMsg = {TMSG_DIALOG_DOMODAL, WINDOW_DIALOG_KEYBOARD, g_windowManager.GetActiveWindow()};
      CApplicationMessenger::Get().SendMessage(tMsg, true);
    }

    // setDefault() Method
    /**
     * setDefault(default) -- Set the default text entry.
     * 
     * default        : string - default text entry.
     * 
     * example:
     *   - kb.setDefault('password')
     */
    void Keyboard::setDefault(const String& line) throw (KeyboardException)
    {
      strDefault = line;

      CGUIDialogKeyboardGeneric *pKeyboard = dlg;
      if(!pKeyboard)
        throw KeyboardException("Unable to load keyboard");

      pKeyboard->SetText(strDefault);
    }

    /**
     * setHiddenInput(hidden) -- Allows hidden text entry.
     * 
     * hidden        : boolean - True for hidden text entry.
     * example:
     *   - kb.setHiddenInput(True)
     */
    void Keyboard::setHiddenInput(bool hidden) throw (KeyboardException)
    {
      bHidden = hidden;

      CGUIDialogKeyboardGeneric *pKeyboard = dlg;
      if(!pKeyboard)
        throw KeyboardException("Unable to load keyboard");

      pKeyboard->SetHiddenInput(bHidden);
    }

    // setHeading() Method
    /**
     * setHeading(heading) -- Set the keyboard heading.
     * 
     * heading        : string - keyboard heading.
     * 
     * example:
     *   - kb.setHeading('Enter password')
     */
    void Keyboard::setHeading(const String& heading) throw (KeyboardException)
    {
      strHeading = heading;

      CGUIDialogKeyboardGeneric *pKeyboard = dlg;
      if(!pKeyboard)
        throw KeyboardException("Unable to load keyboard");

      pKeyboard->SetHeading(strHeading);
    }

    // getText() Method
    /**
     * getText() -- Returns the user input as a string.
     * 
     * *Note, This will always return the text entry even if you cancel the keyboard.
     *        Use the isConfirmed() method to check if user cancelled the keyboard.
     * 
     * example:
     *   - text = kb.getText()
     */
    String Keyboard::getText() throw (KeyboardException)
    {
      CGUIDialogKeyboardGeneric *pKeyboard = dlg;
      if(!pKeyboard)
        throw KeyboardException("Unable to load keyboard");
      return pKeyboard->GetText();
    }

    // isConfirmed() Method
    /**
     * isConfirmed() -- Returns False if the user cancelled the input.
     * 
     * example:
     *   - if (kb.isConfirmed()):
     */
    bool Keyboard::isConfirmed() throw (KeyboardException)
    {
      CGUIDialogKeyboardGeneric *pKeyboard = dlg;
      if(!pKeyboard)
        throw KeyboardException("Unable to load keyboard");
      return pKeyboard->IsConfirmed();
    }
  }
}

