//  本程序只有go的功能
//  实验时序设计相关的各个参数：
//unsigned long /=100;   //一个trial周期时长
unsigned long soundDelay =500; // trial开始到给声音的时间
unsigned long soundDuration=1000;   //发声时长，单位毫秒下面相同
unsigned long timeWindowDelay=2000;   // 时间窗口相对声音开始的响应延时
unsigned long timeWindowDuration=2000;   //响应窗口总时长
unsigned long afterDelay=4000; // 
unsigned long waterDuration=300;   //给水时长

// ---- 下面的程序一般不需要更改 ------- //

// 端口定义：舔水, 给水与声音
#define pinLick 5
#define pinWater 7
#define pinSound 4
#define freq 2000
// 各指示LED
#define lickLED 8
#define waterLED 9
#define windowLED 11
// 程序中用到的临时变量

int state = 1; // 目前所处的状态，0为暂停，1为一个trial开始前，2为trial运行中

unsigned long soundTime; 
unsigned long windowTime; 
unsigned long trialEndTime; // 发声、时间窗口和trial结束时间，在trial开始前确定

unsigned long curTime; // 当前时间
unsigned long validLickTime = 0; // 有效舔水时间：定义为时间窗内第一次舔水的时间。给水参考这一时间进行。
unsigned long waterCommandTime = 0; // 上一次人工给水指令的时间。

bool lickState; 
bool soundState;
bool windowState;
bool waterState; // 各状态变量
bool toneWrote=0;
bool noToneWrote=0;
bool waterOn=0;

void setup() {
   Serial.begin(9600);
   
   //输入输出管脚设置
   pinMode(pinLick,INPUT);
   pinMode(pinSound,OUTPUT);
   pinMode(pinWater, OUTPUT);
   pinMode(lickLED, OUTPUT);
   pinMode(waterLED, OUTPUT);
   pinMode(windowLED, OUTPUT);
}

void loop() {
  recvCmd();
  if (state == 0) whenPause();
  if (state == 1) beforetrial();
  if (state == 2) intrial();
}


 String readSerial()   //读串口
{
  String val = "";    
  while (Serial.available())
  {
    val += char(Serial.read());
  }
  return val;      
}

void recvCmd(){
    // 读串口
    String cmd = readSerial();
    if(cmd == "") return;
    if(cmd == "p") state = 0;
    else if(cmd == "s") state = 1;
    else if(cmd == "w") waterCommandTime = millis();
    else if(cmd == "o") waterOn = !waterOn;
  }

  
void whenPause(){
    noTone(pinSound);
    digitalWrite(lickLED, LOW);
    digitalWrite(windowLED, LOW);
    digitalWrite(waterLED, LOW);
    digitalWrite(pinWater,LOW);
    return;
  }



void beforetrial(){
  // 生成时间; 目前使用固定生成模式
  curTime = millis();
  soundTime = curTime + soundDelay;
  windowTime = soundTime + timeWindowDelay;
  trialEndTime = windowTime + timeWindowDuration + afterDelay;
  // 重置各变量
  validLickTime = 0;
  toneWrote = 0;
  noToneWrote = 0;
  // 更改state到2
  state = 2;
  }

void prt(bool data, bool isFinal=false);
bool inWindow(unsigned long start, unsigned long cur, unsigned long duration);

void intrial(){
  // 数据读入
  lickState = not digitalRead(pinLick);
  curTime = millis();
  // 状态机状态更新
  if (curTime > trialEndTime) {state = 1; return;} 
  // 状态更新
  windowState = inWindow(windowTime, curTime, timeWindowDuration);
  soundState = inWindow(soundTime, curTime, soundDuration);
  if (windowState and (validLickTime == 0) and lickState) validLickTime = curTime;
  waterState = inWindow(validLickTime, curTime, waterDuration) or inWindow(waterCommandTime, curTime, waterDuration) or waterOn;
  // 输出各数据
  digitalWrite(lickLED, lickState);
  digitalWrite(windowLED, windowState);
  digitalWrite(waterLED, waterState);
  
  digitalWrite(pinWater, waterState);
  if (soundState ) {tone(pinSound, freq);}
  if (!soundState) {noTone(pinSound);}

  prt(lickState);
  prt(soundState);
  prt(waterState);
  prt(windowState,true);
  
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

bool inWindow(unsigned long start, unsigned long cur, unsigned long duration){
    unsigned long delta = cur - start;
    if (delta >= 0 and delta < duration) return 1;
    else return 0;
}

  
