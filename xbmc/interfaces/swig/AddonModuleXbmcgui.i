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
#include "interfaces/native/Dialog.h"
#include "interfaces/native/ModuleXbmcgui.h"
#include "interfaces/native/Control.h"
#include "interfaces/native/Window.h"
#include "interfaces/native/WindowDialog.h"
#include "interfaces/native/Dialog.h"
#include "interfaces/native/WindowXML.h"

using namespace XBMCAddon;
using namespace xbmcgui;

#if defined(__GNUG__) && (__GNUC__>4) || (__GNUC__==4 && __GNUC_MINOR__>=2)
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

%}

%include "interfaces/native/swighelper.h"

%include "interfaces/native/ModuleXbmcgui.h"

%feature("ref") ListItem "${ths}->Acquire();"
%feature("unref") ListItem "${ths}->Release();"

%include "interfaces/native/Exception.h"

%include "interfaces/native/ListItem.h"
%include "interfaces/native/Control.h"
%include "interfaces/native/Dialog.h"

%feature("director") Window;
%feature("ref") Window "${ths}->Acquire();"
%feature("unref") Window "${ths}->Release();"
%feature("director") WindowDialog;
%feature("director") WindowXML;
%feature("director") WindowXMLDialog;

%include "interfaces/native/Window.h"
%include "interfaces/native/WindowDialog.h"
%include "interfaces/native/Dialog.h"
%include "interfaces/native/WindowXML.h"

