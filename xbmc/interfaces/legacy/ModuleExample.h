/*
 * AddonExample.h
 *
 *  Created on: Oct 3, 2019
 *      Author: jim
 */

#include "utils/log.h"

namespace Something {

   inline void test() {
     CLog::Log(LOGNOTICE, "Hello, I'm in a new module.");
   }

}


