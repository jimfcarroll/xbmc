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

#include "Player.h"
#include "ListItem.h"
#include "PlayList.h"
#include "PlayListPlayer.h"
#include "settings/Settings.h"
#include "Application.h"
#include "ApplicationMessenger.h"
#include "GUIInfoManager.h"
#include "AddonUtils.h"
#include "utils/LangCodeExpander.h"
#include "utils/log.h"

namespace XBMCAddon
{
  namespace xbmc
  {
    Player::Player(int _playerCore): AddonCallback("Player")
    {
      iPlayList = PLAYLIST_MUSIC;

      if (_playerCore == EPC_DVDPLAYER ||
          _playerCore == EPC_MPLAYER ||
          _playerCore == EPC_PAPLAYER)
        playerCore = (EPLAYERCORES)_playerCore;
      else
        playerCore = EPC_NONE;

      // now that we're done, register hook me into the system
      if (languageHook)
        languageHook->registerPlayerCallback(this);
    }

    Player::~Player()
    {
      deallocating();

      // we're shutting down so unregister me.
      if (languageHook)
        languageHook->unregisterPlayerCallback(this);
    }

    /**
     * playStream([item, listitem, windowed]) -- Play this item.
     * 
     * item           : [opt] string - filename or url.
     * listitem       : [opt] listitem - used with setInfo() to set different infolabels.
     * windowed       : [opt] bool - true=play video windowed, false=play users preference.(default)
     * 
     * *Note, If item is not given then the Player will try to play the current item
     *        in the current playlist.
     * 
     *        You can use the above as keywords for arguments and skip certain optional arguments.
     *        Once you use a keyword, all following arguments require the keyword.
     * 
     * example:
     *   - listitem = xbmcgui.ListItem('Ironman')
     *   - listitem.setInfo('video', {'Title': 'Ironman', 'Genre': 'Science Fiction'})
     *   - xbmc.Player( xbmc.PLAYER_CORE_MPLAYER ).playStream(url, listitem, windowed)
     */
    void Player::playStream(const String& item, const xbmcgui::ListItem* plistitem, bool windowed)
    {
      TRACE;
      if (item != nullString)
      {
        // set fullscreen or windowed
        g_settings.m_bStartVideoWindowed = windowed;

        // force a playercore before playing
        g_application.m_eForcedNextPlayer = playerCore;

        const AddonClass::Ref<xbmcgui::ListItem> listitem(plistitem);

        if (listitem.isSet())
        {
          // set m_strPath to the passed url
          listitem->item->SetPath(item.c_str());
          CApplicationMessenger::Get().PlayFile((const CFileItem)(*(listitem->item)), false);
        }
        else
        {
          CFileItem nextitem(item, false);
          CApplicationMessenger::Get().MediaPlay(nextitem.GetPath());
        }
      }
      else
        playCurrent(windowed);
    }

    /**
     * play() -- try to play the current item in the current playlist.
     *
     * windowed       : [opt] bool - true=play video windowed, false=play users preference.(default)
     * 
     * example:
     *   - xbmc.Player( xbmc.PLAYER_CORE_MPLAYER ).play()
     */
    void Player::playCurrent(bool windowed)
    {
      TRACE;
      // set fullscreen or windowed
      g_settings.m_bStartVideoWindowed = windowed;

      // force a playercore before playing
      g_application.m_eForcedNextPlayer = playerCore;

      // play current file in playlist
      if (g_playlistPlayer.GetCurrentPlaylist() != iPlayList)
        g_playlistPlayer.SetCurrentPlaylist(iPlayList);
      CApplicationMessenger::Get().PlayListPlayerPlay(g_playlistPlayer.GetCurrentSong());
    }

    /**
     * playPlaylist([playlist, windowed]) -- Play this item.
     * 
     * playlist       : [opt] playlist.
     * windowed       : [opt] bool - true=play video windowed, false=play users preference.(default)
     * 
     * *Note, If playlist is not given then the Player will try to play the current item
     *        in the current playlist.
     * 
     *        You can use the above as keywords for arguments and skip certain optional arguments.
     *        Once you use a keyword, all following arguments require the keyword.
     * 
     * example:
     */
    void Player::playPlaylist(const PlayList* playlist, bool windowed)
    {
      TRACE;
      if (playlist != NULL)
      {
        // set fullscreen or windowed
        g_settings.m_bStartVideoWindowed = windowed;

        // force a playercore before playing
        g_application.m_eForcedNextPlayer = playerCore;

        // play a python playlist (a playlist from playlistplayer.cpp)
        iPlayList = playlist->getPlayListId();
        g_playlistPlayer.SetCurrentPlaylist(iPlayList);
        CApplicationMessenger::Get().PlayListPlayerPlay();
      }
      else
        playCurrent(windowed);
    }

