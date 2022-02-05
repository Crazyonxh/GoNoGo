#define pinLick 5
#define pinWater 7
#define lickLED 8
#define waterLED 9
#define waterDuration 100

unsigned long lastLickTime = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(pinLick, INPUT);
  pinMode(pinWater,OUTPUT);
  pinMode(lickLED, OUTPUT);
  pinMode(waterLED, OUTPUT);
  
  lastLickTime = millis() - 10000;
}

void loop() {
  // 检查舔水的状态，据此更新给水时间
  bool isLicking = ! digitalRead(pinLick);
  unsigned long curTime = millis();
  if (isLicking){
     lastLickTime = curTime;
    }

  Serial.print(isLicking); Serial.print(",");
  digitalWrite(lickLED, isLicking);
    
  // 如果当前时间在上次给水的时间内，则给水，否则不给
  unsigned long deltaTime = curTime - lastLickTime;
  bool giveWater = (deltaTime >= 0) and (deltaTime < waterDuration);
  digitalWrite(pinWater, giveWater); 
  digitalWrite(waterLED, giveWater);
  Serial.print(giveWater); Serial.print("\n");
}
