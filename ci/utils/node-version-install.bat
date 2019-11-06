@echo off

REM run from NodeJS folder
REM ..\\src\\node-rfc\\ci\\utils\\node-version-install.bat 10.17.0

if "%1" == "" goto :error
set INST_VERSION=%1

echo "Install: %INST_VERSION%"

curl -OJ https://nodejs.org/dist/v%INST_VERSION%/node-v%INST_VERSION%-win-x64.zip
curl -OJ https://nodejs.org/download/release/v%INST_VERSION%/node-v%INST_VERSION%-headers.tar.gz 

winrar x -ibck node-v%INST_VERSION%-win-x64.zip *.* 
tar -xzf node-v%INST_VERSION%-headers.tar.gz
move node-v%INST_VERSION%/include node-v%INST_VERSION%-win-x64/.
rmdir node-v%INST_VERSION

goto :done

:error
echo "nodejs version parameter missing, run like"
echo ..\\src\\node-rfc\\ci\\utils\\node-version-install.bat 10.17.0

:done