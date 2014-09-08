rem * Build WPSTK library with RPM GCC 4.x & GNU make

set LANG=en_US
set MAKESHELL=cmd.exe

rem * Optional override of the OS/2 Toolkit location (must be present if the Toolkit is
rem * not set up in CONFIG.SYS)
rem set CFG_WARPTK_PATH=D:/Tools/OS2TK45

make -f GNUMakefile lib
make -f GNUMakefile lib DEBUG=1
