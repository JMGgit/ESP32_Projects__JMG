xcopy /y build z:\
cd /d z:
echo FILE=/esp32/ESP32.bin > ota_cfg.txt
echo VERSION=%DATE:~6,4%%DATE:~3,2%%DATE:~0,2%%TIME:~0,2%%TIME:~3,2%%TIME:~6,2% >> ota_cfg.txt