    // Player_Stop
    /**
     * stop() -- Stop playing.
     */
    void Player::stop()
    {
      TRACE;
      CApplicationMessenger::Get().MediaStop();
    }

    // Player_Pause
    /**
     * pause() -- Pause playing.
     */
    void Player::pause()
    {
      TRACE;
      CApplicationMessenger::Get().MediaPause();
    }

    // Player_PlayNext
    /**
     * playnext() -- Play next item in playlist.
     */
    void Player::playnext()
    {
      TRACE;
      // force a playercore before playing
      g_application.m_eForcedNextPlayer = playerCore;

      CApplicationMessenger::Get().PlayListPlayerNext();
    }

    // Player_PlayPrevious
    /**
     * playprevious() -- Play previous item in playlist.
     */
    void Player::playprevious()
    {
      TRACE;
      // force a playercore before playing
      g_application.m_eForcedNextPlayer = playerCore;

      CApplicationMessenger::Get().PlayListPlayerPrevious();
    }

    // Player_PlaySelected
    /**
     * playselected(selected) -- Play a certain item from the current playlist.
     *
     * selected is the number of the selected item
     */
    void Player::playselected(int selected)
    {
      TRACE;
      // force a playercore before playing
      g_application.m_eForcedNextPlayer = playerCore;

      if (g_playlistPlayer.GetCurrentPlaylist() != iPlayList)
      {
        g_playlistPlayer.SetCurrentPlaylist(iPlayList);
      }
      g_playlistPlayer.SetCurrentSong(selected);

      CApplicationMessenger::Get().PlayListPlayerPlay(selected);
      //g_playlistPlayer.Play(selected);
      //CLog::Log(LOGNOTICE, "Current Song After Play: %i", g_playlistPlayer.GetCurrentSong());
    }

    void Player::OnPlayBackStarted()
    { 
      TRACE;
      handleCallback(new VoidCallbackFunction<Player>(this,&Player::onPlayBackStarted));
    }

    void Player::OnPlayBackEnded()
    { 
      TRACE;
      handleCallback(new VoidCallbackFunction<Player>(this,&Player::onPlayBackEnded));
    }

    void Player::OnPlayBackStopped()
    { 
      TRACE;
      handleCallback(new VoidCallbackFunction<Player>(this,&Player::onPlayBackStopped));
    }

    void Player::OnPlayBackPaused()
    { 
      TRACE;
      handleCallback(new VoidCallbackFunction<Player>(this,&Player::onPlayBackPaused));
    }

    void Player::OnPlayBackResumed()
    { 
      TRACE;
      handleCallback(new VoidCallbackFunction<Player>(this,&Player::onPlayBackResumed));
    }

    void Player::OnQueueNextItem()
    { 
      TRACE;
      handleCallback(new VoidCallbackFunction<Player>(this,&Player::onQueueNextItem));
    }

    // Player_OnPlayBackStarted
    /**
     * onPlayBackStarted() -- onPlayBackStarted method.
     * 
     * Will be called when xbmc starts playing a file
     */
    void Player::onPlayBackStarted()
    {
      TRACE;
    }

    // Player_OnPlayBackEnded
    /**
     * onPlayBackEnded() -- onPlayBackEnded method.
     * 
     * Will be called when xbmc stops playing a file
     */
    void Player::onPlayBackEnded()
    {
      TRACE;
    }

    // Player_OnPlayBackStopped
    /**
     * onPlayBackStopped() -- onPlayBackStopped method.
     * 
     * Will be called when user stops xbmc playing a file
     */
    void Player::onPlayBackStopped()
    {
      TRACE;
    }

    // Player_OnPlayBackPaused
    /**
     * onPlayBackPaused() -- onPlayBackPaused method.
     * 
     * Will be called when user pauses a playing file
     */
    void Player::onPlayBackPaused()
    {
      TRACE;
    }

