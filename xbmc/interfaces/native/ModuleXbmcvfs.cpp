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

#include "ModuleXbmcvfs.h"
#include "LanguageHook.h"
#include "filesystem/File.h"
#include "filesystem/Directory.h"
#include "utils/FileUtils.h"
#include "Util.h"

namespace XBMCAddon
{

  namespace xbmcvfs
  {
    bool copy(const String& strSource, const String& strDestnation)
    {
      DelayedCallGuard dg;
      return XFILE::CFile::Cache(strSource, strDestnation);
    }

    // delete a file
    bool deleteFile(const String& strSource)
    {
      DelayedCallGuard dg;
      return XFILE::CFile::Delete(strSource);
    }

    // rename a file
    bool rename(const String& file, const String& newFile)
    {
      DelayedCallGuard dg;
      return XFILE::CFile::Rename(file,newFile);
    }  

    // check for a file or folder existance, mimics Pythons os.path.exists()
    bool exists(const String& path)
    {
      DelayedCallGuard dg;
      return XFILE::CFile::Exists(path, false);
    }      

    // make a directory
    bool mkdir(const String& path)
    {
      DelayedCallGuard dg;
      return XFILE::CDirectory::Create(path);
    }      

    // make all directories along the path
    bool mkdirs(const String& path)
    {
      DelayedCallGuard dg;
      return CUtil::CreateDirectoryEx(path);
    }

    bool rmdir(const String& path, bool force)
    {
      DelayedCallGuard dg;
      return CFileUtils::DeleteItem(path,force);
    }      

    // TODO: make this work.
    // lists content of a folder
//    PyObject* vfs_listdir(File *self, PyObject *args, PyObject *kwds)
//    {
//      PyObject *f_line;
//      if (!PyArg_ParseTuple(
//                            args,
//                            (char*)"O",
//                            &f_line))
//      {
//        return NULL;
//      }
//      CStdString strSource;
//      CFileItemList items;
//      if (!PyXBMCGetUnicodeString(strSource, f_line, 1))
//        return NULL;
//
//      CPyThreadState pyState;
//      CDirectory::GetDirectory(strSource, items);
//      pyState.Restore();
//
//      PyObject *fileList = PyList_New(0);
//      PyObject *folderList = PyList_New(0);
//      for (int i=0; i < items.Size(); i++)
//      {
//        CStdString itemPath = items[i]->GetPath();
//        PyObject *obj;
//        if (URIUtils::HasSlashAtEnd(itemPath)) // folder
//        {
//          URIUtils::RemoveSlashAtEnd(itemPath);
//          CStdString strFileName = URIUtils::GetFileName(itemPath);
//          obj = Py_BuildValue((char*)"s", strFileName.c_str());
//          PyList_Append(folderList, obj);
//        }
//        else // file
//        {
//          CStdString strFileName = URIUtils::GetFileName(itemPath);
//          obj = Py_BuildValue((char*)"s", strFileName.c_str());
//          PyList_Append(fileList, obj);
//        }
//        Py_DECREF(obj); //we have to do this as PyList_Append is known to leak
//      }
//      return Py_BuildValue((char*)"O,O", folderList, fileList);
//    }

  }
}
