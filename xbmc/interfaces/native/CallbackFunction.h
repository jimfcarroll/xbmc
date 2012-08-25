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

#pragma once

#include "AddonClass.h"

namespace XBMCAddon
{
  /**
   * This is the parent class for the class templates that hold
   *   a callback. A callback is essentially a templatized
   *   functor (functoid?) for a call to a member function.
   */
  class Callback : public AddonClass 
  { 
  protected:
    AddonClass* addonClassObject;
    Callback(AddonClass* _object, const char* classname) : AddonClass(classname), addonClassObject(_object) {}

  public:
    virtual void executeCallback() = 0;
    virtual ~Callback();

    AddonClass* getObject() { return addonClassObject; }
  };

  /**
   * This is the template to carry a callback to a member function
   *  that returns 'void' (has no return) and takes no parameters.
   */
  template<class M> class VoidCallbackFunction: public Callback
  { 
  public:
    typedef void (M::*MemberFunction)();
    MemberFunction meth;
    M* object;

    VoidCallbackFunction(M* _object, MemberFunction _meth) : 
      Callback(_object, "VoidCallbackFunction"), meth(_meth), object(_object) {}

    virtual ~VoidCallbackFunction() { deallocating(); }

    virtual void executeCallback()
    { 
      TRACE;
      ((*object).*(meth))(); 
    }
  };

  /**
   * This is the template to carry a callback to a member function
   *  that returns 'void' (has no return) and takes one parameter.
   */
  template<class M, typename P1> class VoidCallbackFunction1Param: public Callback
  { 
  public:
    typedef void (M::*MemberFunction)(P1);
    MemberFunction meth;
    M* object;
    P1 parameter;

    VoidCallbackFunction1Param(M* _object, MemberFunction _meth, P1 _parameter) : 
      Callback(_object, "VoidCallbackFunction1"), meth(_meth), object(_object),
      parameter(_parameter) {}

    virtual ~VoidCallbackFunction1Param() { deallocating(); }

    virtual void executeCallback()
    { 
      TRACE;
      ((*object).*(meth))(parameter); 
    }
  };

  /**
   * This is the template to carry a callback to a member function
   *  that returns 'void' (has no return) and takes one parameter.
   */
  template<class M, typename P1> class VoidCallbackFunction1Ref: public Callback
  { 
  public:
    typedef void (M::*MemberFunction)(P1*);
    MemberFunction meth;
    M* object;
    AddonClass::Ref<P1> parameter;

    VoidCallbackFunction1Ref(M* _object, MemberFunction _meth, P1* _parameter) : 
      Callback(_object, "VoidCallbackFunction1"), meth(_meth), object(_object),
      parameter(_parameter) {}

    virtual ~VoidCallbackFunction1Ref() { deallocating(); }

    virtual void executeCallback()
    { 
      TRACE;
      ((*object).*(meth))(parameter.get()); 
    }
  };

  // Currently all callbacks return void since the current system
  //  requires that they be asynchronous and called from the language
  //  thread. This is commented out until we work out the means of
  //  doing synchronous callbacks even with language requirements like
  //  those of Python
//  template<class M, typename R> class CallbackFunction: public Callback
//  { 
//  public:
//    typedef R (M::*MemberFunction)();
//
//    M* object;
//    MemberFunction meth;
//    
//    virtual R executeCallback()
//    {
//      return ((*object).*(meth))();
//    }
//  };
}


