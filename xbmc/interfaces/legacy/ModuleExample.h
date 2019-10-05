/*
 * AddonExample.h
 *
 *  Created on: Oct 3, 2019
 *      Author: jim
 */

#include "utils/log.h"
#include "interfaces/legacy/AddonClass.h"

namespace Something {

   inline void test() {
     CLog::Log(LOGNOTICE, "Hello, I'm in a new module.");
   }

  class Example : public XBMCAddon::AddonClass {
    const char* m_message;
  public:
    Example(const char* p_message) : m_message(p_message) {}

    inline void logMe() {
      CLog::Log(LOGNOTICE, m_message);
    }
  };
}