    // Player_OnPlayBackResumed
    /**
     * onPlayBackResumed() -- onPlayBackResumed method.
     * 
     * Will be called when user resumes a paused file
     */
    void Player::onPlayBackResumed()
    {
      TRACE;
    }

    /**
     * onQueueNextItem() -- onQueueNextItem method.
     * 
     * Will be called when user queues the next item
     */
    void Player::onQueueNextItem()
    {
      TRACE;
    }

    // Player_IsPlaying
    /**
     * isPlaying() -- returns True is xbmc is playing a file.
     */
    bool Player::isPlaying()
    {
      TRACE;
      return g_application.IsPlaying();
    }

    // Player_IsPlayingAudio
    /**
     * isPlayingAudio() -- returns True is xbmc is playing an audio file.
     */
    bool Player::isPlayingAudio()
    {
      TRACE;
      return g_application.IsPlayingAudio();
    }

    // Player_IsPlayingVideo
    /**
     * isPlayingVideo() -- returns True if xbmc is playing a video.
     */
    bool Player::isPlayingVideo()
    {
      TRACE;
      return g_application.IsPlayingVideo();
    }

    // Player_GetPlayingFile
    /**
     * getPlayingFile() -- returns the current playing file as a string.
     * 
     * Throws: Exception, if player is not playing a file.
     */
    CStdString Player::getPlayingFile() throw (PlayerException)
    {
      TRACE;
      if (!g_application.IsPlaying())
        throw PlayerException("XBMC is not playing any file");

      return g_application.CurrentFile();
    }

    // Player_GetVideoInfoTag
    /**
     * getVideoInfoTag() -- returns the VideoInfoTag of the current playing Movie.
     * 
     * Throws: Exception, if player is not playing a file or current file is not a movie file.
     * 
     * Note, this doesn't work yet, it's not tested
     * 
     */
    InfoTagVideo* Player::getVideoInfoTag() throw (PlayerException)
    {
      TRACE;
      if (!g_application.IsPlayingVideo())
        throw PlayerException("XBMC is not playing any videofile");

      const CVideoInfoTag* movie = g_infoManager.GetCurrentMovieTag();
      if (movie)
        return new InfoTagVideo(*movie);

      return new InfoTagVideo();
    }

    // Player_GetMusicInfoTag
    /**
     *getMusicInfoTag() -- returns the MusicInfoTag of the current playing 'Song'.
     *
     *Throws: Exception, if player is not playing a file or current file is not a music file.\n"
     */
    InfoTagMusic* Player::getMusicInfoTag() throw (PlayerException)
    {
      TRACE;
      if (g_application.IsPlayingVideo() || !g_application.IsPlayingAudio())
        throw PlayerException("XBMC is not playing any music file");

      const MUSIC_INFO::CMusicInfoTag* tag = g_infoManager.GetCurrentSongTag();
      if (tag)
        return new InfoTagMusic(*tag);

      return new InfoTagMusic();
    }

    /**
     *getTotalTime() -- Returns the total time of the current playing media in
     *                  seconds.  This is only accurate to the full second.
     *
     *Throws: Exception, if player is not playing a file.
     */

    double Player::getTotalTime() throw (PlayerException)
    {
      TRACE;
      if (!g_application.IsPlaying())
        throw PlayerException("XBMC is not playing any media file");

      return g_application.GetTotalTime();
    }

    // Player_GetTime
    /**
     *getTime() -- Returns the current time of the current playing media as fractional seconds.
     *
     *Throws: Exception, if player is not playing a file.
     */
    double Player::getTime() throw (PlayerException)
    {
      TRACE;
      if (!g_application.IsPlaying())
        throw PlayerException("XBMC is not playing any media file");

      return g_application.GetTime();
    }

    // Player_SeekTime
    /**
     *seekTime() -- Seeks the specified amount of time as fractional seconds.
     *              The time specified is relative to the beginning of the
     *              currently playing media file.
     *
     *Throws: Exception, if player is not playing a file.
     */
    void Player::seekTime(double pTime) throw (PlayerException)
    {
      TRACE;
      if (!g_application.IsPlaying())
        throw PlayerException("XBMC is not playing any media file");

      g_application.SeekTime( pTime );
    }

    /**
     *setSubtitles() -- set subtitle file and enable subtitles
     */
    void Player::setSubtitles(const char* cLine)
    {
      TRACE;
      if (g_application.m_pPlayer)
      {
        int nStream = g_application.m_pPlayer->AddSubtitle(cLine);
        if(nStream >= 0)
        {
          g_application.m_pPlayer->SetSubtitle(nStream);
          g_application.m_pPlayer->SetSubtitleVisible(true);
        }
      }
    }


