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

// TODO: Need a uniform way of returning an error status

#if (defined HAVE_CONFIG_H) && (!defined WIN32)
  #include "config.h"
#endif

#include "ModuleXbmc.h"

#include "Application.h"
#include "ApplicationMessenger.h"
#ifdef HAS_HTTPAPI
#include "interfaces/http-api/XBMChttp.h"
#include "interfaces/http-api/HttpApi.h"
#endif
#include "utils/URIUtils.h"
#include "aojsonrpc.h"
#ifndef TARGET_WINDOWS
#include "XTimeUtils.h"
#endif
#include "guilib/LocalizeStrings.h"
#include "settings/GUISettings.h"
#include "GUIInfoManager.h"
#include "guilib/GUIAudioManager.h"
#include "guilib/GUIWindowManager.h"
#include "filesystem/File.h"
#include "filesystem/SpecialProtocol.h"
#include "storage/IoSupport.h"
#include "utils/Crc32.h"
#include "FileItem.h"
#include "LangInfo.h"
#include "settings/Settings.h"
#include "guilib/TextureManager.h"
#include "Util.h"
#include "URL.h"
#include "utils/FileUtils.h"

#include "CallbackHandler.h"
#include "AddonUtils.h"

#include "LanguageHook.h"

#include "cores/VideoRenderers/RenderCapture.h"

#include "threads/SystemClock.h"
#include <vector>

namespace XBMCAddon
{

  namespace xbmc
  {
    /*****************************************************************
     * start of xbmc methods
     *****************************************************************/
    /**
     * log(msg[, level]) -- Write a string to XBMC's log file and the debug window.
     *     msg            : string - text to output.
     *     level          : [opt] integer - log level to ouput at. (default=LOGNOTICE)
     *     
     * *Note, You can use the above as keywords for arguments and skip certain optional arguments.
     *        Once you use a keyword, all following arguments require the keyword.
     * 
     * Text is written to the log for the following conditions.
     *           XBMC loglevel == -1 (NONE, nothing at all is logged)
     *           XBMC loglevel == 0 (NORMAL, shows LOGNOTICE, LOGERROR, LOGSEVERE and LOGFATAL)\
     *           XBMC loglevel == 1 (DEBUG, shows all)
     *           See pydocs for valid values for level.
     *           
     *           example:
     *             - xbmc.output(msg='This is a test string.', level=xbmc.LOGDEBUG));
     */
    void log(const char* msg, int level)
    {
//      TRACE;
      // check for a valid loglevel
      if (level < LOGDEBUG || level > LOGNONE)
        level = LOGNOTICE;
      CLog::Log(level, "%s", msg);
    }


    /**
     * Shutdown() -- Shutdown the xbox.
     *
     * example:
     *  - xbmc.shutdown()
     */
    void shutdown()
    {
      TRACE;
      ThreadMessage tMsg = {TMSG_SHUTDOWN};
      CApplicationMessenger::Get().SendMessage(tMsg);
    }


    /**
     * restart() -- Restart the xbox.
     * example:
     *  - xbmc.restart()
     */
    void restart()
    {
      TRACE;
      ThreadMessage tMsg = {TMSG_RESTART};
      CApplicationMessenger::Get().SendMessage(tMsg);
    }

    /**
     * executescript(script) -- Execute a python script.
     * 
     * script         : string - script filename to execute.
     * 
     * example:
     *   - xbmc.executescript('special://home/scripts/update.py')
     */
    void executescript(const char* script)
    {
      TRACE;
      if (! script)
        return;

      ThreadMessage tMsg = {TMSG_EXECUTE_SCRIPT};
      tMsg.strParam = script;
      CApplicationMessenger::Get().SendMessage(tMsg);
    }

