@echo off
set YMD=%date:~,10%
set YMD=%YMD:/=%
set HMS=%time:~,6%
set HMS=%HMS::=%
make
mkdir build
copy main.out "build\main%YMD%%HMS%.exe"
del "..\server\main0.exe"
copy main.out "..\server\main0.exe"
make clean
cd ..\server\
start python server.py
start python server.py
pause