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

#include "ModuleXbmcplugin.h"

#include "filesystem/PluginDirectory.h"
#include "FileItem.h"

namespace XBMCAddon
{

  namespace xbmcplugin
  {
    /**
     * addDirectoryItem(handle, url, listitem [,isFolder, totalItems]) -- Callback function to pass directory contents back to XBMC.
     *  - Returns a bool for successful completion.
     * 
     * handle      : integer - handle the plugin was started with.
     * url         : string - url of the entry. would be plugin:// for another virtual directory
     * listitem    : ListItem - item to add.
     * isFolder    : [opt] bool - True=folder / False=not a folder(default).
     * totalItems  : [opt] integer - total number of items that will be passed.(used for progressbar)
     * 
     * *Note, You can use the above as keywords for arguments and skip certain optional arguments.
     *        Once you use a keyword, all following arguments require the keyword.
     * 
     * example:
     *   - if not xbmcplugin.addDirectoryItem(int(sys.argv[1]), 'F:\\\\Trailers\\\\300.mov', listitem, totalItems=50): break
     */
    bool addDirectoryItem(int handle, const String& url, const xbmcgui::ListItem* listItem,
                          bool isFolder, int totalItems)
    {
      AddonClass::Ref<xbmcgui::ListItem> pListItem(listItem);
      pListItem->item->SetPath(url);
      pListItem->item->m_bIsFolder = isFolder;

      // call the directory class to add our item
      return XFILE::CPluginDirectory::AddItem(handle, pListItem->item.get(), totalItems);
    }

    /**
     * addDirectoryItems(handle, items [,totalItems]) -- Callback function to pass directory contents back to XBMC as a list.
     *  - Returns a bool for successful completion.
     * 
     * handle      : integer - handle the plugin was started with.
     * items       : List - list of (url, listitem[, isFolder]) as a tuple to add.
     * totalItems  : [opt] integer - total number of items that will be passed.(used for progressbar)
     * 
     *        Large lists benefit over using the standard addDirectoryItem()
     *        You may call this more than once to add items in chunks
     * 
     * example:
     *   - if not xbmcplugin.addDirectoryItems(int(sys.argv[1]), [(url, listitem, False,)]: raise
     */
    bool addDirectoryItems(int handle, 
                           const std::vector<Tuple<String,const XBMCAddon::xbmcgui::ListItem*,bool> >& items, 
                           int totalItems)
    {
      CFileItemList fitems;
      for (std::vector<Tuple<String,const XBMCAddon::xbmcgui::ListItem*,bool> >::const_iterator item = items.begin();
           item < items.end(); item++ )
      {
        const Tuple<String,const XBMCAddon::xbmcgui::ListItem*,bool>* pItem = &(*item);
        const String& url = pItem->first();
        const XBMCAddon::xbmcgui::ListItem *pListItem = pItem->second();
        bool bIsFolder = pItem->GetNumValuesSet() > 2 ? pItem->third() : false;
        pListItem->item->SetPath(url);
        pListItem->item->m_bIsFolder = bIsFolder;
        fitems.Add(pListItem->item);
      }

      // call the directory class to add our items
      return XFILE::CPluginDirectory::AddItems(handle, &fitems, totalItems);
    }

    /**
     * endOfDirectory(handle[, succeeded, updateListing, cacheToDisc]) -- Callback function to tell XBMC that the end of the directory listing in a virtualPythonFolder module is reached.
     * 
     * handle           : integer - handle the plugin was started with.
     * succeeded        : [opt] bool - True=script completed successfully(Default)/False=Script did not.
     * updateListing    : [opt] bool - True=this folder should update the current listing/False=Folder is a subfolder(Default).
     * cacheToDisc      : [opt] bool - True=Folder will cache if extended time(default)/False=this folder will never cache to disc.
     * 
     * example:
     *   - xbmcplugin.endOfDirectory(int(sys.argv[1]), cacheToDisc=False)
     */
    void endOfDirectory(int handle, bool succeeded, bool updateListing, 
                        bool cacheToDisc)
    {
      // tell the directory class that we're done
      XFILE::CPluginDirectory::EndOfDirectory(handle, succeeded, updateListing, cacheToDisc);
    }

    /**
     * setResolvedUrl(handle, succeeded, listitem) -- Callback function to tell XBMC that the file plugin has been resolved to a url
     * 
     * handle           : integer - handle the plugin was started with.
     * succeeded        : bool - True=script completed successfully/False=Script did not.
     * listitem         : ListItem - item the file plugin resolved to for playback.
     * 
     * example:
     *   - xbmcplugin.setResolvedUrl(int(sys.argv[1]), True, listitem)
     */
    void setResolvedUrl(int handle, bool succeeded, const xbmcgui::ListItem* listItem)
    {
      AddonClass::Ref<xbmcgui::ListItem> pListItem(listItem);
      XFILE::CPluginDirectory::SetResolvedUrl(handle, succeeded, pListItem->item.get());
    }

