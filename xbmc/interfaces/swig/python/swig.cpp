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

#include "interfaces/swig/python/swig.h"

namespace PythonBindings
{
  void PyXBMCInitializeTypeObject(PyTypeObject* type_object)
  {
    static PyTypeObject py_type_object_header = { PyObject_HEAD_INIT(NULL) 0};
    int size = (long*)&(py_type_object_header.tp_name) - (long*)&py_type_object_header;
    memset(type_object, 0, sizeof(PyTypeObject));
    memcpy(type_object, &py_type_object_header, size);
  }

  int PyXBMCGetUnicodeString(std::string& buf, PyObject* pObject, bool coerceToString,
                             const char* argumentName, const char* methodname)
  {
    // TODO: UTF-8: Does python use UTF-16?
    //              Do we need to convert from the string charset to UTF-8
    //              for non-unicode data?
    if (PyUnicode_Check(pObject))
    {
      // Python unicode objects are UCS2 or UCS4 depending on compilation
      // options, wchar_t is 16-bit or 32-bit depending on platform.
      // Avoid the complexity by just letting python convert the string.
      PyObject *utf8_pyString = PyUnicode_AsUTF8String(pObject);

      if (utf8_pyString)
      {
        buf = PyString_AsString(utf8_pyString);
        Py_DECREF(utf8_pyString);
        return 1;
      }
    }
    if (PyString_Check(pObject))
    {
      buf = PyString_AsString(pObject);
      return 1;
    }

    // if we got here then we need to coerce the value to a string
    if (coerceToString)
    {
      PyObject* pyStrCast = PyObject_Str(pObject);
      if (pyStrCast)
      {
        int ret = PyXBMCGetUnicodeString(buf,pyStrCast,false,argumentName,methodname);
        Py_DECREF(pyStrCast);
        return ret;
      }
    }

    // Object is not a unicode or a normal string.
    buf = "";
    PyErr_Format(PyExc_TypeError, "argument \"%s\" for method \"%s\" must be unicode or str", argumentName, methodname);
    return 0;
  }
}

