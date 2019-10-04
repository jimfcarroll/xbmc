
%module something

%{
#include "interfaces/legacy/ModuleExample.h"

using namespace Something;
%}

%include "interfaces/legacy/ModuleExample.h"

