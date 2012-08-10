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

#include <sstream>

#include "ListItem.h"
#include "AddonUtils.h"

#include "video/VideoInfoTag.h"
#include "music/tags/MusicInfoTag.h"
#include "pictures/PictureInfoTag.h"
#include "utils/log.h"
#include "utils/Variant.h"
#include "utils/StringUtils.h"
#include "settings/AdvancedSettings.h"

namespace XBMCAddon
{
  namespace xbmcgui
  {
    ListItem::ListItem(const String& label, 
                       const String& label2,
                       const String& iconImage,
                       const String& thumbnailImage,
                       const String& path) : AddonClass("ListItem")
    {
      item.reset();

      // create CFileItem
      item.reset(new CFileItem());
      if (!item) // not sure if this is really possible
        return;

      if (isSet(label))
        item->SetLabel( label );
      if (isSet(label2))
        item->SetLabel2( label2 );
      if (isSet(iconImage))
        item->SetIconImage( iconImage );
      if (isSet(thumbnailImage))
        item->SetThumbnailImage( thumbnailImage );
      if (isSet(path))
        item->SetPath(path);
    }

    ListItem::~ListItem()
    {
      item.reset();
    }

    CStdString ListItem::getLabel()
    {
      if (!item) return "";

      CStdString ret;
      {
        LOCKGUI;
        ret = item->GetLabel();
      }

      return ret;
    }

    CStdString ListItem::getLabel2()
    {
      if (!item) return "";

      CStdString ret;
      {
        LOCKGUI;
        ret = item->GetLabel2();
      }

      return ret;
    }

    void ListItem::setLabel(const CStdString& label)
    {
      if (!item) return;
      // set label
      {
        LOCKGUI;
        item->SetLabel(label);
      }
    }

    void ListItem::setLabel2(const CStdString& label)
    {
      if (!item) return;
      // set label
      {
        LOCKGUI;
        item->SetLabel2(label);
      }
    }

    void ListItem::setIconImage(const String& iconImage)
    {
      if (!item) return;
      {
        LOCKGUI;
        item->SetIconImage(iconImage);
      }
    }

    void ListItem::setThumbnailImage(const String& thumbFilename)
    {
      if (!item) return;
      {
        LOCKGUI;
        item->SetThumbnailImage(thumbFilename);
      }
    }

    void ListItem::select(bool selected)
    {
      if (!item) return;
      {
        LOCKGUI;
        item->Select(selected);
      }
    }


    bool ListItem::isSelected()
    {
      if (!item) return false;

      bool ret;
      {
        LOCKGUI;
        ret = item->IsSelected();
      }

      return ret;
    }

    void ListItem::setProperty(const char * key, const String& value)
    {
      LOCKGUI;
      CStdString lowerKey = key;
      if (lowerKey.CompareNoCase("startoffset") == 0)
      { // special case for start offset - don't actually store in a property,
        // we store it in item.m_lStartOffset instead
        item->m_lStartOffset = (int)(atof(value.c_str()) * 75.0); // we store the offset in frames, or 1/75th of a second
      }
      else if (lowerKey.CompareNoCase("mimetype") == 0)
      { // special case for mime type - don't actually stored in a property,
        item->SetMimeType(value.c_str());
      }
      else if (lowerKey.CompareNoCase("specialsort") == 0)
      {
        if (value == "bottom")
          item->SetSpecialSort(SortSpecialOnBottom);
        else if (value == "top")
          item->SetSpecialSort(SortSpecialOnTop);
      }
      else
        item->SetProperty(lowerKey.ToLower(), value.c_str());
    }

    CStdString ListItem::getProperty(const char* key)
    {
      LOCKGUI;
      CStdString lowerKey = key;
      CStdString value;
      if (lowerKey.CompareNoCase("startoffset") == 0)
      { // special case for start offset - don't actually store in a property,
        // we store it in item.m_lStartOffset instead
        value.Format("%f", item->m_lStartOffset / 75.0);
      }
      else
        value = item->GetProperty(lowerKey.ToLower()).asString();

      return value;
    }

    void ListItem::setPath(const CStdString& path)
    {
      LOCKGUI;
      item->SetPath(path);
    }

    void ListItem::setMimeType(const CStdString& mimetype)
    {
      LOCKGUI;
      item->SetMimeType(mimetype);
    }

    CStdString ListItem::getdescription()
    {
      return item->GetLabel();
    }

