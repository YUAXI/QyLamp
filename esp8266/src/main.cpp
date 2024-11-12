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
        delay(200);
        digitalWrite(2, HIGH);           
        Serial.println(receivedString);
      } 
      else if (receivedString == "on"){
        digitalWrite(2, LOW);         
        Serial.println(receivedString);
      }
      else if (receivedString == "off"){
        digitalWrite(2, HIGH);        
        Serial.println(receivedString);
      }
      else if (receivedString == "red"){
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);               
        Serial.println(receivedString);
      }
      else if (receivedString == "orange"){
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);                 
        Serial.println(receivedString);
      }
      else if (receivedString == "yellow"){
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);           
        Serial.println(receivedString);
      }
      else if (receivedString == "green"){
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);           
        Serial.println(receivedString);
      }
    else if (receivedString == "cyan"){
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);           
        Serial.println(receivedString);
      }
      else if (receivedString == "blue"){
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);           
        Serial.println(receivedString);
      }
      else if (receivedString == "purple"){
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);           
        Serial.println(receivedString);
      }
      else if (receivedString == "white"){
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);           
        Serial.println(receivedString);
      }
      else if (receivedString == "pink"){
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);           
        Serial.println(receivedString);
      }
      else if (receivedString == "darker"){
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);           
        Serial.println(receivedString);
      }
      else if (receivedString == "brighter"){
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);           
        Serial.println(receivedString);
      }
      else if (receivedString == "onmarquee"){
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);           
        Serial.println(receivedString);
      }
      else if (receivedString == "offmarquee"){
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);           
        Serial.println(receivedString);
      }
      else if (receivedString == "onbreathe"){
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);           
        Serial.println(receivedString);
      }
      else if (receivedString == "offbreathe"){
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);           
        Serial.println(receivedString);
      }
      else if (receivedString == "onflash"){
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);           
        Serial.println(receivedString);
      }
      else if (receivedString == "offflash"){
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);           
        Serial.println(receivedString);
      }
      else if (receivedString == "onrescue"){
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);           
        Serial.println(receivedString);
      }
      else if (receivedString == "offrescue"){
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);           
        Serial.println(receivedString);
      }
      // 清空接收的字符串
      receivedString = "";
    }
  }
}