    /**
     * executebuiltin(function) -- Execute a built in XBMC function.
     * 
     * function       : string - builtin function to execute.
     * 
     * List of functions - http://wiki.xbmc.org/?title=List_of_Built_In_Functions 
     * 
     * example:
     *   - xbmc.executebuiltin('XBMC.RunXBE(c:\\\\avalaunch.xbe)')
     */
    void executebuiltin(const char* function, bool wait /* = false*/)
    {
      TRACE;
      if (! function)
        return;
      CApplicationMessenger::Get().ExecBuiltIn(function,wait);
    }

#ifdef HAS_HTTPAPI
    /**
     * executehttpapi(httpcommand) -- Execute an HTTP API command.
     * 
     * httpcommand    : string - http command to execute.
     * 
     * List of commands - http://wiki.xbmc.org/?title=WebServerHTTP-API#The_Commands 
     * 
     * example:
     *   - response = xbmc.executehttpapi('TakeScreenShot(special://temp/test.jpg,0,false,200,-1,90)')
     */
    String executehttpapi(const char* httpcommand) 
    {
      TRACE;
      String ret;
      if (! httpcommand)
        return ret;

      if (!m_pXbmcHttp)
        m_pXbmcHttp = new CXbmcHttp();

      int open, close;
      CStdString parameter="", cmd=httpcommand, execute;
      open = cmd.Find("(");
      if (open>0)
        {
          close=cmd.length();
          while (close>open && cmd.Mid(close,1)!=")")
            close--;
          if (close>open)
            {
              parameter = cmd.Mid(open + 1, close - open - 1);
              parameter.Replace(",",";");
              execute = cmd.Left(open);
            }
          else //open bracket but no close
            return ret;
        }
      else //no parameters
        execute = cmd;

      CURL::Decode(parameter);
      return CHttpApi::MethodCall(execute, parameter);
    }
#endif

#ifdef HAS_JSONRPC
    // executehttpapi() method
    /**
     * executeJSONRPC(jsonrpccommand) -- Execute an JSONRPC command.
     * 
     * jsonrpccommand    : string - jsonrpc command to execute.
     * 
     * List of commands - 
     * 
     * example:
     *   - response = xbmc.executeJSONRPC('{ \"jsonrpc\": \"2.0\", \"method\": \"JSONRPC.Introspect\", \"id\": 1 }')
     */
    String executeJSONRPC(const char* jsonrpccommand)
    {
      TRACE;
      String ret;

      if (! jsonrpccommand)
        return ret;

      //    String method = jsonrpccommand;

      CAddOnTransport transport;
      CAddOnTransport::CAddOnClient client;

      return JSONRPC::CJSONRPC::MethodCall(/*method*/ jsonrpccommand, &transport, &client);
    }
#endif

    // sleep() method
    /**
     * sleep(time) -- Sleeps for 'time' msec.
     * 
     * time           : integer - number of msec to sleep.
     * 
     * *Note, This is useful if you have for example a Player class that is waiting
     *        for onPlayBackEnded() calls.
     * 
     * Throws: PyExc_TypeError, if time is not an integer.
     * 
     * example:
     *   - xbmc.sleep(2000) # sleeps for 2 seconds
     */
    void sleep(long timemillis)
    {
      TRACE;

      XbmcThreads::EndTime endTime(timemillis);
      while (!endTime.IsTimePast())
      {
        DelayedCallGuard dcguard;
        long nextSleep = endTime.MillisLeft();
        if (nextSleep > 100)
          nextSleep = 100; // only sleep for 100 millis
        ::Sleep(nextSleep);
      }
    }

    // getLocalizedString() method
    /**
     * getLocalizedString(id) -- Returns a localized 'unicode string'.
     * 
     * id             : integer - id# for string you want to localize.
     * 
     * *Note, See strings.xml in \\language\\{yourlanguage}\\ for which id
     *        you need for a string.
     * 
     * example:
     *   - locstr = xbmc.getLocalizedString(6)
     */
    String getLocalizedString(int id)
    {
      TRACE;
      String label;
      if (id >= 30000 && id <= 30999)
        label = g_localizeStringsTemp.Get(id);
      else if (id >= 32000 && id <= 32999)
        label = g_localizeStringsTemp.Get(id);
      else
        label = g_localizeStrings.Get(id);

      return label;
    }

    // getSkinDir() method
    /**
     * getSkinDir() -- Returns the active skin directory as a string.
     * 
     * *Note, This is not the full path like 'special://home/addons/MediaCenter', but only 'MediaCenter'.
     * 
     * example:
     *   - skindir = xbmc.getSkinDir()
     */
    String getSkinDir()
    {
      TRACE;
      return g_guiSettings.GetString("lookandfeel.skin");
    }

    // getLanguage() method
    /**
     * getLanguage() -- Returns the active language as a string.
     * 
     * example:
     *   - language = xbmc.getLanguage()
     */
    String getLanguage()
    {
      TRACE;
      return g_guiSettings.GetString("locale.language");
    }

