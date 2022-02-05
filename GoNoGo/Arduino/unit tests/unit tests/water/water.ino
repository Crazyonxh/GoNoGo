#define pinWater 7
#define pinLED 9
#define delayTime 2000
#define waterDuration 100

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(pinWater,OUTPUT);//声音管脚为输出
  pinMode(pinLED, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(pinWater, HIGH); 
  digitalWrite(pinLED, HIGH);
  delay(waterDuration);
  digitalWrite(pinWater, LOW);
  digitalWrite(pinLED, LOW);
  delay(delayTime);
}
