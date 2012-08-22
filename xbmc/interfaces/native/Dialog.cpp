
#include "Dialog.h"
#include "LanguageHook.h"

#include "dialogs/GUIDialogOK.h"
#include "dialogs/GUIDialogYesNo.h"
#include "dialogs/GUIDialogSelect.h"
#include "guilib/GUIWindowManager.h"
#include "dialogs/GUIDialogFileBrowser.h"
#include "dialogs/GUIDialogNumeric.h"
#include "settings/Settings.h"

#define ACTIVE_WINDOW g_windowManager.GetActiveWindow()

namespace XBMCAddon
{
  namespace xbmcgui
  {

    static void XBMCWaitForThreadMessage(int message, int param1, int param2)
    {
      ThreadMessage tMsg = {message, param1, param2};
      CApplicationMessenger::Get().SendMessage(tMsg, true);
    }

    Dialog::~Dialog() {}

    /**
     * yesno(heading, line1[, line2, line3]) -- Show a dialog 'YES/NO'.
     * 
     * heading        : string or unicode - dialog heading.
     * line1          : string or unicode - line #1 text.
     * line2          : [opt] string or unicode - line #2 text.
     * line3          : [opt] string or unicode - line #3 text.
     * nolabel        : [opt] label to put on the no button.
     * yeslabel       : [opt] label to put on the yes button.
     * 
     * *Note, Returns True if 'Yes' was pressed, else False.
     * 
     * example:
     *   - dialog = xbmcgui.Dialog()
     *   - ret = dialog.yesno('XBMC', 'Do you want to exit this script?')\n
     */
    bool Dialog::yesno(const String& heading, const String& line1, 
                       const String& line2,
                       const String& line3,
                       const String& nolabel,
                       const String& yeslabel) throw (WindowException)
    {
      DelayedCallGuard dcguard(languageHook);
      const int window = WINDOW_DIALOG_YES_NO;
      CGUIDialogYesNo* pDialog = (CGUIDialogYesNo*)g_windowManager.GetWindow(window);
      if (pDialog == NULL)
        throw WindowException("Error: Window is NULL, this is not possible :-)");

      // get lines, last 4 lines are optional.
      if (!heading.empty())
        pDialog->SetHeading(heading);
      if (!line1.empty())
        pDialog->SetLine(0, line1);
      if (!line2.empty())
        pDialog->SetLine(1, line2);
      if (!line3.empty())
        pDialog->SetLine(2, line3);

      if (!nolabel.empty())
        pDialog->SetChoice(0,nolabel);
      if (!yeslabel.empty())
        pDialog->SetChoice(1,yeslabel);

      //send message and wait for user input
      XBMCWaitForThreadMessage(TMSG_DIALOG_DOMODAL, window, ACTIVE_WINDOW);

      return pDialog->IsConfirmed();
    }

    /**
     * select(heading, list) -- Show a select dialog.
     * 
     * heading        : string or unicode - dialog heading.
     * list           : string list - list of items.
     * autoclose      : [opt] integer - milliseconds to autoclose dialog. (default=do not autoclose)
     * 
     * *Note, Returns the position of the highlighted item as an integer.
     * 
     * example:
     *   - dialog = xbmcgui.Dialog()
     *   - ret = dialog.select('Choose a playlist', ['Playlist #1', 'Playlist #2, 'Playlist #3'])\n
     */
    int Dialog::select(const String& heading, const std::vector<String>& list, int autoclose) throw (WindowException)
    {
      DelayedCallGuard dcguard(languageHook);
      const int window = WINDOW_DIALOG_SELECT;
      CGUIDialogSelect* pDialog= (CGUIDialogSelect*)g_windowManager.GetWindow(window);
      if (pDialog == NULL)
        throw WindowException("Error: Window is NULL, this is not possible :-)");

      pDialog->Reset();
      CStdString utf8Heading;
      if (!heading.empty())
        pDialog->SetHeading(utf8Heading);

      String listLine;
      for(unsigned int i = 0; i < list.size(); i++)
      {
        listLine = list[i];
          pDialog->Add(listLine);
      }
      if (autoclose > 0)
        pDialog->SetAutoClose(autoclose);

      //send message and wait for user input
      XBMCWaitForThreadMessage(TMSG_DIALOG_DOMODAL, window, ACTIVE_WINDOW);

      return pDialog->GetSelectedLabel();
    }

