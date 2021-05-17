deltree /y c:\tmp\rbench
mkdir c:\tmp\rbench
mkdir c:\tmp\rbench\src
mkdir c:\tmp\rbench\src\dos
mkdir c:\tmp\rbench\tools

copy Makefile.dos c:\tmp\rbench
copy tools\lutgen.c c:\tmp\rbench\tools
copy src\*.c c:\tmp\rbench\src
copy src\*.h c:\tmp\rbench\src
copy src\dos\*.c c:\tmp\rbench\src\dos
copy src\dos\*.h c:\tmp\rbench\src\dos
copy pack.bat c:\tmp\rbench
