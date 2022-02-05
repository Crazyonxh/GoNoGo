
 /*
 * @Date: 2021-02-21 20:07:36
 * @LastEditTime: 2021-02-22 22:44:10
 * @LastEditors: Hao Xie, Lekang Yuan
 * @Organization: BBNC Lab@THU
 * @Describtions: Arduino Progam for the go-nogo system.
 */

//  实验时序设计相关的各个参数：
//unsigned long /=100;   //一个trial周期时
unsigned long soundDuration=1000;   //发声时长，单位毫秒下面相同
unsigned long timeWindowDelay=2000;   // 时间窗口相对声音开始的响应延时
unsigned long timeWindowDuration=2000;   //响应窗口总时长
unsigned long headTailDuration = 3000;
unsigned long waterDuration=2000;   //给水时长
unsigned long puffDuration=300;   //给airpuff时长
int flag=1;  //随机数决定是给哪种声音

// ---- 下面的程序一般不需要更改 ------- //
#include <ArduinoJson.h>
DynamicJsonDocument doc(400);
// 端口定义：舔水, 给水与声音
#define pinLick 8
#define pinWater 10
#define pinPuff 9
#define pinSound 11
#define freq1 1000
#define freq2 2000

// 各指示LED
#define lickLED 7
#define waterLED 6
#define puffLED 5
#define windowLED 4
// 程序中用到的临时变量

bool useHost = true;
bool noGo = true;
int state = 1; // 目前所处的状态，0为暂停，1为一个trial开始前，2为trial运行中
unsigned long leftTrials = 1000;

unsigned long soundTime; 
unsigned long windowTime; 
unsigned long trialEndTime; // 发声、时间窗口和trial结束时间，在trial开始前确定

unsigned long curTime; // 当前时间
unsigned long validLickTime = 0; // 有效舔水时间：定义为时间窗内第一次舔水的时间。给水参考这一时间进行。
unsigned long waterCommandTime = 0; // 上一次人工给水指令的时间。
unsigned long puffCommandTime = 0; // 上一次人工给水指令的时间。


bool lickState; 
int soundState;
bool windowState;
bool waterState; // 各状态变量
bool puffState; // 各状态变量

bool toneWrote=0;
bool noToneWrote=0;
bool waterOn=0;
bool puffOn=0;
int actorStateOutput=0;
int soundStateOutput=0;

void recvCmd();
String readSerial();
void prt(bool data, bool isFinal=false);
void prt(int data, bool isFinal = false);
void prt(unsigned long data, bool isFinal = false);
bool inWindow(unsigned long start, unsigned long cur, unsigned long duration);

void setup() {
   Serial.begin(9600);
   
   //输入输出管脚设置
   pinMode(pinLick,INPUT);
   pinMode(pinSound,OUTPUT);
   pinMode(pinWater, OUTPUT);
   pinMode(pinPuff, OUTPUT);
   pinMode(lickLED, OUTPUT);
   pinMode(waterLED, OUTPUT);
   pinMode(puffLED, OUTPUT);
   pinMode(windowLED, OUTPUT);
}

void loop() {
  recvCmd();
  if (state == 0) whenPause();
  if (state == 1) beforetrial();
  if (state == 2) intrial();
  if (state == 3) afterTrial();
}

void whenPause(){
    noTone(pinSound);
    digitalWrite(lickLED, LOW);
    digitalWrite(windowLED, LOW);
    digitalWrite(waterLED, LOW);
    digitalWrite(puffLED, LOW);
    digitalWrite(pinWater,LOW);
    digitalWrite(pinPuff,LOW);
    return;
  }

void beforetrial(){
  // 生成时间; 目前使用固定生成模式
  curTime = millis();
  unsigned long soundDelay = random(0,headTailDuration);
  soundTime = curTime + soundDelay;
  windowTime = soundTime + timeWindowDelay;
  trialEndTime = curTime + timeWindowDelay + timeWindowDuration + headTailDuration;
  // 重置各变量
  validLickTime = 0;
  toneWrote = 0;
  noToneWrote = 0;
  // 更改state到2
  state = 2;
  if(noGo) flag = random(0,2); // 1. flag生成似乎应该放在beforetrial里面？一个trial生成一次。
  else flag = 0; // 如果只想要Go，则把Flag设置成1即可

  if (useHost) {
      Serial.write("s");
      prt(curTime);
      prt(trialEndTime);
      prt(flag,true);
    }
  }

