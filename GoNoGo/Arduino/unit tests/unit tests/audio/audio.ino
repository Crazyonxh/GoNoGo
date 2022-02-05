#define pinSound 4
#define lowFreq 1000
#define highFreq 2000
#define delayTime 1000
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(pinSound,OUTPUT);//声音管脚为输出
}

void loop() {
  // put your main code here, to run repeatedly:
  tone(pinSound,lowFreq);
  delay(delayTime);
  tone(pinSound,highFreq);
  delay(delayTime);
}
