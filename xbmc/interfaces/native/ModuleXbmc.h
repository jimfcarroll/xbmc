/*
 *      Copyright (C) 2005-2010 Team XBMC
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

#include "AddonString.h"
#include "utils/log.h"
#include "utils/StdString.h"

#include "swighelper.h"
#include <vector>

namespace XBMCAddon
{
  namespace xbmc
  {
#ifndef SWIG
    // This is a bit of a hack to get around a SWIG problem
    extern const int lLOGNOTICE;
#endif

    /**
     * "log(msg[, level]) -- Write a string to XBMC's log file and the debug window.\n"
     *     "\n"
     *     "msg            : string - text to output.\n"
     *     "level          : [opt] integer - log level to ouput at. (default=LOGNOTICE)\n"
     *     "\n"
     * "*Note, You can use the above as keywords for arguments and skip certain optional arguments.\n"
     * "       Once you use a keyword, all following arguments require the keyword.\n"
     * "\n"
     * Text is written to the log for the following conditions.\n"
     *           XBMC loglevel == -1 (NONE, nothing at all is logged)"
     *           XBMC loglevel == 0 (NORMAL, shows LOGNOTICE, LOGERROR, LOGSEVERE and LOGFATAL)"\
     *           XBMC loglevel == 1 (DEBUG, shows all)"
     *           See pydocs for valid values for level.\n"
     *           "\n"
     *           "example:\n"
     *           "  - xbmc.output(msg='This is a test string.', level=xbmc.LOGDEBUG)\n");
     */
    void log(const char* message, int level = lLOGNOTICE);
    void shutdown();
    void restart();
    void executescript(const char* script);
    void executebuiltin(const char* function, bool wait = false);
#ifdef HAS_HTTPAPI
    String executehttpapi(const char* httpcommand);
#endif

#ifdef HAS_JSONRPC
    String executeJSONRPC(const char* jsonrpccommand);
#endif

    void sleep(long timemillis);
    String getLocalizedString(int id);
    String getSkinDir();
    String getLanguage();
    String getIPAddress();
    long getDVDState();
    long getFreeMem();
    String getInfoLabel(const char* cLine);
    String getInfoImage(const char * infotag);
    void playSFX(const char* filename);
    void enableNavSounds(bool yesNo);
    bool getCondVisibility(const char *condition);
    int getGlobalIdleTime();
    String getCacheThumbName(const String& path);
    String makeLegalFilename(const String& filename,bool fatX);
    String translatePath(const String& path);
    std::vector<CStdString> getCleanMovieTitle(const String& path, bool usefoldername = false);
    String validatePath(const String& path);
    String getRegion(const char* id);
    String getSupportedMedia(const char* mediaType);
    bool skinHasImage(const char* image);
    bool startServer(int iTyp, bool bStart, bool bWait = false);

    SWIG_CONSTANT_FROM_GETTER(int,SERVER_WEBSERVER);
    SWIG_CONSTANT_FROM_GETTER(int,SERVER_AIRPLAYSERVER);
    SWIG_CONSTANT_FROM_GETTER(int,SERVER_UPNPSERVER);
    SWIG_CONSTANT_FROM_GETTER(int,SERVER_UPNPRENDERER);
    SWIG_CONSTANT_FROM_GETTER(int,SERVER_EVENTSERVER);
    SWIG_CONSTANT_FROM_GETTER(int,SERVER_JSONRPCSERVER);
    SWIG_CONSTANT_FROM_GETTER(int,SERVER_ZEROCONF);

    SWIG_CONSTANT_FROM_GETTER(int,PLAYLIST_MUSIC);
    SWIG_CONSTANT_FROM_GETTER(int,PLAYLIST_VIDEO);
    SWIG_CONSTANT_FROM_GETTER(int,PLAYER_CORE_AUTO);
    SWIG_CONSTANT_FROM_GETTER(int,PLAYER_CORE_DVDPLAYER);
    SWIG_CONSTANT_FROM_GETTER(int,PLAYER_CORE_MPLAYER);
    SWIG_CONSTANT_FROM_GETTER(int,PLAYER_CORE_PAPLAYER);
    SWIG_CONSTANT_FROM_GETTER(int,TRAY_OPEN);
    SWIG_CONSTANT_FROM_GETTER(int,DRIVE_NOT_READY);
    SWIG_CONSTANT_FROM_GETTER(int,TRAY_CLOSED_NO_MEDIA);
    SWIG_CONSTANT_FROM_GETTER(int,TRAY_CLOSED_MEDIA_PRESENT);
    SWIG_CONSTANT_FROM_GETTER(int,LOGDEBUG);
    SWIG_CONSTANT_FROM_GETTER(int,LOGINFO);
    SWIG_CONSTANT_FROM_GETTER(int,LOGNOTICE);
    SWIG_CONSTANT_FROM_GETTER(int,LOGWARNING);
    SWIG_CONSTANT_FROM_GETTER(int,LOGERROR);
    SWIG_CONSTANT_FROM_GETTER(int,LOGSEVERE);
    SWIG_CONSTANT_FROM_GETTER(int,LOGFATAL);
    SWIG_CONSTANT_FROM_GETTER(int,LOGNONE);

    SWIG_CONSTANT_FROM_GETTER(bool,abortRequested);

    SWIG_CONSTANT_FROM_GETTER(int,CAPTURE_STATE_WORKING);
    SWIG_CONSTANT_FROM_GETTER(int,CAPTURE_STATE_DONE);
    SWIG_CONSTANT_FROM_GETTER(int,CAPTURE_STATE_FAILED);

#ifndef SWIG
    void setabortRequested(bool abortRequested);
#endif
  }
}