    // getIPAddress() method
    /**
     * getIPAddress() -- Returns the current ip address as a string.
     * 
     * example:
     *   - ip = xbmc.getIPAddress()
     */
    String getIPAddress()
    {
      TRACE;
      char cTitleIP[32];
      sprintf(cTitleIP, "127.0.0.1");
      CNetworkInterface* iface = g_application.getNetwork().GetFirstConnectedInterface();
      if (iface)
        return iface->GetCurrentIPAddress();

      return cTitleIP;
    }

    // getDVDState() method
    /**
     * getDVDState() -- Returns the dvd state as an integer.
     * 
     * return values are:
     *    1 : xbmc.DRIVE_NOT_READY
     *   16 : xbmc.TRAY_OPEN
     *   64 : xbmc.TRAY_CLOSED_NO_MEDIA
     *   96 : xbmc.TRAY_CLOSED_MEDIA_PRESENT
     * 
     * example:
     *   - dvdstate = xbmc.getDVDState()
     */
    long getDVDState()
    {
      TRACE;
      return CIoSupport::GetTrayState();
    }

    // getFreeMem() method
    /**
     * getFreeMem() -- Returns the amount of free memory in MB as an integer.
     * 
     * example:
     *   - freemem = xbmc.getFreeMem()
     */
    long getFreeMem()
    {
      TRACE;
      MEMORYSTATUSEX stat;
      stat.dwLength = sizeof(MEMORYSTATUSEX);
      GlobalMemoryStatusEx(&stat);
      return (long)(stat.ullAvailPhys  / ( 1024 * 1024 ));
    }

    // getCpuTemp() method
    // ## Doesn't work right, use getInfoLabel('System.CPUTemperature') instead.
    /*PyDoc_STRVAR(getCpuTemp__doc__,
      "getCpuTemp() -- Returns the current cpu temperature as an integer."
      ""
      "example:"
      "  - cputemp = xbmc.getCpuTemp()");

      PyObject* XBMC_GetCpuTemp(PyObject *self, PyObject *args)
      {
      unsigned short cputemp;
      unsigned short cpudec;

      _outp(0xc004, (0x4c<<1)|0x01);
      _outp(0xc008, 0x01);
      _outpw(0xc000, _inpw(0xc000));
      _outp(0xc002, (0) ? 0x0b : 0x0a);
      while ((_inp(0xc000) & 8));
      cputemp = _inpw(0xc006);

      _outp(0xc004, (0x4c<<1)|0x01);
      _outp(0xc008, 0x10);
      _outpw(0xc000, _inpw(0xc000));
      _outp(0xc002, (0) ? 0x0b : 0x0a);
      while ((_inp(0xc000) & 8));
      cpudec = _inpw(0xc006);

      if (cpudec<10) cpudec = cpudec * 100;
      if (cpudec<100) cpudec = cpudec *10;

      return PyInt_FromLong((long)(cputemp + cpudec / 1000.0f));
      }*/

    // getInfolabel() method
    /**
     * getInfoLabel(infotag) -- Returns an InfoLabel as a string.
     * 
     * infotag        : string - infoTag for value you want returned.
     * 
     * List of InfoTags - http://wiki.xbmc.org/?title=InfoLabels 
     * 
     * example:
     *   - label = xbmc.getInfoLabel('Weather.Conditions')
     */
    String getInfoLabel(const char* cLine)
    {
      TRACE;
      if (!cLine)
      {
        String ret;
        return ret;
      }

      int ret = g_infoManager.TranslateString(cLine);
      //doesn't seem to be a single InfoTag?
      //try full blown GuiInfoLabel then
      if (ret == 0)
      {
        CGUIInfoLabel label(cLine);
        return label.GetLabel(0);
      }
      else
        return g_infoManager.GetLabel(ret);
    }

    // getInfoImage() method
    /**
     * getInfoImage(infotag) -- Returns a filename including path to the InfoImage's
     *                          thumbnail as a string.
     * 
     * infotag        : string - infotag for value you want returned.
     * 
     * List of InfoTags - http://wiki.xbmc.org/?title=InfoLabels 
     * 
     * example:
     *   - filename = xbmc.getInfoImage('Weather.Conditions')
     */
    String getInfoImage(const char * infotag)
    {
      TRACE;
      if (!infotag)
        {
          String ret;
          return ret;
        }

      int ret = g_infoManager.TranslateString(infotag);
      return g_infoManager.GetImage(ret, WINDOW_INVALID);
    }

    // playSFX() method
    /**
     * playSFX(filename) -- Plays a wav file by filename
     * 
     * filename       : string - filename of the wav file to play.
     * 
     * example:
     *   - xbmc.playSFX('special://xbmc/scripts/dingdong.wav')
     */
    void playSFX(const char* filename)
    {
      TRACE;
      if (!filename)
        return;

      if (XFILE::CFile::Exists(filename))
      {
        g_audioManager.PlayPythonSound(filename);
      }
    }

