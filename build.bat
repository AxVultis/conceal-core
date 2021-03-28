@echo off

SET build_type=%~1
SET num_thread=%~2
SET result_folder=results\"%build_type%"-"%num_thread%"
mkdir -p "%result_folder%"
mkdir -p build
DEL /S build\*
cd build
cmake  .. -G "Visual Studio 16 2019" -DCMAKE_BUILD_TYPE="%build_type%" -DCMAKE_EXPORT_COMPILE_COMMANDS=True
msbuild concealX.sln /p:Configuration=Release /m:"%num_thread%"
cd ..
COPY  build\compile_commands.json "%result_folder%"
COPY  build\src\conceal* "%result_folder%"
COPY  build\src\walletd "%result_folder%"