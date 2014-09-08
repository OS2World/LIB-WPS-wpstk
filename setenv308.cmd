/* 20130126 Andreas Buchinger (AB) */
/* 
    Script to set up environment settings for building the Workplace Shell Toolkit wpstk
*/

/* first check if 4OS2 is our command processor */
Address CMD '@set 4os2test_env=%@eval[2 + 2]';
if (value('4os2test_env',, 'OS2ENVIRONMENT') = '4') then
	'set 4os2test_env=1';
else 
	'set 4os2test_env=';

/* Path to you lokal xwp plugin directory */
'set XWP_PLUGINPATH=m:\xworkplace\plugins\xcenter'
'set DEBUG=1'
'set PMPRINTF=1'

/* Compiler vac3 must be set for vac3.08 and vac3.65 */
'set CFG_COMPILER=vac3'
'set CFG_WARPTK=V4'
'set CFG_TREE=DEV'
'set MT=1'

/* Additional setting is required when using vac3.65 */
'REM set CFG_COMPILER_36=yes'
'set CFG_COMPILER_36='
'set compiler365='

/* Clear watcom environment as otherwise rules.in falsely thinks we are using watcom */
'set watcom='

/* Used compiler environment must be set before calling wtkenv */
/* When not set in config.sys this is done with a script usually located in 
    your compilers root directory. Often called setenv.cmd or similar. On my 
    systems use a script called envicc365.cmd loacated in one of the directories
    in my PATH */
'call envicc.cmd'

/* */
'call p:\wpstk170\bin\wtkenv.cmd vac3'

