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

# 使用说明
⼀. 怎么⽤
1. 硬件单元测试
烧录 Arduino/Unit Test 中的⼏个⽂件，测试各硬件单元是否运转正常:
Audio.ino: 烧录后扬声器应⼀⾼⾳⼀低⾳正常发声。旋转蓝⾊旋钮调节分⻉。
Lick.ino: 运⾏后打开串⼝绘图器，⽤⼿摸舔⽔输⼊的线，看波形是否正常
Water.ino: 万⽤表测试光电耦合模块对应⼝，5s 24V 5s 0V。如已连接电磁阀可直接看出⽔/出⽓情
况。
如果都没问题电路基本ok了。
2. 使⽤Arduino IDE测试Go-Nogo程序
打开 Arduino/main ，修改 useHost = false （true为使⽤Python上位机监视，false为使⽤IDE⾃带
串⼝绘图器）， noGo = true （true为Go-nogo模式，false只有go）。烧录，打开串⼝绘图器查看：
说明：
该程序在Arduino上电后就开始运⾏，程序中参数为上电后默认使⽤的参数。
图中红⻩蓝绿分别为 soundState, TimeWindow, lickState, actorState . 其中：
soundState ：Nogo时响为2，Go时响为1，不响为0
actorState ：water（出⽔）为1，puff（出⽓）为2，water+puff为3，什么都没有为0
可以通过串⼝向Arduino发送控制指令（中图框内），指令包括：
3. Python上位机
使⽤⽅法：
1. 打开 Arduino/main ，修改 useHost = true
2. 双击 run.bat (该程序就是在该⽂件夹中 python main.py )运⾏。
功能说明：
1. 建⽴连接：
1. 端⼝选择
2. 发起连接键。选好端⼝后建⽴连接，开始运⾏程序
2. 监视与储存：
1. 动态显示波形
2. 过去30个Trials的情况，其中0-3分别表示Miss（go没舔），Hit（go舔），CR (Correct
rejection, nogo没舔)，FA（False alarm, nogo舔）
3. 过去Trial各情况次数统计。格式：[Miss, Hit, CR, FA] / Total
4. 开始记录键。上位机刚开始运⾏时不记录数据，准备好后点击REC开始记录。
5. trial成败情况储存。⾃动存在 data ⽂件夹内，以程序开始时间命名。【不完善，请根据实验
需求提建议（例如希望存什么数据，先后训多只⿏的话希望怎么存）】
3. 参数更新：
1. 选择参数。参数⽂件储存在 config ⽂件夹中，选择对应的⽂件名。
2. 参数发送键。发送选中⽂件中的参数到Arduino.
4. 直接控制：发送控制指令到Arduino. 通信格式同上⼀节。
⼆. 如果出了问题...
Arduino IDE
找不到端⼝：⼀般菜单栏找到端⼝换⼀下即可。
端⼝占⽤：Arduino端⼝只可以同时连接⼀个程序（上传，IDE的监视器或绘图器，Python上位
机），断开其他连接。
Hardware
检查连线 & 电源模块是否接上。
舔⽔模块如果⼀直为true：
检查是否有电容⼲扰
断开电源重新上电（舔⽔模块的原理为电容检测，⽆接触时的基准电容值在上电时确定，重启
以修复）
阀⼯作不正常：
检查是否为⽓阀故障：使⽤万⽤电源，或发送 w/p 指令保持⽔⽓常开，或从5-24模块24V端⼝
引出24V连到⽓阀上，看给电后⽓阀是否运⾏正常。
检查液体流动路径：测试不连阀时是否正常流动。
其他注意事项：
可变电源电压值应为为24V
⽓阀为三通阀，需要把不⽤的那个孔堵住，否则会漏⽓漏⽔
请不要往⾥⾯倒含有颗粒的液体，否则可能堵住（有次奶没有过滤倒进去弄死了⼀只电
磁阀）
上位机
注意打开程序后点 CON 后才开始连接，点 REC 后才开始记录数据。如果使⽤烧录进去的默认参数，
不需要点击 PARA （如果⼀段时间⽤相同参数做单个实验，建议修改 Arduino/main/main.ino 中
的参数，运⾏多个实验需要切换参数的话可以把各个实验的参数储存在不同⽂件中上传）
# 搭建说明
1. 元件集合
Arduino主体和扩展板
1. Arduino Uno【1】
2. Arduino扩展板【2】。有的话连线会⽅便⼀些，不必须
传感和执⾏单元
3. 扬声器【3】
4. 电磁阀【4】。⽤于控制给⽔。
5. 5V~24V光耦转换模块【5】。电磁阀需要24V电压驱动，将5V信号输⼊转化为24V输出。
6. 舔⽔模块。【6，7】芯⽚和两个电容。
电源供应
7. Usb升降压模块【8】。提供0~24V电压，需要两个，分别给到电磁阀（需要24V）和舔⽔模块
（5V, 该模块⽤Arduino直接驱动时容易受Arduino电源波动⼲扰导致输出结果错误，故需要单独⼀
路电源）
8. Usb Hub。接两个升降压模块。插到充电宝啥的上⾯也⾏。
其他：10nF电容，排针，杜邦线，⾯包板。
【】内标号对应最后的购买链接。
2. 电路搭建说明
2.1 舔⽔检测电路搭建
⽅法1：电路板焊接
⽤那家的电路板焊。其实没什么必要。
⼀个芯⽚，两个电容：芯⽚上的点位于右下⻆，电容不分正负
排针
两根线
右侧⻓线：将芯⽚输出⼝直接连接对应引脚
左侧短线：焊完排针之后可以把6⼝GND和闲置的7⼝焊到⼀起，多出⼀个地。两个地分别连
电源和Arduino
⽅法2：转换后按图连接
将⼩芯⽚贴焊到转换板上，⾃⼰搭出来即可。电路连接：
1, 3, 4, 5 连VCC，8 连GND
两个10nF电容：6和7之间，7和8（GND)之间
7为输⼊，2为输出。
QT113H DataSheet: https://media.digikey.com/pdf/Data%20Sheets/Quantum%20PDFs/QT11
3(H).pdf
2.2 整体连接
各模块备⻬后，只需要连⼏组线即可：
两路电源：将两个升降压模块分别调到5V和24V, 连接到舔⽔模块和光电耦合模块对应的VCC和
GND
三路信号：分别连接扬声器、舔⽔模块、光电耦合模块到Arduino. （GND & Signal，扬声器⽤
Arduino供电多连个VCC)
两路输出：
将舔⽔的针头连接到舔⽔模块输⼊上
将光电耦合的24V输出连接到电磁阀。电磁阀为⼀输⼊两输出三通阀，需要把另外那个输出⼝
堵上。
搭建完成的系统如下