    /**
     * ok(heading, line1[, line2, line3]) -- Show a dialog 'OK'.
     * 
     * heading        : string or unicode - dialog heading.
     * line1          : string or unicode - line #1 text.
     * line2          : [opt] string or unicode - line #2 text.
     * line3          : [opt] string or unicode - line #3 text.
     * 
     * *Note, Returns True if 'Ok' was pressed, else False.
     * 
     * example:
     *   - dialog = xbmcgui.Dialog()
     *   - ok = dialog.ok('XBMC', 'There was an error.')\n
     */
    bool Dialog::ok(const String& heading, const String& line1, 
                    const String& line2,
                    const String& line3) throw (WindowException)
    {
      DelayedCallGuard dcguard(languageHook);
      const int window = WINDOW_DIALOG_OK;

      CGUIDialogOK* pDialog = (CGUIDialogOK*)g_windowManager.GetWindow(window);
      if (pDialog == NULL)
        throw WindowException("Error: Window is NULL, this is not possible :-)");

      if (!heading.empty())
        pDialog->SetHeading(heading);
      if (!line1.empty())
        pDialog->SetLine(0, line1);
      if (!line2.empty())
        pDialog->SetLine(1, line2);
      if (!line3.empty())
        pDialog->SetLine(2, line3);

      //send message and wait for user input
      XBMCWaitForThreadMessage(TMSG_DIALOG_DOMODAL, window, ACTIVE_WINDOW);

      return pDialog->IsConfirmed();
    }

    /**
     * browse(type, heading, shares[, mask, useThumbs, treatAsFolder, default]) -- Show a 'Browse' dialog.
     * 
     * type           : integer - the type of browse dialog.
     * heading        : string or unicode - dialog heading.
     * shares         : string or unicode - from sources.xml. (i.e. 'myprograms')
     * mask           : [opt] string or unicode - '|' separated file mask. (i.e. '.jpg|.png')
     * useThumbs      : [opt] boolean - if True autoswitch to Thumb view if files exist (default=false).
     * treatAsFolder  : [opt] boolean - if True playlists and archives act as folders (default=false).
     * default        : [opt] string - default path or file.
     * 
     * Types:
     *   0 : ShowAndGetDirectory
     *   1 : ShowAndGetFile
     *   2 : ShowAndGetImage
     *   3 : ShowAndGetWriteableDirectory
     * 
     * *Note, Returns filename and/or path as a string to the location of the highlighted item,
     *        if user pressed 'Ok' or a masked item was selected.
     *        Returns the default value if dialog was canceled.
     * 
     * example:
     *   - dialog = xbmcgui.Dialog()
     *   - fn = dialog.browse(3, 'XBMC', 'files', '', False, False, 'special://masterprofile/script_data/XBMC Lyrics')\n
     */
    String Dialog::browse(int type, const String& heading, const String& s_shares,
                          const String& maskparam, bool useThumbs, 
                          bool useFileDirectories, 
                          const String& defaultt ) throw (WindowException)
    {
      DelayedCallGuard dcguard(languageHook);
      CStdString value;
      std::string mask = maskparam;
      VECSOURCES *shares = g_settings.GetSourcesFromType(s_shares);
      if (!shares) 
        throw WindowException(((std::string("Error: GetSourcesFromType given ") += s_shares) += " is NULL.").c_str());

      if (useFileDirectories && (!maskparam.empty() && !maskparam.size() == 0))
        mask += "|.rar|.zip";

      value = defaultt;
      if (type == 1)
          CGUIDialogFileBrowser::ShowAndGetFile(*shares, mask, heading, value, useThumbs, useFileDirectories);
      else if (type == 2)
        CGUIDialogFileBrowser::ShowAndGetImage(*shares, heading, value);
      else
        CGUIDialogFileBrowser::ShowAndGetDirectory(*shares, heading, value, type != 0);
      return value;
    }

    std::vector<String> Dialog::browseMultiple(int type, const String& heading, const String& s_shares,
                          const String& mask, bool useThumbs, 
                          bool useFileDirectories, const String& defaultt ) throw (WindowException)
    {
      DelayedCallGuard dcguard(languageHook);
      VECSOURCES *shares = g_settings.GetSourcesFromType(s_shares);
      CStdStringArray tmpret;
      String lmask = mask;
      if (!shares) 
        throw WindowException(((std::string("Error: GetSourcesFromType given ") += s_shares) += " is NULL.").c_str());

      if (useFileDirectories && (!lmask.empty() && !(lmask.size() == 0)))
        lmask += "|.rar|.zip";

      if (type == 1)
        CGUIDialogFileBrowser::ShowAndGetFileList(*shares, lmask, heading, tmpret, useThumbs, useFileDirectories);
      else if (type == 2)
        CGUIDialogFileBrowser::ShowAndGetImageList(*shares, heading, tmpret);
      else
        throw WindowException(((std::string("Error: Cannot retreive multuple directories using browse ") += s_shares) += " is NULL.").c_str());

      std::vector<String> valuelist;
      int index = 0;
      for (CStdStringArray::iterator iter = tmpret.begin(); iter != tmpret.end(); iter++)
        valuelist[index++] = (*iter);

      return valuelist;
    }


