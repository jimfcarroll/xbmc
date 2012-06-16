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

%module(directors="1") xbmc

%{
#include "native/Player.h"
#include "native/Keyboard.h"
#include "native/ModuleXbmc.h"

using namespace XBMCAddon;
using namespace xbmc;

#if defined(__GNUG__) && (__GNUC__>4) || (__GNUC__==4 && __GNUC_MINOR__>=2)
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif
%}

%include "native/swighelper.h"

%include "native/ModuleXbmc.h"

%feature("director") Player;
%feature("ref") Player "${ths}->Acquire();"
%feature("unref") Player "${ths}->Release();"

%feature("python:method:play") Player
{
    PyObject *pObject = NULL;
    PyObject *pObjectListItem = NULL;
    char bWindowed = false;
    static const char *keywords[] = { "item", "listitem", "windowed", NULL };

    if (!PyArg_ParseTupleAndKeywords(
      args,
      kwds,
      (char*)"|OOb",
      (char**)keywords,
      &pObject,
      &pObjectListItem,
      &bWindowed))
    {
      return NULL;
    }

    Player* player = ((Player*)retrieveApiInstance((PyObject*)self,&PyXBMCAddon_xbmc_Player_Type,"play","XBMCAddon::xbmc::Player"));

    // set fullscreen or windowed
    bool windowed = (0 != bWindowed);

    if (pObject == NULL)
      player->playCurrent(windowed);
    else if ((PyString_Check(pObject) || PyUnicode_Check(pObject)) && pObjectListItem != NULL)
    {
      CStdString item;
      PyXBMCGetUnicodeString(item,pObject,"item","Player::play");
      player->playStream(item,(XBMCAddon::xbmcgui::ListItem *)retrieveApiInstance(pObjectListItem,"p.XBMCAddon::xbmcgui::ListItem","play"),windowed);
    }
    else // pObject must be a playlist
      player->playPlaylist((PlayList *)retrieveApiInstance(pObject,"p.PlayList","play"), windowed);

    Py_INCREF(Py_None);
    return Py_None;
  }



%include "native/Player.h"

%include "native/InfoTagMusic.h"
%include "native/InfoTagVideo.h"
%include "native/Keyboard.h"
%include "native/PlayList.h"