    // Player_ShowSubtitles
    /**
     * showSubtitles(visible) -- enable/disable subtitles
     * 
     * visible        : boolean - True for visible subtitles.
     * example:
     * - xbmc.Player().showSubtitles(True)
     */
    void Player::showSubtitles(bool bVisible)
    {
      TRACE;
      if (g_application.m_pPlayer)
      {
        g_settings.m_currentVideoSettings.m_SubtitleOn = bVisible != 0;
        g_application.m_pPlayer->SetSubtitleVisible(bVisible != 0);
      }
    }

    /**
     *getSubtitles() -- get subtitle stream name\n
     */
    String Player::getSubtitles()
    {
      TRACE;
      if (g_application.m_pPlayer)
      {
        int i = g_application.m_pPlayer->GetSubtitle();
        CStdString strName;
        g_application.m_pPlayer->GetSubtitleName(i, strName);

        if (strName == "Unknown(Invalid)")
          strName = "";
        return strName;
      }

      return NULL;
    }


    /**
     *DisableSubtitles() -- disable subtitles
     */
    void Player::disableSubtitles()
    {
      TRACE;
      CLog::Log(LOGWARNING,"'xbmc.Player().disableSubtitles()' is deprecated and will be removed in future releases, please use 'xbmc.Player().showSubtitles(false)' instead");
      if (g_application.m_pPlayer)
      {
        g_settings.m_currentVideoSettings.m_SubtitleOn = false;
        g_application.m_pPlayer->SetSubtitleVisible(false);
      }
    }


    // Player_getAvailableSubtitleStreams
    /**
     * getAvailableSubtitleStreams() -- get Subtitle stream names
     */
    std::vector<String>* Player::getAvailableSubtitleStreams()
    {
      if (g_application.m_pPlayer)
      {
        int subtitleCount = g_application.m_pPlayer->GetSubtitleCount();
        std::vector<String>* ret = new std::vector<String>(subtitleCount);
        for (int iStream=0; iStream < subtitleCount; iStream++)
        {
          CStdString strName;
          CStdString FullLang;
          g_application.m_pPlayer->GetSubtitleName(iStream, strName);
          if (!g_LangCodeExpander.Lookup(FullLang, strName))
            FullLang = strName;
          (*ret)[iStream] = FullLang;
        }
        return ret;
      }

      return NULL;
    }

    // Player_setSubtitleStream
    /**
     * setSubtitleStream(stream) -- set Subtitle Stream 
     * 
     * stream           : int
     * 
     * example:
     *   - setSubtitleStream(1)
     */
    void Player::setSubtitleStream(int iStream)
    {
      if (g_application.m_pPlayer)
      {
        int streamCount = g_application.m_pPlayer->GetSubtitleCount();
        if(iStream < streamCount)
        {
          g_application.m_pPlayer->SetSubtitle(iStream);
          g_application.m_pPlayer->SetSubtitleVisible(true);
        }
      }
    }

    std::vector<String>* Player::getAvailableAudioStreams()
    {
      if (g_application.m_pPlayer)
      {
        int streamCount = g_application.m_pPlayer->GetAudioStreamCount();
        std::vector<String>* ret = new std::vector<String>(streamCount);
        for (int iStream=0; iStream < streamCount; iStream++)
        {  
          CStdString strName;
          CStdString FullLang;
          g_application.m_pPlayer->GetAudioStreamLanguage(iStream, strName);
          g_LangCodeExpander.Lookup(FullLang, strName);
          if (FullLang.IsEmpty())
            g_application.m_pPlayer->GetAudioStreamName(iStream, FullLang);
          (*ret)[iStream] = FullLang;
        }
        return ret;
      }
    
      return NULL;
    } 

    /**
     * setAudioStream(stream) -- set Audio Stream 
     *
     * stream           : int
     *
     * example:
     *    - setAudioStream(1)\n");
     */
    void Player::setAudioStream(int iStream)
    {
      if (g_application.m_pPlayer)
      {
        int streamCount = g_application.m_pPlayer->GetAudioStreamCount();
        if(iStream < streamCount)
          g_application.m_pPlayer->SetAudioStream(iStream);
      }
    }  
  }
}

