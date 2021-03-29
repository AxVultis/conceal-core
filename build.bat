@echo off

SET build_type=%~1
SET num_thread=%~2
SET result_folder=results\"%build_type%"-"%num_thread%"
mkdir "%result_folder%"
mkdir build
DEL /S /q build\*
cd build
cmake  .. -G "Visual Studio 16 2019" -DCMAKE_BUILD_TYPE="%build_type%"
msbuild concealX.sln /p:Configuration=Release /m:"%num_thread%"
cd ..
COPY  build\src\"%build_type%"\conceal*.exe "%result_folder%"
COPY  build\src\"%build_type%"\walletd.exe "%result_folder%"