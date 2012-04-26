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
#pragma once

#include "String.h"
#include "AddonUtils.h"
#include "swighelper.h"

#include "XSyncUtils.h"

#include "utils/log.h"
#include "utils/StdString.h"

#define COPYVARARGS(fmt) va_list argList; va_start(argList, fmt); set(fmt, argList); va_end(argList)

#ifndef SWIG
namespace XBMCAddon
{
  /**
   * This class is the superclass for all exceptions thrown in the addon interface.
   * It provides a means for the bindings to retrieve error messages as needed.
   */
  class Exception
  {
    String classname;
    String message;

    void logThrowMessage();

  protected:
    inline Exception(bool discrim, const char* _classname) : classname(_classname) { TRACE; }

    /**
     * This method is called from the constructor of subclasses. It
     *  will set the message from varargs as well as call log message
     */
    void set(const char* fmt, va_list& argList);

    /**
     * This message can be called from the constructor of subclasses.
     * It will set the message and log the throwing.
     */
    void setMessage(const char* fmt, ...);

  public:
    inline Exception(const Exception& other) : classname(other.classname), 
                                               message(other.message) { TRACE; logThrowMessage(); }

    SWIGHIDDENVIRTUAL ~Exception();

    inline CStdString getMessage() const { return message; }
  };

  /**
   * UnimplementedException Can be used in places like the 
   *  Control hierarchy where the
   *  requirements of dynamic language usage force us to add 
   *  unimplmenented methods to a class hierarchy. See the 
   *  detailed explanation on the class Control for more.
   */
  class UnimplementedException : public Exception
  {
  public:
    inline UnimplementedException(const UnimplementedException& other) : Exception(other) { }
    inline UnimplementedException(const char* classname, const char* methodname) : 
      Exception(true,"UnimplementedException") 
    { setMessage("Unimplemented method: %s::%s(...)", classname, methodname); }
  };

  /**
   * This is what callback exceptions from the scripting language
   *  are translated to.
   */
  class UnhandledException : public Exception
  {
  public:
    inline UnhandledException(const UnhandledException& other) : Exception(other) { }
    inline UnhandledException(const char* _message,...) : Exception(true,"UnhandledException") { COPYVARARGS(_message); } 
  };
}
#endif

/**
 * These macros allow the easy declaration (and definition) of parent 
 *  class virtual methods that are not implemented until the child class.
 *  This is to support the idosyncracies of dynamically typed scripting 
 *  languages. See the comment in AddonControl.h for more details.
 */
#define THROW_UNIMP(classname) throw UnimplementedException(classname, __FUNCTION__)
#define DECL_UNIMP(classname) throw(UnimplementedException) { throw UnimplementedException(classname, __FUNCTION__); }
#define DECL_UNIMP2(classname,otherexception) throw(UnimplementedException,otherexception) { throw UnimplementedException(classname, __FUNCTION__); }

/**
 * The DECLARE_EXCEPTION macro will create a specific Exception
 *   type that can be used in the API and incorporates the appropriate
 *   SWIG handling for when that exception is thrown.
 *
 * Note: The DECLARE_EXCEPTION_TYPEMAP macro is not really public but is 
 *  an implementation detail of the DECLARE_EXCEPTION macro
 */
#define DECLARE_EXCEPTION(module,exception) \
    class exception##Exception : public XBMCAddon::Exception      \
    { \
    public: \
      inline exception##Exception(const exception##Exception& other) : Exception(other) { } \
      inline exception##Exception(const char* _message,...) : Exception(true,#exception "Exception") { COPYVARARGS(_message); } \
    }