    /**
     * addSortMethod(handle, sortMethod, label2) -- Adds a sorting method for the media list.
     * 
     * handle      : integer - handle the plugin was started with.
     * sortMethod  : integer - number for sortmethod see FileItem.h.
     * label2Mask  : [opt] string - the label mask to use for the second label.  Defaults to '%D'
     *               applies to: SORT_METHOD_NONE, SORT_METHOD_UNSORTED, SORT_METHOD_VIDEO_TITLE,
     *                           SORT_METHOD_TRACKNUM, SORT_METHOD_FILE, SORT_METHOD_TITLE
     *                           SORT_METHOD_TITLE_IGNORE_THE, SORT_METHOD_LABEL
     *                           SORT_METHOD_LABEL_IGNORE_THE
     * 
     * example:
     *   - xbmcplugin.addSortMethod(int(sys.argv[1]), xbmcplugin.SORT_METHOD_TITLE)
     */
    void addSortMethod(int handle, int sortMethod, const String& clabel2Mask)
    {
      String label2Mask;
      label2Mask = (clabel2Mask.empty() ? "%D" : clabel2Mask.c_str());

      // call the directory class to add the sort method.
      if (sortMethod >= SORT_METHOD_NONE && sortMethod < SORT_METHOD_MAX)
        XFILE::CPluginDirectory::AddSortMethod(handle, (SORT_METHOD)sortMethod, label2Mask);
    }

    /**
     * getSetting(handle, id) -- Returns the value of a setting as a string.
     * 
     * handle    : integer - handle the plugin was started with.
     * id        : string - id of the setting that the module needs to access.
     * 
     * *Note, You can use the above as a keyword.
     * 
     * example:
     *   - apikey = xbmcplugin.getSetting(int(sys.argv[1]), 'apikey')
     */
    String getSetting(int handle, const char* id)
    {
      return XFILE::CPluginDirectory::GetSetting(handle, id);
    }

    /**
     * setSetting(handle, id, value) -- Sets a plugin setting for the current running plugin.
     * 
     * handle    : integer - handle the plugin was started with.
     * id        : string - id of the setting that the module needs to access.
     * value     : string or unicode - value of the setting.
     * 
     * example:
     *   - xbmcplugin.setSetting(int(sys.argv[1]), id='username', value='teamxbmc')
     */
    void setSetting(int handle, const String& id, const String& value)
    {
      XFILE::CPluginDirectory::SetSetting(handle, id, value);
    }

    /**
     * setContent(handle, content) -- Sets the plugins content.
     * 
     * handle      : integer - handle the plugin was started with.
     * content     : string - content type (eg. movies)
     * 
     *  *Note:  content: files, songs, artists, albums, movies, tvshows, episodes, musicvideos
     * 
     * example:
     *   - xbmcplugin.setContent(int(sys.argv[1]), 'movies')
     */
    void setContent(int handle, const char* content)
    {
      XFILE::CPluginDirectory::SetContent(handle, content);
    }

    /**
     * setPluginCategory(handle, category) -- Sets the plugins name for skins to display.
     * 
     * handle      : integer - handle the plugin was started with.
     * category    : string or unicode - plugins sub category.
     * 
     * example:
     *   - xbmcplugin.setPluginCategory(int(sys.argv[1]), 'Comedy')
     */
    void setPluginCategory(int handle, const String& category)
    {
      XFILE::CPluginDirectory::SetProperty(handle, "plugincategory", category);
    }

    /**
     * setPluginFanart(handle, image, color1, color2, color3) -- Sets the plugins fanart and color for skins to display.
     * 
     * handle      : integer - handle the plugin was started with.
     * image       : [opt] string - path to fanart image.
     * color1      : [opt] hexstring - color1. (e.g. '0xFFFFFFFF')
     * color2      : [opt] hexstring - color2. (e.g. '0xFFFF3300')
     * color3      : [opt] hexstring - color3. (e.g. '0xFF000000')
     * 
     * example:
     *   - xbmcplugin.setPluginFanart(int(sys.argv[1]), 'special://home/addons/plugins/video/Apple movie trailers II/fanart.png', color2='0xFFFF3300')\n
     */
    void setPluginFanart(int handle, const char* image, 
                         const char* color1,
                         const char* color2,
                         const char* color3)
    {
      if (image)
        XFILE::CPluginDirectory::SetProperty(handle, "fanart_image", image);
      if (color1)
        XFILE::CPluginDirectory::SetProperty(handle, "fanart_color1", color1);
      if (color2)
        XFILE::CPluginDirectory::SetProperty(handle, "fanart_color2", color2);
      if (color3)
        XFILE::CPluginDirectory::SetProperty(handle, "fanart_color3", color3);
    }

    /**
     * setProperty(handle, key, value) -- Sets a container property for this plugin.
     * 
     * handle      : integer - handle the plugin was started with.
     * key         : string - property name.
     * value       : string or unicode - value of property.
     * 
     * *Note, Key is NOT case sensitive.
     * 
     * example:
     *   - xbmcplugin.setProperty(int(sys.argv[1]), 'Emulator', 'M.A.M.E.')\n
     */
    void setProperty(int handle, const char* key, const String& value)
    {
      XFILE::CPluginDirectory::SetProperty(handle, key, value);
    }
    
  }
}
