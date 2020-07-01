@Echo off
cls

Title CmdMenuSel 1.4 - Demo - www.thebateam.org
Set "Path=%Path%;%cd%;%cd%\files"
Color 0a
Mode 80,25

:Main
Cls
Echo.
Echo.Select the Menu-options (you can use KEYBOARD or MOUSE)
Echo.
CmdMenuSel 0AA0 "Option 1" "Option 2" "Option 3" "Option 4" "Exit"
If /i "%Errorlevel%" == "5" (Exit)
Cls
Echo. You Selected: Option %Errorlevel%
Echo.
Echo.
timeout /t 2 >nul
Goto :Main
Pause
exit
