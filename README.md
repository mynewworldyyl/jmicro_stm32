# jmicro_stm32
jm_client移植到STM32

代码构建
分别下载以下两个Git代码
https://github.com/mynewworldyyl/jm_client.git
https://github.com/mynewworldyyl/jmicro_stm32.git

打开命令行窗口，进入到jmicro_stm32目录，运行如下命令
mklink /J D:\opensource\schip51\aithinkerSDK\smart_config\app\jm_client jm_client

使用Keil打开jmicro_stm32项目即可构建并下载到STM32开发板运行测试

test目录下为测试样例，注意每次测试打开一个测试样例
