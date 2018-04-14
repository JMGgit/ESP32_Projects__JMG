xcopy /y build z:\
cd /d z:
echo FILE=/esp32/ESP32.bin > SW_Info.txt
rem add missing "0" to HOUR if necessary
SET HOUR=%time:~0,2%
if "%HOUR:~0,1%" == " " (
echo VERSION=%DATE:~6,4%%DATE:~3,2%%DATE:~0,2%0%TIME:~1,1%%TIME:~3,2%%TIME:~6,2% >> SW_Info.txt
) ELSE (
echo VERSION=%DATE:~6,4%%DATE:~3,2%%DATE:~0,2%%TIME:~0,2%%TIME:~3,2%%TIME:~6,2% >> SW_Info.txt
)