@echo off
set "path=%~dp1"
set "filename=%~nx1"
echo '%path%%filename%'
C:\Windows\System32\certutil.exe -hashfile "%path%%filename%" MD5
pause