    CStdString ListItem::getduration()
    {
      if (item->LoadMusicTag())
        {
          std::ostringstream oss;
          oss << item->GetMusicInfoTag()->GetDuration();
          return oss.str();
        }

      if (item->HasVideoInfoTag())
        return item->GetVideoInfoTag()->m_strRuntime;

      return "0";
    }

    CStdString ListItem::getfilename()
    {
      return item->GetPath();
    }

    /**
     * setInfo(type, infoLabels) -- Sets the listitem's infoLabels.
     * 
     * type           : string - type of media(video/music/pictures).
     * infoLabels     : dictionary - pairs of { label: value }.
     * 
     * *Note, To set pictures exif info, prepend 'exif:' to the label. Exif values must be passed
     *        as strings, separate value pairs with a comma. (eg. {'exif:resolution': '720,480'}
     *        See CPictureInfoTag::TranslateString in PictureInfoTag.cpp for valid strings.
     * 
     *        You can use the above as keywords for arguments and skip certain optional arguments.
     *        Once you use a keyword, all following arguments require the keyword.
     * 
     * General Values that apply to all types:
     *     count       : integer (12) - can be used to store an id for later, or for sorting purposes
     *     size        : long (1024) - size in bytes
     *     date        : string (%d.%m.%Y / 01.01.2009) - file date
     * 
     * Video Values:
     *     genre       : string (Comedy)
     *     year        : integer (2009)
     *     episode     : integer (4)
     *     season      : integer (1)
     *     top250      : integer (192)
     *     tracknumber : integer (3)
     *     rating      : float (6.4) - range is 0..10
     *     watched     : depreciated - use playcount instead
     *     playcount   : integer (2) - number of times this item has been played
     *     overlay     : integer (2) - range is 0..8.  See GUIListItem.h for values
     *     cast        : list (Michal C. Hall)
     *     castandrole : list (Michael C. Hall|Dexter)
     *     director    : string (Dagur Kari)
     *     mpaa        : string (PG-13)
     *     plot        : string (Long Description)
     *     plotoutline : string (Short Description)
     *     title       : string (Big Fan)
     *     duration    : string (3:18)
     *     studio      : string (Warner Bros.)
     *     tagline     : string (An awesome movie) - short description of movie
     *     writer      : string (Robert D. Siegel)
     *     tvshowtitle : string (Heroes)
     *     premiered   : string (2005-03-04)
     *     status      : string (Continuing) - status of a TVshow
     *     code        : string (tt0110293) - IMDb code
     *     aired       : string (2008-12-07)
     *     credits     : string (Andy Kaufman) - writing credits
     *     lastplayed  : string (%Y-%m-%d %h:%m:%s = 2009-04-05 23:16:04)
     *     album       : string (The Joshua Tree)
     *     artist      : list (['U2'])
     *     votes       : string (12345 votes)
     *     trailer     : string (/home/user/trailer.avi)
     *     dateadded   : string (%Y-%m-%d %h:%m:%s = 2009-04-05 23:16:04)
     * 
     * Music Values:
     *     tracknumber : integer (8)
     *     duration    : integer (245) - duration in seconds
     *     year        : integer (1998)
     *     genre       : string (Rock)
     *     album       : string (Pulse)
     *     artist      : string (Muse)
     *     title       : string (American Pie)
     *     rating      : string (3) - single character between 0 and 5
     *     lyrics      : string (On a dark desert highway...)
     * 
     * Picture Values:
     *     title       : string (In the last summer-1)
     *     picturepath : string (/home/username/pictures/img001.jpg)
     *     exif*       : string (See CPictureInfoTag::TranslateString in PictureInfoTag.cpp for valid strings)
     * 
     * example:
     *   - self.list.getSelectedItem().setInfo('video', { 'Genre': 'Comedy' })
     */
    void ListItem::setInfo(const char* type, const Dictionary& infoLabels)
    {
      LOCKGUI;

      if (strcmpi(type, "video") == 0)
      {
        for (Dictionary::const_iterator it = infoLabels.begin(); it != infoLabels.end(); it++)
        {
          const CStdString& key = it->first;
          const CStdString& value = it->second;

          if (key == "year")
            item->GetVideoInfoTag()->m_iYear = strtol(value, NULL, 10);
          else if (key == "episode")
            item->GetVideoInfoTag()->m_iEpisode = strtol(value, NULL, 10);
          else if (key == "season")
            item->GetVideoInfoTag()->m_iSeason = strtol(value, NULL, 10);
          else if (key == "top250")
            item->GetVideoInfoTag()->m_iTop250 = strtol(value, NULL, 10);
          else if (key == "tracknumber")
            item->GetVideoInfoTag()->m_iTrack = strtol(value, NULL, 10);
          else if (key == "count")
            item->m_iprogramCount = strtol(value, NULL, 10);
          else if (key == "rating")
            item->GetVideoInfoTag()->m_fRating = (float)strtod(value, NULL);
          else if (key == "size")
            item->m_dwSize = (int64_t)strtoll(value, NULL, 10);
          else if (key == "watched") // backward compat - do we need it?
            item->GetVideoInfoTag()->m_playCount = strtol(value, NULL, 10);
          else if (key == "playcount")
            item->GetVideoInfoTag()->m_playCount = strtol(value, NULL, 10);
          else if (key == "overlay")
          {
            long overlay = strtol(value, NULL, 10);
            if (overlay >= 0 && overlay <= 8)
              item->SetOverlayImage((CGUIListItem::GUIIconOverlay)overlay);
          }
// TODO: This is a dynamic type for the value where a list is expected as the 
//   Dictionary value.
//          else if (key == "cast" || key == "castandrole")
//          {
//            if (!PyObject_TypeCheck(value, &PyList_Type)) continue;
//            item->GetVideoInfoTag()->m_cast.clear();
//            for (int i = 0; i < PyList_Size(value); i++)
//            {
//              PyObject *pTuple = NULL;
//              pTuple = PyList_GetItem(value, i);
//              if (pTuple == NULL) continue;
//              PyObject *pActor = NULL;
//              PyObject *pRole = NULL;
//              if (PyObject_TypeCheck(pTuple, &PyTuple_Type))
//              {
//                if (!PyArg_ParseTuple(pTuple, (char*)"O|O", &pActor, &pRole)) continue;
//              }
//              else
//                pActor = pTuple;
//              SActorInfo info;
//              if (!PyXBMCGetUnicodeString(info.strName, pActor, 1)) continue;
//              if (pRole != NULL)
//                PyXBMCGetUnicodeString(info.strRole, pRole, 1);
//              item->GetVideoInfoTag()->m_cast.push_back(info);
//            }
//          }
//          else if (strcmpi(PyString_AsString(key), "artist") == 0)
//          {
//            if (!PyObject_TypeCheck(value, &PyList_Type)) continue;
//            self->item->GetVideoInfoTag()->m_artist.clear();
//            for (int i = 0; i < PyList_Size(value); i++)
//            {
//              PyObject *pActor = PyList_GetItem(value, i);
//              if (pActor == NULL) continue;
//              CStdString actor;
//              if (!PyXBMCGetUnicodeString(actor, pActor, 1)) continue;
//              self->item->GetVideoInfoTag()->m_artist.push_back(actor);
//            }
//          }
          else if (key == "genre")
            item->GetVideoInfoTag()->m_genre = StringUtils::Split(value, g_advancedSettings.m_videoItemSeparator);
          else if (key == "director")
            item->GetVideoInfoTag()->m_director = StringUtils::Split(value, g_advancedSettings.m_videoItemSeparator);
          else if (key == "mpaa")
            item->GetVideoInfoTag()->m_strMPAARating = value;
          else if (key == "plot")
            item->GetVideoInfoTag()->m_strPlot = value;
          else if (key == "plotoutline")
            item->GetVideoInfoTag()->m_strPlotOutline = value;
          else if (key == "title")
            item->GetVideoInfoTag()->m_strTitle = value;
          else if (key == "originaltitle")
            item->GetVideoInfoTag()->m_strOriginalTitle = value;
          else if (key == "duration")
            item->GetVideoInfoTag()->m_strRuntime = value;
          else if (key == "studio")
            item->GetVideoInfoTag()->m_studio = StringUtils::Split(value, g_advancedSettings.m_videoItemSeparator);            
          else if (key == "tagline")
            item->GetVideoInfoTag()->m_strTagLine = value;
          else if (key == "writer")
            item->GetVideoInfoTag()->m_writingCredits = StringUtils::Split(value, g_advancedSettings.m_videoItemSeparator);
          else if (key == "tvshowtitle")
            item->GetVideoInfoTag()->m_strShowTitle = value;
          else if (key == "premiered")
            item->GetVideoInfoTag()->m_premiered.SetFromDateString(value);
          else if (key == "status")
            item->GetVideoInfoTag()->m_strStatus = value;
          else if (key == "code")
            item->GetVideoInfoTag()->m_strProductionCode = value;
          else if (key == "aired")
            item->GetVideoInfoTag()->m_firstAired.SetFromDateString(value);
          else if (key == "credits")
            item->GetVideoInfoTag()->m_writingCredits = StringUtils::Split(value, g_advancedSettings.m_videoItemSeparator);
          else if (key == "lastplayed")
            item->GetVideoInfoTag()->m_lastPlayed.SetFromDBDateTime(value);
          else if (key == "album")
            item->GetVideoInfoTag()->m_strAlbum = value;
          else if (key == "votes")
            item->GetVideoInfoTag()->m_strVotes = value;
          else if (key == "trailer")
            item->GetVideoInfoTag()->m_strTrailer = value;
          else if (key == "date")
          {
            if (strlen(value) == 10)
              item->m_dateTime.SetDate(atoi(value.Right(4).c_str()), atoi(value.Mid(3,4).c_str()), atoi(value.Left(2).c_str()));
            else
              CLog::Log(LOGERROR,"NEWADDON Invalid Date Format \"%s\"",value.c_str());
          }
          else if (key == "dateadded")
            item->GetVideoInfoTag()->m_dateAdded.SetFromDBDateTime(value.c_str());
        }
      }
      else if (strcmpi(type, "music"))
      {
        for (Dictionary::const_iterator it = infoLabels.begin(); it != infoLabels.end(); it++)
        {
          const CStdString& key = it->first;
          const CStdString& value = it->second;

          // TODO: add the rest of the infolabels
          if (key == "tracknumber")
            item->GetMusicInfoTag()->SetTrackNumber(strtol(value, NULL, 10));
          else if (key == "count")
            item->m_iprogramCount = strtol(value, NULL, 10);
          else if (key == "size")
            item->m_dwSize = (int64_t)strtoll(value, NULL, 10);
          else if (key == "duration")
            item->GetMusicInfoTag()->SetDuration(strtol(value, NULL, 10));
          else if (key == "year")
            item->GetMusicInfoTag()->SetYear(strtol(value, NULL, 10));
          else if (key == "listeners")
            item->GetMusicInfoTag()->SetListeners(strtol(value, NULL, 10));
          else if (key == "playcount")
            item->GetMusicInfoTag()->SetPlayCount(strtol(value, NULL, 10));
          else if (key == "genre")
            item->GetMusicInfoTag()->SetGenre(value);
          else if (key == "album")
            item->GetMusicInfoTag()->SetAlbum(value);
          else if (key == "artist")
            item->GetMusicInfoTag()->SetArtist(value);
          else if (key == "title")
            item->GetMusicInfoTag()->SetTitle(value);
          else if (key == "rating")
            item->GetMusicInfoTag()->SetRating(*value);
          else if (key == "lyrics")
            item->GetMusicInfoTag()->SetLyrics(value);
          else if (key == "lastplayed")
            item->GetMusicInfoTag()->SetLastPlayed(value);
          else if (key == "musicbrainztrackid")
            item->GetMusicInfoTag()->SetMusicBrainzTrackID(value);
          else if (key == "musicbrainzartistid")
            item->GetMusicInfoTag()->SetMusicBrainzArtistID(value);
          else if (key == "musicbrainzalbumid")
            item->GetMusicInfoTag()->SetMusicBrainzAlbumID(value);
          else if (key == "musicbrainzalbumartistid")
            item->GetMusicInfoTag()->SetMusicBrainzAlbumArtistID(value);
          else if (key == "musicbrainztrmid")
            item->GetMusicInfoTag()->SetMusicBrainzTRMID(value);
          else if (key == "comment")
            item->GetMusicInfoTag()->SetComment(value);
          else if (key == "date")
          {
            if (strlen(value) == 10)
              item->m_dateTime.SetDate(atoi(value.Right(4)), atoi(value.Mid(3,4)), atoi(value.Left(2)));
          }
          else
            CLog::Log(LOGERROR,"NEWADDON Unknown Music Info Key \"%s\"", key.c_str());

          // This should probably be set outside of the loop but since the original
          //  implementation set it inside of the loop, I'll leave it that way. - Jim C.
          item->GetMusicInfoTag()->SetLoaded(true);
        }
      }
      else if (strcmpi(type,"pictures"))
      {
        for (Dictionary::const_iterator it = infoLabels.begin(); it != infoLabels.end(); it++)
        {
          const CStdString& key = it->first;
          const CStdString& value = it->second;

          if (key == "count")
            item->m_iprogramCount = strtol(value, NULL, 10);
          else if (key == "size")
            item->m_dwSize = (int64_t)strtoll(value, NULL, 10);
          else if (key == "title")
            item->m_strTitle = value;
          else if (key == "picturepath")
            item->SetPath(value);
          else if (key == "date")
          {
            if (strlen(value) == 10)
              item->m_dateTime.SetDate(atoi(value.Right(4)), atoi(value.Mid(3,4)), atoi(value.Left(2)));
          }
          else
          {
            const CStdString& exifkey = key;
            if (!exifkey.Left(5).Equals("exif:") || exifkey.length() < 6) continue;
            int info = CPictureInfoTag::TranslateString(exifkey.Mid(5));
            item->GetPictureInfoTag()->SetInfo(info, value);
          }

          // This should probably be set outside of the loop but since the original
          //  implementation set it inside of the loop, I'll leave it that way. - Jim C.
          item->GetPictureInfoTag()->SetLoaded(true);
        }
      }
    } // end ListItem::setInfo

