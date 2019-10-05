/*
 * AddonExample.h
 *
 *  Created on: Oct 3, 2019
 *      Author: jim
 */

#include "utils/log.h"
#include "interfaces/legacy/AddonClass.h"

namespace Something {

  class Example : public XBMCAddon::AddonCallback {
    const char* m_message;
  public:
    Example(const char* p_message) : m_message(p_message) {}

    inline virtual void funcToCall(const char* message) {
      CLog::Log(LOGNOTICE, "DEFAULT - %s", message);
    }

    inline void callFunc() {
      funcToCall(m_message);
    }
  };
}


