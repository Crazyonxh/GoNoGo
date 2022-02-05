#define pinLick 3

void setup() { 
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(pinLick,INPUT);//舔水管脚为输入
}

void loop() {
  // put your main code here, to run repeatedly:
  bool isLicking = digitalRead(pinLick);
  Serial.println(!isLicking);
}
