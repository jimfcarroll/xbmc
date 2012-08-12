SET cur_dir=%CD%

cd %1

IF NOT EXIST python\generated md python\generated
IF EXIST python\generated\%2.xml del python\generated\%2.xml
IF EXIST python\generated\%2.cpp del python\generated\%2.cpp

%cur_dir%\..\BuildDependencies\bin\swig\swig.exe -w401 -c++ -outdir python\generated -o python\generated\%2.xml -xml -I"%cur_dir%\..\..\xbmc" -xmllang python %2.i
java.exe -cp "%cur_dir%\..\..\lib\groovy\groovy-all-1.8.4.jar;%cur_dir%\..\..\lib\groovy\commons-lang-2.6.jar;generator" groovy.ui.GroovyMain generator\Generator.groovy python\generated\%2.xml generator\bindings\PythonSwig.cpp.template python\generated\%2.cpp

cd %cur_dir%