void intrial(){
  // 数据读入
  lickState = not digitalRead(pinLick);
  curTime = millis();
  // 状态机状态更新
  if (curTime > trialEndTime) {state = 3; return;} 
  
  // 状态更新
  windowState = inWindow(windowTime, curTime, timeWindowDuration);
  soundState = inWindow(soundTime, curTime, soundDuration);
  if (windowState and (validLickTime == 0) and lickState) validLickTime = curTime;

  waterState = (inWindow(validLickTime, curTime, waterDuration)and (flag==0)) or inWindow(waterCommandTime, curTime, waterDuration) or waterOn;
  puffState = (inWindow(validLickTime, curTime, puffDuration)and (flag==1)) or inWindow(puffCommandTime, curTime, puffDuration) or puffOn;
  // 输出各数据
  
  digitalWrite(lickLED, lickState);  // 这些LED如果没有连就别连了，看串口监视器更直观一点。不过不连LED这几行放着也没什么问题。
  digitalWrite(windowLED, windowState);
  digitalWrite(waterLED, waterState);
  digitalWrite(puffLED, puffState);

  digitalWrite(pinWater, waterState);
  digitalWrite(pinPuff, puffState);

  soundStateOutput = int(soundState) * (1 + int(flag)); // 2.加了这行判断 (不响的时候是0，flag = 0 的时候响为1， flag = 1的时候响为2)
  if (soundStateOutput ==1 ) {tone(pinSound, freq1);}
    if (soundStateOutput ==2 ) {tone(pinSound, freq2);}
  if (soundStateOutput ==0) {noTone(pinSound);}

  actorStateOutput = int(waterState) * 1 + int(puffState) * 2; // 3. 简化了表述. 全没有，water, puff, water + puff 应该分别为0，1，2，3

  if (useHost) {
      Serial.print("t");
     prt(lickState);
    prt(soundStateOutput);
    prt(actorStateOutput);
    prt(windowState);
    prt(curTime,true);
    }
   else{
    prt(lickState);
    prt(soundStateOutput);
    prt(actorStateOutput);
    prt(windowState,true);
    }
  }

void afterTrial(){
   leftTrials = leftTrials - 1;
   state = 1;
   int succ = int(validLickTime > 0) + 2 * int(flag);
   if (useHost) {
     Serial.print("e");
     prt(succ);
     prt(leftTrials, true); 
    } 
  }

void prt(bool data, bool isFinal = false){
    if (!isFinal){
        Serial.print(data); 
        Serial.print(",");
      }
     else{
        Serial.println(data);
      }
  }

void prt(int data, bool isFinal = false){
    if (!isFinal){
        Serial.print(data); 
        Serial.print(",");
      }
     else{
        Serial.println(data);
      }
  }

void prt(unsigned long data, bool isFinal = false){
    if (!isFinal){
        Serial.print(data); 
        Serial.print(",");
      }
     else{
        Serial.println(data);
      }
  }

bool inWindow(unsigned long start, unsigned long cur, unsigned long duration){
    unsigned long delta = cur - start;
    if (delta >= 0 and delta < duration) return 1;
    else return 0;
}



 String readSerial()   //读串口
{
  String val = "";    
  while (Serial.available())
  {
    val = Serial.readStringUntil('\r\n');
//    val += char(Serial.read());
  }
  return val;      
}

void updateParams(String s){
    deserializeJson(doc, s);
    noGo = bool(doc["noGo"]);
    headTailDuration = doc["headTailDuration"];
    soundDuration = doc["soundDuration"];
    timeWindowDelay = doc["timeWindowDelay"];
    timeWindowDuration = doc["timeWindowDuration"];
    waterDuration = doc["waterDuration"];
    puffDuration = doc["puffDuration"];
    leftTrials = doc["leftTrials"];
    Serial.print("mreceived. left trials: ");
    prt(leftTrials, true);
  }

void recvCmd(){
    // 读串口
    String cmd = readSerial();
    if (useHost and cmd.length() > 0) {Serial.print("m Command recv: "); Serial.print(cmd); Serial.println();}
    if(cmd == "") return;
    if(cmd[0] == 'p') state = 0;
    else if(cmd[0] == 's') state = 1;
    else if(cmd[0] == 'w') waterCommandTime = millis();
     else if(cmd[0] == 'f') puffCommandTime = millis();
    else if(cmd[0] == 'o') waterOn = !waterOn;
    else if(cmd[0] == 'a') puffOn = !puffOn;
    else if(cmd[0] == 'c') updateParams(cmd.substring(1));
  }
  
