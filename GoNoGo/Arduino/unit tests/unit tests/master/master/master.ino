//舔水输入端口
#define pinLick 9
//声音输出
#define pinSound 8
#define lowFreq 1000
#define highFreq 2000
// 舔水与时间窗口LED
#define pinLickLED 6
// 给水与惩罚
#define pinWater 2
#define pinPunish 3
//实验总数
#define trial 100
#define pinServo 0

unsigned long soundDuration=500;   //发声时长，单位毫秒下面相同
unsigned long trialDuration=10000;   //一个trial周期时长
unsigned long lickDuration=2000;   //给水时长
unsigned long punishDuration=2000;   //惩罚时长
unsigned long timeWindowDuration=2000;   //响应窗口总时长
unsigned long timeWindowDelay=10;   //时间窗口相对声音开始的响应延时
unsigned long period=1;  //循环间隔，单位毫秒

//这些是程序中用到的临时变量
bool soundState=0;  //是否发声
bool timeWindowState=0;  //是否在响应时间窗
bool waterState=0;   //是否给水
bool punishState=0;  //是否惩罚

unsigned long soundTime[trial]; //发声时间      ¸
int soundType[trial]; //发声种类
unsigned long timeLastLick=0;    //上次舔水时刻
unsigned long timeLastLickPunish=0;  //上次惩罚时刻
unsigned long previousMillis=0;   //上次循环时刻

int currentTrial=1;   //trail的编号
int ii=0; // 循环变量
int state=0;

//定义两个主循环的操作函数
void checkState(unsigned long  currentMillis); // 根据输入改变状态
void changeState(unsigned long  currentMillis); //根据状态输出信号

void setup() {
  // put your setup code here, to run once:
   Serial.begin(9600);
   pinMode(pinLick,INPUT);//舔水管脚为输入 
   pinMode(pinSound,OUTPUT);//声音管脚为输出  
   pinMode(pinWater, OUTPUT);//给水管脚为输出  
   pinMode(pinPunish, OUTPUT);//惩罚管脚为输出
   pinMode(pinLickLED, OUTPUT);//舔提示管脚为输出
   
   
   digitalWrite(pinSound,LOW); //初始输出的信号都给低电平
   digitalWrite(pinWater,LOW); 
   digitalWrite(pinPunish,LOW); 
   digitalWrite(pinServo,LOW);
   digitalWrite(pinLickLED,LOW); 
   //发声的时间和类型都初始化并输出
  for (ii=1;ii<trial;ii++){
    soundTime[ii]=trialDuration*(ii-1)+random(0,(trialDuration-timeWindowDelay-timeWindowDuration));   //在连个trial之间随机给时间
    soundType[ii]=(random(0,2)); //两种类型的声音
//    Serial.println(soundTime[ii], DEC);       
//    Serial.println(soundType[ii], DEC); 
  }
//   Serial.println(' '); 
}

void loop() {
  unsigned long currentMillis = millis(); // 当前时刻
  if (currentMillis - previousMillis >= period)    // 如果已经过了循环间隔
  {
   previousMillis = currentMillis;   // 把当前时刻记录到上一时刻
   checkState(currentMillis);       // 根据输入改变状态
   changeState(currentMillis);     //根据状态输出信号
  }
}


void  checkState(unsigned long  currentMillis){
  // 显示Licking状态
  bool isLicking = !digitalRead(pinLick);
  Serial.print(isLicking);Serial.print(",");
  if (isLicking){
    digitalWrite(pinLickLED, HIGH);
    }
  else{
    digitalWrite(pinLickLED, LOW);
    }
  //如果之前没有声音，时间到了发声
  if ((soundState==0)&&(currentMillis>soundTime[currentTrial])&&(currentMillis<soundTime[currentTrial]+soundDuration)){
    soundState=1;
  }
    //如果之前有声音，时间到了停止发声
  if ((soundState!=0)&&(currentMillis>soundTime[currentTrial]+soundDuration)){
    soundState=0;
  }
   //如果有声音，延迟一会产生时间窗口
  if ((soundState==1)&&(timeWindowState==0)&&(currentMillis>soundTime[currentTrial]+timeWindowDelay)&&(currentMillis<soundTime[currentTrial]+timeWindowDelay+timeWindowDuration)){
    timeWindowState=1;
  }
  //如果有时间窗口开着，时间窗口过一阵子关闭
  if ((timeWindowState==1)&&(currentMillis>soundTime[currentTrial]+timeWindowDelay+timeWindowDuration)){
    timeWindowState=0;
  }
 //如果在给水，时间窗口过一阵子关水
  if ((waterState==1)&&(currentMillis>timeLastLick+lickDuration)){
    waterState=0;
  }
   //如果在惩罚，过一阵停止
  if ((punishState==1)&&(currentMillis>timeLastLickPunish+punishDuration)){
    punishState=0;
  }
   //如果在时间窗口内，舔对了就给水
  if ((waterState==0)&&( timeWindowState==1)&&(soundType[currentTrial]==0)&&isLicking){
    waterState=1;
    timeLastLick=currentMillis;
  }
     //如果在时间窗口内，舔错了就惩罚
   if ((punishState==0)&&( timeWindowState==1)&&(soundType[currentTrial]==1)&&isLicking){
    punishState=1;
    timeLastLickPunish=currentMillis;
  }
   //如果过了一个trial的时间,trials数目加一
  if (currentMillis>currentTrial*trialDuration){
    currentTrial+=1;
    delay(10);
  }
 }


void changeState(unsigned long  currentMillis){
  Serial.print(waterState); Serial.print(",");
  Serial.print(punishState);Serial.print(",");
  Serial.println(timeWindowState);  
  if(waterState==1){
    digitalWrite(pinWater, HIGH);  //输出舔水信号
  }
  if(waterState==0) digitalWrite(pinWater, LOW);  //关闭舔水信号
  if(punishState==1) digitalWrite(pinPunish, HIGH);  //输出惩罚信号
  if(punishState==0) digitalWrite(pinPunish, LOW);//关闭惩罚信号

   //输出两种声音信号
  if((soundState==1)&&(soundType[currentTrial]==0)) {
     tone(pinSound,lowFreq);
  }
  if((soundState==1)&&(soundType[currentTrial]==1)) {
     tone(pinSound,highFreq);
  }
  //关闭声音信号
  if(soundState==0) {
     noTone(pinSound);
  }
}
