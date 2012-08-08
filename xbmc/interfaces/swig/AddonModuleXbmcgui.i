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

%module(directors="1") xbmcgui

%{
#include "native/Dialog.h"
#include "native/ModuleXbmcgui.h"
#include "native/Control.h"
#include "native/Window.h"
#include "native/WindowDialog.h"
#include "native/Dialog.h"
#include "native/WindowXML.h"

using namespace XBMCAddon;
using namespace xbmcgui;

#if defined(__GNUG__) && (__GNUC__>4) || (__GNUC__==4 && __GNUC_MINOR__>=2)
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

%}

%include "native/swighelper.h"

%include "native/ModuleXbmcgui.h"

%feature("ref") ListItem "${ths}->Acquire();"
%feature("unref") ListItem "${ths}->Release();"

%include "native/Exception.h"

%include "native/ListItem.h"
%include "native/Control.h"
%include "native/Dialog.h"

%feature("director") Window;
%feature("ref") Window "${ths}->Acquire();"
%feature("unref") Window "${ths}->Release();"
%feature("director") WindowDialog;
%feature("director") WindowXML;
%feature("director") WindowXMLDialog;

%include "native/Window.h"
%include "native/WindowDialog.h"
%include "native/Dialog.h"
%include "native/WindowXML.h"