    // enableNavSounds() method
    /**
     * enableNavSounds(yesNo) -- Enables/Disables nav sounds
     * 
     * yesNo          : integer - enable (True) or disable (False) nav sounds
     * 
     * example:
     *   - xbmc.enableNavSounds(True)
     */
    void enableNavSounds(bool yesNo)
    {
      TRACE;
      g_audioManager.Enable(yesNo);
    }

    // getCondVisibility() method
    /**
     * getCondVisibility(condition) -- Returns True (1) or False (0) as a bool.
     * 
     * condition      : string - condition to check.
     * 
     * List of Conditions - http://wiki.xbmc.org/?title=List_of_Boolean_Conditions 
     * 
     * *Note, You can combine two (or more) of the above settings by using \"+\" as an AND operator,
     * \"|\" as an OR operator, \"!\" as a NOT operator, and \"[\" and \"]\" to bracket expressions.
     * 
     * example:
     *   - visible = xbmc.getCondVisibility('[Control.IsVisible(41) + !Control.IsVisible(12)]')
     */
    bool getCondVisibility(const char *condition)
    {
      TRACE;
      if (!condition)
        return false;

      int id;
      bool ret;
      {
        LOCKGUI;

        id = g_windowManager.GetTopMostModalDialogID();
        if (id == WINDOW_INVALID) id = g_windowManager.GetActiveWindow();
        ret = g_infoManager.EvaluateBool(condition,id);
      }

      return ret;
    }

    // getGlobalIdleTime() method
    /**
     * getGlobalIdleTime() -- Returns the elapsed idle time in seconds as an integer.
     * 
     * example:
     *   - t = xbmc.getGlobalIdleTime()
     */
    int getGlobalIdleTime()
    {
      TRACE;
      return g_application.GlobalIdleTime();
    }

    // getCacheThumbName function
    /**
     * getCacheThumbName(path) -- Returns a thumb cache filename.
     * 
     * path           : string or unicode - path to file
     * 
     * example:
     *   - thumb = xbmc.getCacheThumbName('f:\\\\videos\\\\movie.avi')
     */
    String getCacheThumbName(const String& path)
    {
      TRACE;
      Crc32 crc;
      crc.ComputeFromLowerCase(path);
      CStdString strPath;
      strPath.Format("%08x.tbn", (unsigned __int32)crc);
      return strPath;
    }

    // makeLegalFilename function
    /**
     * makeLegalFilename(filename[, fatX]) -- Returns a legal filename or path as a string.
     * 
     * filename       : string or unicode - filename/path to make legal
     * fatX           : [opt] bool - True=Xbox file system(Default)
     * 
     * *Note, If fatX is true you should pass a full path. If fatX is false only pass
     *        the basename of the path.
     * 
     *        You can use the above as keywords for arguments and skip certain optional arguments.
     *        Once you use a keyword, all following arguments require the keyword.
     * 
     * example:
     *   - filename = xbmc.makeLegalFilename('F:\\Trailers\\Ice Age: The Meltdown.avi')
     */
    String makeLegalFilename(const String& filename, bool fatX)
    {
      TRACE;
      return CUtil::MakeLegalPath(filename);
    }

    // translatePath function
    /**
     * translatePath(path) -- Returns the translated path.
     * 
     * path           : string or unicode - Path to format
     * 
     * *Note, Only useful if you are coding for both Linux and Windows/Xbox.
     *        e.g. Converts 'special://masterprofile/script_data' -> '/home/user/XBMC/UserData/script_data'
     *        on Linux. Would return 'special://masterprofile/script_data' on the Xbox.
     * 
     * example:
     *   - fpath = xbmc.translatePath('special://masterprofile/script_data')
     */
    String translatePath(const String& path)
    {
      TRACE;
      return CSpecialProtocol::TranslatePath(path);
    }

    // getcleanmovietitle function
    /**
     * getCleanMovieTitle(path[, usefoldername]) -- Returns a clean movie title and year string if available.
     * 
     * path           : string or unicode - String to clean
     * bool           : [opt] bool - use folder names (defaults to false)
     * 
     * example:
     *   - title, year = xbmc.getCleanMovieTitle('/path/to/moviefolder/test.avi', True)
     */
    Tuple<String,String> getCleanMovieTitle(const String& path, bool usefoldername)
    {
      TRACE;
      CFileItem item(path, false);
      CStdString strName = item.GetMovieName(usefoldername);

      CStdString strTitleAndYear;
      CStdString strTitle;
      CStdString strYear;
      CUtil::CleanString(strName, strTitle, strTitleAndYear, strYear, usefoldername);
      return Tuple<String,String>(strTitle,strYear);
    }

