
%module(directors="1") something

%{
#include "interfaces/legacy/ModuleExample.h"

using namespace Something;
%}

// Must be before the class is included
%feature("director") Example;

%include "interfaces/legacy/ModuleExample.h"