    /**
     * numeric(type, heading[, default]) -- Show a 'Numeric' dialog.
     * 
     * type           : integer - the type of numeric dialog.
     * heading        : string or unicode - dialog heading.
     * default        : [opt] string - default value.
     * 
     * Types:
     *   0 : ShowAndGetNumber    (default format: #)
     *   1 : ShowAndGetDate      (default format: DD/MM/YYYY)
     *   2 : ShowAndGetTime      (default format: HH:MM)
     *   3 : ShowAndGetIPAddress (default format: #.#.#.#)
     * 
     * *Note, Returns the entered data as a string.
     *        Returns the default value if dialog was canceled.
     * 
     * example:
     *   - dialog = xbmcgui.Dialog()
     *   - d = dialog.numeric(1, 'Enter date of birth')\n
     */
    String Dialog::numeric(int inputtype, const String& heading, const String& defaultt)
    {
      DelayedCallGuard dcguard(languageHook);
      CStdString value;
      SYSTEMTIME timedate;
      GetLocalTime(&timedate);

      if (!heading.empty())
      {
        if (inputtype == 1)
        {
          if (!defaultt.empty() && defaultt.size() == 10)
          {
            CStdString sDefault = defaultt;
            timedate.wDay = atoi(sDefault.Left(2));
            timedate.wMonth = atoi(sDefault.Mid(3,4));
            timedate.wYear = atoi(sDefault.Right(4));
          }
          if (CGUIDialogNumeric::ShowAndGetDate(timedate, heading))
            value.Format("%2d/%2d/%4d", timedate.wDay, timedate.wMonth, timedate.wYear);
          else
            return emptyString;
        }
        else if (inputtype == 2)
        {
          if (!defaultt.empty() && defaultt.size() == 5)
          {
            CStdString sDefault = defaultt;
            timedate.wHour = atoi(sDefault.Left(2));
            timedate.wMinute = atoi(sDefault.Right(2));
          }
          if (CGUIDialogNumeric::ShowAndGetTime(timedate, heading))
            value.Format("%2d:%02d", timedate.wHour, timedate.wMinute);
          else
            return emptyString;
        }
        else if (inputtype == 3)
        {
          value = defaultt;
          if (!CGUIDialogNumeric::ShowAndGetIPAddress(value, heading))
            return emptyString;
        }
        else
        {
          value = defaultt;
          if (!CGUIDialogNumeric::ShowAndGetNumber(value, heading))
            return emptyString;
        }
      }
      return value;
    }

    DialogProgress::~DialogProgress() {}


    /**
     * create(heading[, line1, line2, line3]) -- Create and show a progress dialog.
     * 
     * heading        : string or unicode - dialog heading.
     * line1          : string or unicode - line #1 text.
     * line2          : [opt] string or unicode - line #2 text.
     * line3          : [opt] string or unicode - line #3 text.
     * 
     * *Note, Use update() to update lines and progressbar.
     * 
     * example:
     *   - pDialog = xbmcgui.DialogProgress()
     *   - pDialog.create('XBMC', 'Initializing script...')
     */
    void DialogProgress::create(const String& heading, const String& line1, 
                                const String& line2,
                                const String& line3) throw (WindowException)
    {
      DelayedCallGuard dcguard(languageHook);
      CGUIDialogProgress* pDialog= (CGUIDialogProgress*)g_windowManager.GetWindow(WINDOW_DIALOG_PROGRESS);

      if (pDialog == NULL)
        throw WindowException("Error: Window is NULL, this is not possible :-)");

      dlg = pDialog;

      pDialog->SetHeading(heading);

      if (!line1.empty())
        pDialog->SetLine(0, line1);
      if (!line2.empty())
        pDialog->SetLine(1, line2);
      if (!line3.empty())
        pDialog->SetLine(2, line3);

      pDialog->StartModal();
    }

    /**
     * update(percent[, line1, line2, line3]) -- Update's the progress dialog.
     * 
     * percent        : integer - percent complete. (0:100)
     * line1          : [opt] string or unicode - line #1 text.
     * line2          : [opt] string or unicode - line #2 text.
     * line3          : [opt] string or unicode - line #3 text.
     * 
     * *Note, If percent == 0, the progressbar will be hidden.
     * 
     * example:
     *   - pDialog.update(25, 'Importing modules...')
     */
    void DialogProgress::update(int percent, const String& line1, 
                                const String& line2,
                                const String& line3) throw (WindowException)
    {
      DelayedCallGuard dcguard(languageHook);
      CGUIDialogProgress* pDialog= dlg;

      if (pDialog == NULL)
        throw WindowException("Error: Window is NULL, this is not possible :-)");

      if (percent >= 0 && percent <= 100)
      {
        pDialog->SetPercentage(percent);
        pDialog->ShowProgressBar(true);
      }
      else
      {
        pDialog->ShowProgressBar(false);
      }

      if (!line1.empty())
        pDialog->SetLine(0, line1);
      if (!line2.empty())
        pDialog->SetLine(1, line2);
      if (!line3.empty())
        pDialog->SetLine(2, line3);
    }

    /**
     * close() -- Close the progress dialog.
     * 
     * example:
     *   - pDialog.close()
     */
    void DialogProgress::close()
    {
      DelayedCallGuard dcguard(languageHook);
      dlg->Close();
    }

    /**
     * iscanceled() -- Returns True if the user pressed cancel.
     * 
     * example:
     *   - if (pDialog.iscanceled()): return
     */
    bool DialogProgress::iscanceled()
    {
      return dlg->IsCanceled();
    }
  }
}

