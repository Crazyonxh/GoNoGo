# Arduino配置
1. 下载最新版本的Arduino IDE
2. 安装 ArduinoJson 包：
1. 搜索安装
2. 添加.zip库
Python配置
1. 下载安装Python3 exe安装包 (3.6-3.9应均可，在3.8.3下测试)，注意勾选添加Python到环境变量
2. 打开命令⾏，使⽤pip安装所需的包：
pip install PyQt5 numpy pyqtgraph serial pyserial -i https://pypi.tuna.tsinghua.edu.cn/simpl
e
Issue：Win7下某些Arduino⽆法识别
有的版本的Arduino使⽤的Usb驱动芯⽚是CH340，Win7下没有这个驱动
双击CH341SER.exe安装即可
DFbot精简版不带驱动，需要⾃⼰下载：
参考https://www.pianshen.com/article/6844885461/安装
https://forum.arduino.cc/index.php?topic=413812.0
https://learn.sparkfun.com/tutorials/how-to-install-ch340-drivers/all
https://www.pianshen.com/article/6844885461/
https://pan.baidu.com/s/182yxyABTae9fMMXMB_CJww 密码: 32a