    // validatePath function
    /**
     * validatePath(path) -- Returns the validated path.
     * 
     * path           : string or unicode - Path to format
     * 
     * *Note, Only useful if you are coding for both Linux and Windows/Xbox for fixing slash problems.
     *        e.g. Corrects 'Z://something' -> 'Z:\\something'
     * 
     * example:
     *   - fpath = xbmc.validatePath(somepath)
     */
    String validatePath(const String& path)
    {
      TRACE;
      return CUtil::ValidatePath(path, true);
    }

    // getRegion function
    /**
     * getRegion(id) -- Returns your regions setting as a string for the specified id.
     * 
     * id             : string - id of setting to return
     * 
     * *Note, choices are (dateshort, datelong, time, meridiem, tempunit, speedunit)
     * 
     *        You can use the above as keywords for arguments.
     * 
     * example:
     *   - date_long_format = xbmc.getRegion('datelong')
     */
    String getRegion(const char* id)
    {
      TRACE;
      CStdString result;

      if (strcmpi(id, "datelong") == 0)
        {
          result = g_langInfo.GetDateFormat(true);
          result.Replace("DDDD", "%A");
          result.Replace("MMMM", "%B");
          result.Replace("D", "%d");
          result.Replace("YYYY", "%Y");
        }
      else if (strcmpi(id, "dateshort") == 0)
        {
          result = g_langInfo.GetDateFormat(false);
          result.Replace("MM", "%m");
          result.Replace("DD", "%d");
          result.Replace("YYYY", "%Y");
        }
      else if (strcmpi(id, "tempunit") == 0)
        result = g_langInfo.GetTempUnitString();
      else if (strcmpi(id, "speedunit") == 0)
        result = g_langInfo.GetSpeedUnitString();
      else if (strcmpi(id, "time") == 0)
        {
          result = g_langInfo.GetTimeFormat();
          result.Replace("H", "%H");
          result.Replace("h", "%I");
          result.Replace("mm", "%M");
          result.Replace("ss", "%S");
          result.Replace("xx", "%p");
        }
      else if (strcmpi(id, "meridiem") == 0)
        result.Format("%s/%s", g_langInfo.GetMeridiemSymbol(CLangInfo::MERIDIEM_SYMBOL_AM), g_langInfo.GetMeridiemSymbol(CLangInfo::MERIDIEM_SYMBOL_PM));

      return result;
    }

    // getSupportedMedia function
    /**
     * getSupportedMedia(media) -- Returns the supported file types for the specific media as a string.
     * 
     * media          : string - media type
     * 
     * *Note, media type can be (video, music, picture).
     * 
     *        The return value is a pipe separated string of filetypes (eg. '.mov|.avi').
     * 
     *        You can use the above as keywords for arguments.
     * 
     * example:
     *   - mTypes = xbmc.getSupportedMedia('video')
     */
    // TODO: Add a mediaType enum
    String getSupportedMedia(const char* mediaType)
    {
      TRACE;
      String result;
      if (strcmpi(mediaType, "video") == 0)
        result = g_settings.m_videoExtensions;
      else if (strcmpi(mediaType, "music") == 0)
        result = g_settings.m_musicExtensions;
      else if (strcmpi(mediaType, "picture") == 0)
        result = g_settings.m_pictureExtensions;

      // TODO:
      //    else
      //      return an error

      return result;
    }

    // skinHasImage function
    /**
     * skinHasImage(image) -- Returns True if the image file exists in the skin.
     * 
     * image          : string - image filename
     * 
     * *Note, If the media resides in a subfolder include it. (eg. home-myfiles\\\\home-myfiles2.png)
     * 
     *        You can use the above as keywords for arguments.
     * 
     * example:
     *   - exists = xbmc.skinHasImage('ButtonFocusedTexture.png')
     */
    bool skinHasImage(const char* image)
    {
      TRACE;
      return g_TextureManager.HasTexture(image);
    }