    /**
     * addStreamInfo(type, values) -- Add a stream with details.
     * 
     * type              : string - type of stream(video/audio/subtitle).
     * values            : dictionary - pairs of { label: value }.
     * 
     * Video Values:
     *     codec         : string (h264)
     *     aspect        : float (1.78)
     *     width         : integer (1280)
     *     height        : integer (720)
     *     duration      : integer (seconds)
     * 
     * Audio Values:
     *     codec         : string (dts)
     *     language      : string (en)
     *     channels      : integer (2)
     * 
     * Subtitle Values:
     *     language      : string (en)
     * 
     * example:
     *   - self.list.getSelectedItem().addStreamInfo('video', { 'Codec': 'h264', 'Width' : 1280 })
    */
    void ListItem::addStreamInfo(const char* cType, const Dictionary& dictionary)
    {
      LOCKGUI;

      CStdString tmp;
      if (strcmpi(cType, "video") == 0)
      {
        CStreamDetailVideo* video = new CStreamDetailVideo;
        for (Dictionary::const_iterator it = dictionary.begin(); it != dictionary.end(); it++)
        {
          const CStdString& key = it->first;
          const CStdString& value = it->second;

          if (key == "codec")
            video->m_strCodec = value;
          else if (key == "aspect")
            video->m_fAspect = (float)atof(value);
          else if (key == "width")
            video->m_iWidth = strtol(value, NULL, 10);
          else if (key == "height")
            video->m_iHeight = strtol(value, NULL, 10);
          else if (key == "duration")
            video->m_iDuration = strtol(value, NULL, 10);
        }
        item->GetVideoInfoTag()->m_streamDetails.AddStream(video);
      }
      else if (strcmpi(cType, "audio") == 0)
      {
        CStreamDetailAudio* audio = new CStreamDetailAudio;
        for (Dictionary::const_iterator it = dictionary.begin(); it != dictionary.end(); it++)
        {
          const CStdString& key = it->first;
          const CStdString& value = it->second;

          if (key == "codec")
            audio->m_strCodec = value;
          else if (key == "language")
            audio->m_strLanguage = value;
          else if (key == "channels")
            audio->m_iChannels = strtol(value, NULL, 10);
        }
        item->GetVideoInfoTag()->m_streamDetails.AddStream(audio);
      }
      else if (strcmpi(cType, "subtitle") == 0)
      {
        CStreamDetailSubtitle* subtitle = new CStreamDetailSubtitle;
        for (Dictionary::const_iterator it = dictionary.begin(); it != dictionary.end(); it++)
        {
          const CStdString& key = it->first;
          const CStdString& value = it->second;

          if (key == "language")
            subtitle->m_strLanguage = value;
        }
        item->GetVideoInfoTag()->m_streamDetails.AddStream(subtitle);
      }
    } // end ListItem::addStreamInfo

    /**
     * addContextMenuItems([(label, action,)*], replaceItems) -- Adds item(s) to the context menu for media lists.
     * 
     * items               : list - [(label, action,)*] A list of tuples consisting of label and action pairs.
     *   - label           : string or unicode - item's label.
     *   - action          : string or unicode - any built-in function to perform.
     * replaceItems        : [opt] bool - True=only your items will show/False=your items will be added to context menu(Default).
     * 
     * List of functions - http://wiki.xbmc.org/?title=List_of_Built_In_Functions 
     * 
     * *Note, You can use the above as keywords for arguments and skip certain optional arguments.
     *        Once you use a keyword, all following arguments require the keyword.
     * 
     * example:
     *   - listitem.addContextMenuItems([('Theater Showtimes', 'XBMC.RunScript(special://home/scripts/showtimes/default.py,Iron Man)',)])\n
     */
    void ListItem::addContextMenuItems(const std::vector<std::vector<CStdString> >& items, bool replaceItems /* = false */)
    {
      
    } // end addContextMenuItems
  }
}
