@echo off
for /r %%i in (*.plg *.vcc *.opt *.log) do del /q %%i
del /s /q ..\..\temp
rmdir /s /q ..\..\temp
echo complete ..\..\temp\clean_temp