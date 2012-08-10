@ECHO OFF

rem paths and directories
SET base_dir=%CD%\..\..\..
SET bin_dir=%base_dir%\project\BuildDependencies\bin
SET out_dir=generated
SET groovy_dir=%base_dir%\lib\groovy
SET swig_dir=%base_dir%\xbmc\interfaces\swig

rem output paths
SET python_out_path=%swig_dir%\python\%out_dir%

rem can't run rmdir and md right after each other without an "access denied" error
IF EXIST %python_out_path% rmdir /S /Q %python_out_path%

rem executables
SET java_exe=java.exe
SET swig_exe=%bin_dir%\swig\swig.exe

rem other variables
SET swig_inc=-I"%base_dir%\xbmc"
SET java_cp="%groovy_dir%\groovy-all-1.8.4.jar;%groovy_dir%\commons-lang-2.6.jar;%swig_dir%\generator"

rem PYTHON

SET python_files=(AddonModuleXbmc AddonModuleXbmcgui AddonModuleXbmcplugin AddonModuleXbmcaddon AddonModuleXbmcvfs)

md %python_out_path%

FOR %%i IN %python_files% DO (
  %swig_exe% -w401 -c++ -outdir %python_out_path% -o %python_out_path%\%%i.xml -xml %swig_inc% -xmllang python %swig_dir%\%%i.i
  %java_exe% -cp %java_cp% groovy.ui.GroovyMain %swig_dir%\generator\Generator.groovy %python_out_path%\%%i.xml %swig_dir%\generator\bindings\PythonSwig.cpp.template %python_out_path%\%%i.cpp
)