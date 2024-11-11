#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(4, 5); // RX, TX  分别接到GPIO4和GPIO5

String receivedString = ""; // 用于存储接收的字符串

void setup() {
  mySerial.begin(9600);       // 软件串口初始化
  Serial.begin(9600);         // 调试串口初始化
  pinMode(2, OUTPUT);         // 设置GPIO2为输出模式
  digitalWrite(2, HIGH);      // 初始状态为高电平
}

void loop() {
  // 如果软件串口有数据可读
  while (mySerial.available() > 0) {
    char receivedChar = mySerial.read(); // 逐字节读取数据
    Serial.print(receivedChar);          // 在调试串口输出收到的字节
    receivedString += receivedChar;      // 将字节拼接成字符串

    // 如果收到换行符（表示一条消息结束）
    if (receivedChar == '\n') {
      receivedString.trim();             // 去除首尾空白字符（包括换行符）

      // 判断是否收到 "wake"
      if (receivedString == "wake") {
        digitalWrite(2, LOW);
        delay(20);
        digitalWrite(2, HIGH);           
        Serial.println("response wake");
      } 
      else if (receivedString == "on"){
        digitalWrite(2, LOW);         
        Serial.println("response turn on");
      }
      else if (receivedString == "off"){
        digitalWrite(2, HIGH);        
        Serial.println("response turn off");
      }
      else if (receivedString == "1"){
        digitalWrite(2, LOW);
        delay(20);
        digitalWrite(2, HIGH);               
        Serial.println("response red");
      }
      else if (receivedString == "2"){
        digitalWrite(2, LOW);
        delay(20);
        digitalWrite(2, HIGH);                 
        Serial.println("response white");
      }
      else if (receivedString == "3"){
        digitalWrite(2, LOW);
        delay(20);
        digitalWrite(2, HIGH);           
        Serial.println("response blue");
      }

      // 清空接收的字符串
      receivedString = "";
    }
  }
}
