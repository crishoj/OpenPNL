@echo off
call _clean_temp.bat
rmdir /s /q ..\..\bin 2>nul
rmdir /s /q ..\..\lib 2>nul
echo complete ..\..\temp\clean_all


