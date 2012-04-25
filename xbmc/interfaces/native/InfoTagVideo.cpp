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

#include "InfoTagVideo.h"
#include "utils/StringUtils.h"
#include "settings/AdvancedSettings.h"

namespace XBMCAddon
{
  namespace xbmc
  {
    InfoTagVideo::InfoTagVideo() : AddonClass("InfoTagVideo")
    {
      infoTag = new CVideoInfoTag();
    }

    InfoTagVideo::InfoTagVideo(const CVideoInfoTag& tag) : AddonClass("InfoTagVideo")
    {
      infoTag = new CVideoInfoTag();
      *infoTag = tag;
    }

    InfoTagVideo::~InfoTagVideo()
    {
      delete infoTag;
    }

    // InfoTagVideo_GetDirector
    CStdString InfoTagVideo::getDirector()
    {
      return StringUtils::Join(infoTag->m_director, g_advancedSettings.m_videoItemSeparator);
    }

    // InfoTagVideo_GetWritingCredits
    CStdString InfoTagVideo::getWritingCredits()
    {
      return StringUtils::Join(infoTag->m_writingCredits, g_advancedSettings.m_videoItemSeparator);
    }

    // InfoTagVideo_GetGenre
    CStdString InfoTagVideo::getGenre()
    {
      return StringUtils::Join(infoTag->m_genre, g_advancedSettings.m_videoItemSeparator).c_str();
    }

    // InfoTagVideo_GetTagLine
    CStdString InfoTagVideo::getTagLine()
    {
      return infoTag->m_strTagLine;
    }

    // InfoTagVideo_GetPlotOutline
    CStdString InfoTagVideo::getPlotOutline()
    {
      return infoTag->m_strPlotOutline;
    }

    // InfoTagVideo_GetPlot
    CStdString InfoTagVideo::getPlot()
    {
      return infoTag->m_strPlot;
    }

    // InfoTagVideo_GetPictureURL
    CStdString InfoTagVideo::getPictureURL()
    {
      return infoTag->m_strPictureURL.GetFirstThumb().m_url;
    }

    // InfoTagVideo_GetTitle
    CStdString InfoTagVideo::getTitle()
    {
      return infoTag->m_strTitle;
    }

    // InfoTagVideo_GetVotes
    CStdString InfoTagVideo::getVotes()
    {
      return infoTag->m_strVotes;
    }

    // InfoTagVideo_GetCast
    CStdString InfoTagVideo::getCast()
    {
      return infoTag->GetCast(true);
    }

    // InfoTagVideo_GetFile
    CStdString InfoTagVideo::getFile()
    {
      return infoTag->m_strFile;
    }

    // InfoTagVideo_GetPath
    CStdString InfoTagVideo::getPath()
    {
      return infoTag->m_strPath;
    }

    // InfoTagVideo_GetIMDBNumber
    CStdString InfoTagVideo::getIMDBNumber()
    {
      return infoTag->m_strIMDBNumber;
    }

    // InfoTagVideo_GetYear
    int InfoTagVideo::getYear()
    {
      return infoTag->m_iYear;
    }

    // InfoTagVideo_GetRating
    double InfoTagVideo::getRating()
    {
      return infoTag->m_fRating;
    }

    // InfoTagVideo_GetPlayCount
    /**
     * getPlayCount() -- returns a integer.
     */
    int InfoTagVideo::getPlayCount()
    {
      return infoTag->m_playCount;
    }

    // InfoTagVideo_GetLastPlayed
    /**
     * getLastPlayed() -- returns a string.
     */
    String InfoTagVideo::getLastPlayed()
    {
      return infoTag->m_lastPlayed.GetAsLocalizedDateTime();
    }

    String InfoTagVideo::getOriginalTitle()
    {
      return infoTag->m_strOriginalTitle;
    }

    String InfoTagVideo::getPremiered()
    {
      return infoTag->m_premiered.GetAsLocalizedDate();
    }

    String InfoTagVideo::getFirstAired()
    {
      return infoTag->m_firstAired.GetAsLocalizedDate();
    }
  }
}
