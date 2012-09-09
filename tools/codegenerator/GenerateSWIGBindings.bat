SET cur_dir=%CD%

SET base_dir=%cur_dir%\..\..
SET groovy_dir=%base_dir%\lib\groovy
SET generator_dir=%base_dir%\tools\codegenerator

cd %1\..\python

SET python_dir=%CD%
SET python_generated_dir=%python_dir%\generated
SET swig_dir=%python_dir%\..\swig

IF NOT EXIST %python_generated_dir% md %python_generated_dir%
IF EXIST %python_generated_dir%\%2.xml del %python_generated_dir%\%2.xml
IF EXIST %python_generated_dir%\%2.cpp del %python_generated_dir%\%2.cpp

%cur_dir%\..\BuildDependencies\bin\swig\swig.exe -w401 -c++ -outdir %python_generated_dir% -o %python_generated_dir%\%2.xml -xml -I"%base_Dir%\xbmc" -xmllang python %swig_dir%\%2.i
java.exe -cp "%groovy_dir%\groovy-all-1.8.4.jar;%groovy_dir%\commons-lang-2.6.jar;%generator_dir%;%python_dir%" groovy.ui.GroovyMain %generator_dir%\Generator.groovy %python_generated_dir%\%2.xml %python_dir%\PythonSwig.cpp.template  %python_generated_dir%\%2.cpp

cd %cur_dir%