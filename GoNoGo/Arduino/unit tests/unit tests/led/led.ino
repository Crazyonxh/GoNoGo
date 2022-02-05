#define pinLED 11
#define delayTime 1000
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(pinLED,OUTPUT);//声音管脚为输出
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(pinLED, HIGH);
  delay(delayTime);
  digitalWrite(pinLED, LOW);
  delay(delayTime);
}