    /**
     * startServer(typ, bStart, bWait) -- start or stop a server.
     * 
     * typ          : integer - use SERVER_* constants
     * 
     * bStart       : bool - start (True) or stop (False) a server
     * 
     * bWait        : [opt] bool - wait on stop before returning (not supported by all servers)
     * 
     * returnValue  : bool - True or False
     * example:
     *   - xbmc.startServer(xbmc.SERVER_AIRPLAYSERVER, False)
     */
    bool startServer(int iTyp, bool bStart, bool bWait)
    {
      TRACE;
      DelayedCallGuard dg;
      return g_application.StartServer((CApplication::ESERVERS)iTyp, bStart != 0, bWait != 0);
    }

    int getSERVER_WEBSERVER() { return CApplication::ES_WEBSERVER; }
    int getSERVER_AIRPLAYSERVER() { return CApplication::ES_AIRPLAYSERVER; }
    int getSERVER_UPNPSERVER() { return CApplication::ES_UPNPSERVER; }
    int getSERVER_UPNPRENDERER() { return CApplication::ES_UPNPRENDERER; }
    int getSERVER_EVENTSERVER() { return CApplication::ES_EVENTSERVER; }
    int getSERVER_JSONRPCSERVER() { return CApplication::ES_JSONRPCSERVER; }
    int getSERVER_ZEROCONF() { return CApplication::ES_ZEROCONF; }

    int getPLAYLIST_MUSIC() { return PLAYLIST_MUSIC; }
    int getPLAYLIST_VIDEO() { return PLAYLIST_VIDEO; }
    int getPLAYER_CORE_AUTO() { return EPC_NONE; }
    int getPLAYER_CORE_DVDPLAYER() { return EPC_DVDPLAYER; }
    int getPLAYER_CORE_MPLAYER() { return EPC_MPLAYER; }
    int getPLAYER_CORE_PAPLAYER() { return EPC_PAPLAYER; }
    int getTRAY_OPEN() { return TRAY_OPEN; }
    int getDRIVE_NOT_READY() { return DRIVE_NOT_READY; }
    int getTRAY_CLOSED_NO_MEDIA() { return TRAY_CLOSED_NO_MEDIA; }
    int getTRAY_CLOSED_MEDIA_PRESENT() { return TRAY_CLOSED_MEDIA_PRESENT; }
    int getLOGDEBUG() { return LOGDEBUG; }
    int getLOGINFO() { return LOGINFO; }
    int getLOGNOTICE() { return LOGNOTICE; }
    int getLOGWARNING() { return LOGWARNING; }
    int getLOGERROR() { return LOGERROR; }
    int getLOGSEVERE() { return LOGSEVERE; }
    int getLOGFATAL() { return LOGFATAL; }
    int getLOGNONE() { return LOGNONE; }

    // render capture user states
    int getCAPTURE_STATE_WORKING() { return CAPTURESTATE_WORKING; }
    int getCAPTURE_STATE_DONE(){ return CAPTURESTATE_DONE; }
    int getCAPTURE_STATE_FAILED() { return CAPTURESTATE_FAILED; }

    // render capture flags
    int getCAPTURE_FLAG_CONTINUOUS() { return (int)CAPTUREFLAG_CONTINUOUS; }
    int getCAPTURE_FLAG_IMMEDIATELY() { return (int)CAPTUREFLAG_IMMEDIATELY; }

    const int lLOGNOTICE = LOGNOTICE;

    static std::vector<AddonClass::Ref<Monitor> > monitors;
    static CCriticalSection monitorsLock;

    void registerMonitor(Monitor* monitor)
    {
      TRACE;
      if (monitor == NULL)
        return;
      unregisterMonitor(monitor);
      CSingleLock l(monitorsLock);
      monitors.push_back(AddonClass::Ref<Monitor>(monitor));

      // for testing, make a callback
      LanguageHook* lh = LanguageHook::getLanguageHook();
      CallbackHandler* ch = lh == NULL ? NULL : lh->getCallbackHandler();
      if (ch)
        ch->handleCallback(new CallbackFunction<Monitor,const char*>(monitor,&Monitor::callback,"through callback mechanism"));

      monitor->callback("direct");
    }

    void unregisterMonitor(Monitor* monitor)
    {
      TRACE;
      if (monitor == NULL)
        return;
      CSingleLock l(monitorsLock);
      for (std::vector<AddonClass::Ref<Monitor> >::iterator iter = monitors.begin(); iter != monitors.end(); iter++)
      {
        AddonClass::Ref<Monitor>& cur = (*iter);
        if (monitor == cur.get())
        {
          monitors.erase(iter);
          break;
        }
      }
    }
  }
}
