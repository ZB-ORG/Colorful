::该批处理用于演示如何调用case/fwpkg目录下的makefile完成代码编译，固件生成操作
::请根据使用习惯，或使用命令行中直接调用makefile谓命令的方式，或调用该批处理实现特定功能
::批处理速度 build_only.bat > target_clean_build.bat > clean_build.bat

::make clean用于只清理exe文件，也就是编译修改过的文件，然后链接
make clean_target

::以下几个配置项只能从中选择一个，不能同时生成

::make DVB_ATS2825用于生成2825普通固件
make DVB_ATS2825

::make DVB_ATS2825_OTA用于生成2825含OTA功能的固件
::make DVB_ATS2825_OTA

::make DVB_ATS2825_TWS用于生成2825含TWS功能的固件
::make DVB_ATS2825_TWS

::make DVB_ATS2823_TWS用于生成2823的固件
:make DVB_ATS2823