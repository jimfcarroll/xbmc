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

#include "PlayList.h"
#include "PlayListPlayer.h"
#include "settings/Settings.h"
#include "Application.h"
#include "playlists/PlayListFactory.h"
#include "utils/URIUtils.h"

using namespace PLAYLIST;

namespace XBMCAddon
{
  namespace xbmc
  {
    // TODO: need a means to check for a valid construction
    //  either by throwing an exception or by an "isValid" check
    PlayList::PlayList(int playList) throw (PlayListException) : 
      AddonClass("PlayList"),
      refs(1), iPlayList(playList), pPlayList(NULL)
    {
      // we do not create our own playlist, just using the ones from playlistplayer
      if (iPlayList != PLAYLIST_MUSIC &&
          iPlayList != PLAYLIST_VIDEO)
        throw PlayListException("PlayList does not exist");

      pPlayList = &g_playlistPlayer.GetPlaylist(playList);
      iPlayList = playList;
    }

    PlayList::~PlayList()  { }

    /**
     * add(url[, listitem, index]) -- Adds a new file to the playlist.
     * 
     * url            : string or unicode - filename or url to add.
     * listitem       : [opt] listitem - used with setInfo() to set different infolabels.
     * index          : [opt] integer - position to add playlist item. (default=end)
     * 
     * *Note, You can use the above as keywords for arguments and skip certain optional arguments.
     *        Once you use a keyword, all following arguments require the keyword.
     * 
     * example:
     *   - playlist = xbmc.PlayList(xbmc.PLAYLIST_VIDEO)
     *   - video = 'F:\\\\movies\\\\Ironman.mov'
     *   - listitem = xbmcgui.ListItem('Ironman', thumbnailImage='F:\\\\movies\\\\Ironman.tbn')
     *   - listitem.setInfo('video', {'Title': 'Ironman', 'Genre': 'Science Fiction'})
     *   - playlist.add(url=video, listitem=listitem, index=7)\n
     */
    void PlayList::add(const String& url, PlayListItem* listitem, int index)
    {
      CFileItemList items;

      if (listitem != NULL)
      {
        // an optional listitem was passed
        // set m_strPath to the passed url
        listitem->item->SetPath(url);

        items.Add(listitem->item);
      }
      else
      {
        CFileItemPtr item(new CFileItem(url, false));
        item->SetLabel(url);

        items.Add(item);
      }

      pPlayList->Insert(items, index);
    }

    /**
     * load(filename) -- Load a playlist.
     * 
     * clear current playlist and copy items from the file to this Playlist
     * filename can be like .pls or .m3u ...
     * returns False if unable to load playlist, True otherwise
     */
    bool PlayList::load(const char* cFileName) throw (PlayListException)
    {
      CFileItem item(cFileName);
      item.SetPath(cFileName);

      if (item.IsPlayList())
      {
        // load playlist and copy al items to existing playlist

        // load a playlist like .m3u, .pls
        // first get correct factory to load playlist
        std::auto_ptr<CPlayList> pPlayList (CPlayListFactory::Create(item));
        if ( NULL != pPlayList.get())
        {
          // load it
          if (!pPlayList->Load(item.GetPath()))
            //hmmm unable to load playlist?
            return false;

          // clear current playlist
          g_playlistPlayer.ClearPlaylist(iPlayList);

          // add each item of the playlist to the playlistplayer
          for (int i=0; i < (int)pPlayList->size(); ++i)
          {
            CFileItemPtr playListItem =(*pPlayList)[i];
            if (playListItem->GetLabel().IsEmpty())
              playListItem->SetLabel(URIUtils::GetFileName(playListItem->GetPath()));

            pPlayList->Add(playListItem);
          }
        }
      }
      else
        // filename is not a valid playlist
        throw PlayListException("Not a valid playlist");

      return true;
    }

    /**
     * remove(filename) -- remove an item with this filename from the playlist.
     */
    void PlayList::remove(const char* filename)
    {
      pPlayList->Remove(filename);
    }

    /**
     * clear() -- clear all items in the playlist.
     */
    void PlayList::clear()
    {
      pPlayList->Clear();
    }

    /**
     * size() -- returns the total number of PlayListItems in this playlist.
     */
    int PlayList::size()
    {
      return pPlayList->size();
    }

    /**
     * shuffle() -- shuffle the playlist.
     */
    void PlayList::shuffle()
    {
      pPlayList->Shuffle();
    }

    /**
     * unshuffle() -- unshuffle the playlist.
     */
    void PlayList::unshuffle()
    {
      pPlayList->UnShuffle();
    }

    /**
     * getposition() -- returns the position of the current song in this playlist.
     */
    int PlayList::getposition()
    {
      return g_playlistPlayer.GetCurrentSong();
    }

  }
}

