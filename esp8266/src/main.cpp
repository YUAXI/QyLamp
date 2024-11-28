#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>

#define PIN        15        // WS2812灯带数据线连接的引脚
#define NUMPIXELS  9         // WS2812灯珠数量
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL   30         // 渐变速度控制
#define server_ip "bemfa.com" //巴法云服务器地址默认即可
#define server_port "8344" //服务器端口，tcp创客云端口8344

//********************需要修改的部分*******************//

#define wifi_name  "OpenWrt_319_2.4G"     //WIFI名称，区分大小写，不要写错
#define wifi_password   "319319666"  //WIFI密码
String UID = "6f17c3f96c79b52c35d71749eed0e47e";
String TOPIC = "yuaxi002";         //主题名字，可在控制台新建

//**************************************************//
//最大字节数
#define MAX_PACKETSIZE 512
//设置心跳值60s
#define KEEPALIVEATIME 60*1000
//tcp客户端相关初始化，默认即可
WiFiClient TCPclient;
String TcpClient_Buff = "";//初始化字符串，用于接收服务器发来的数据
unsigned int TcpClient_BuffIndex = 0;
unsigned long TcpClient_preTick = 0;
unsigned long preHeartTick = 0;//心跳
unsigned long preTCPStartTick = 0;//连接
bool preTCPConnected = false;


SoftwareSerial mySerial(4, 5); // 定义软串口

String receivedString = "";  // 接收串口数据的字符串
uint32_t currentColor = 0;   // 当前颜色的32位RGB值
int brightness = 80;        // 当前亮度，初始为80%（最大值）
bool isFlashing = false;     // 是否正在闪烁的标志
bool isMarquee = false;      // 是否启用跑马灯效果
bool isRescue = false;       // 是否启用求救信号
unsigned long lastFlashTime = 0; // 上一次闪烁时间
bool flashState = false;     // 当前闪烁状态（亮或灭）

// 颜色定义
uint32_t colors[] = {
  pixels.Color(255, 0, 0),     // 红色
  pixels.Color(255, 165, 0),   // 橙色
  pixels.Color(255, 255, 0),   // 黄色
  pixels.Color(0, 255, 0),     // 绿色
  pixels.Color(0, 255, 255),   // 青色
  pixels.Color(0, 0, 255),     // 蓝色
  pixels.Color(128, 0, 128),   // 紫色
  pixels.Color(255, 255, 255), // 白色
  pixels.Color(255, 90, 90)  // 粉色
};

// 函数声明
uint32_t getColorFromString(String colorName);
void handleFlashing();
void handleRescue();
void handleMarquee();
void setBrightness(int value);
void handleCommand(String command);
void colorTransition(uint32_t targetColor, int steps);

//相关函数初始化
//连接WIFI
void doWiFiTick();
void startSTA();

//TCP初始化连接
void doTCPClientTick();
void startTCPClient();
void sendtoTCPServer(String p);

//led控制函数，具体函数内容见下方
void turnOnLed();
void turnOffLed();

// 获取颜色字符串对应的颜色
uint32_t getColorFromString(String colorName) {
  if (colorName.equalsIgnoreCase("red")) return colors[0];
  if (colorName.equalsIgnoreCase("orange")) return colors[1];
  if (colorName.equalsIgnoreCase("yellow")) return colors[2];
  if (colorName.equalsIgnoreCase("green")) return colors[3];
  if (colorName.equalsIgnoreCase("cyan")) return colors[4];
  if (colorName.equalsIgnoreCase("blue")) return colors[5];
  if (colorName.equalsIgnoreCase("purple")) return colors[6];
  if (colorName.equalsIgnoreCase("white")) return colors[7];
  if (colorName.equalsIgnoreCase("pink")) return colors[8];
  return currentColor; // 默认返回当前颜色
}

// 发送数据到TCP服务器
void sendtoTCPServer(String p){
  if (!TCPclient.connected()) 
  {
    Serial.println("Client is not readly");
    return;
  }
  TCPclient.print(p);
  preHeartTick = millis();//心跳计时开始，需要每隔60秒发送一次数据
}


// 初始化和服务器建立连接
void startTCPClient(){
  if(TCPclient.connect(server_ip, atoi(server_port))){
    Serial.print("\nConnected to server:");
    Serial.printf("%s:%d\r\n",server_ip,atoi(server_port));
    
    String tcpTemp="";  //初始化字符串
    tcpTemp = "cmd=1&uid="+UID+"&topic="+TOPIC+"\r\n"; //构建订阅指令
    sendtoTCPServer(tcpTemp); //发送订阅指令
    tcpTemp="";//清空
    /*
     //如果需要订阅多个主题，可发送  cmd=1&uid=xxxxxxxxxxxxxxxxxxxxxxx&topic=xxx1,xxx2,xxx3,xxx4\r\n
    教程：https://bbs.bemfa.com/64
     */
    preTCPConnected = true;
    TCPclient.setNoDelay(true);
  }
  else{
    Serial.print("Failed connected to server:");
    Serial.println(server_ip);
    TCPclient.stopAll();
    preTCPConnected = false;
  }
  preTCPStartTick = millis();
}

// 检查数据，发送心跳
void doTCPClientTick(){
 //检查是否断开，断开后重连
  if(WiFi.status() != WL_CONNECTED) return;
  if (!TCPclient.connected()) {//断开重连
  if(preTCPConnected == true){
    preTCPConnected = false;
    preTCPStartTick = millis();
    Serial.println();
    Serial.println("TCP Client disconnected.");
    TCPclient.stopAll();
  }
  else if(millis() - preTCPStartTick > 1*1000){//重新连接
    TCPclient.stopAll();
    startTCPClient();
    }
  }
  else
  {
    if (TCPclient.available()) {//收数据
      char c =TCPclient.read();
      TcpClient_Buff +=c;
      TcpClient_BuffIndex++;
      TcpClient_preTick = millis();
      
      if(TcpClient_BuffIndex>=MAX_PACKETSIZE - 1){
        TcpClient_BuffIndex = MAX_PACKETSIZE-2;
        TcpClient_preTick = TcpClient_preTick - 200;
      }
 
    }
    if(millis() - preHeartTick >= KEEPALIVEATIME){//保持心跳
      preHeartTick = millis();
      Serial.println("--Keep alive:");
      sendtoTCPServer("ping\r\n"); //发送心跳，指令需\r\n结尾，详见接入文档介绍
    }
  }
  if((TcpClient_Buff.length() >= 1) && (millis() - TcpClient_preTick>=200))
  {
    TCPclient.flush();
    Serial.print("Rev string: ");
    TcpClient_Buff.trim(); //去掉首位空格
    Serial.println(TcpClient_Buff); //打印接收到的消息
    String getTopic = "";
    String getMsg = "";
    if(TcpClient_Buff.length() > 15){//注意TcpClient_Buff只是个字符串，在上面开头做了初始化 String TcpClient_Buff = "";
      //此时会收到推送的指令，指令大概为 cmd=2&uid=xxx&topic=light002&msg=off
      int topicIndex = TcpClient_Buff.indexOf("&topic=")+7; //c语言字符串查找，查找&topic=位置，并移动7位，不懂的可百度c语言字符串查找
      int msgIndex = TcpClient_Buff.indexOf("&msg=");//c语言字符串查找，查找&msg=位置
      getTopic = TcpClient_Buff.substring(topicIndex,msgIndex);//c语言字符串截取，截取到topic,不懂的可百度c语言字符串截取
      getMsg = TcpClient_Buff.substring(msgIndex+5);//c语言字符串截取，截取到消息
      Serial.print("topic:------");
      Serial.println(getTopic); //打印截取到的主题值
      Serial.print("msg:--------");
      Serial.println(getMsg);   //打印截取到的消息值
   }
   if(getMsg  == "on"){       //如果是消息==打开
     turnOnLed();
   }else if(getMsg == "off"){ //如果是消息==关闭
      turnOffLed();
    }
   TcpClient_Buff="";
   TcpClient_BuffIndex = 0;
  }
}

// 初始化wifi连接
void startSTA(){
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_name, wifi_password);
}


/**************************************************************************
                                 WIFI
***************************************************************************/
/*
  WiFiTick
  检查是否需要初始化WiFi
  检查WiFi是否连接上，若连接成功启动TCP Client
  控制指示灯
*/
void doWiFiTick(){
  static bool startSTAFlag = false;
  static bool taskStarted = false;
  static uint32_t lastWiFiCheckTick = 0;

  if (!startSTAFlag) {
    startSTAFlag = true;
    startSTA();
  }

  //未连接1s重连
  if ( WiFi.status() != WL_CONNECTED ) {
    if (millis() - lastWiFiCheckTick > 1000) {
      lastWiFiCheckTick = millis();
    }
    taskStarted = false;
  }
  //连接成功建立
  else {
    if (taskStarted == false) {
      taskStarted = true;
      Serial.print("\r\nGet IP Address: ");
      Serial.println(WiFi.localIP());
      startTCPClient();
    }
  }
}

//开灯
void turnOnLed(){
  Serial.println("Turn ON");
  // 开灯时确保从黑色渐变到目标颜色
  currentColor = 0;  // 将当前颜色设置为黑色
  uint32_t targetColor = colors[7];  // 白色
  colorTransition(targetColor, 30);  // 逐渐过渡到白色
}

//关灯
void turnOffLed(){
  Serial.println("Turn OFF");
  for (int i = brightness; i > 0; i -= 10) {
      setBrightness(i);
      delay(DELAYVAL);  // 每次减少亮度的延时
    }
    pixels.clear();  // 完全关闭灯带
    pixels.show();
    brightness = 80;  // 恢复为初始亮度，可以根据需要修改
    setBrightness(brightness);
}

// 颜色渐变函数
void colorTransition(uint32_t targetColor, int steps) {
  uint8_t currentR = (currentColor >> 16) & 0xFF;
  uint8_t currentG = (currentColor >> 8) & 0xFF;
  uint8_t currentB = currentColor & 0xFF;

  uint8_t targetR = (targetColor >> 16) & 0xFF;
  uint8_t targetG = (targetColor >> 8) & 0xFF;
  uint8_t targetB = targetColor & 0xFF;

  // 从黑色 (0, 0, 0) 到目标颜色渐变
  for (int step = 1; step <= steps; step++) {
    uint8_t newR = (currentR * (steps - step) + targetR * step) / steps;
    uint8_t newG = (currentG * (steps - step) + targetG * step) / steps;
    uint8_t newB = (currentB * (steps - step) + targetB * step) / steps;

    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(newR, newG, newB));
    }
    pixels.show();
    delay(DELAYVAL);
  }

  currentColor = targetColor; // 更新当前颜色
}

// 设置亮度
void setBrightness(int value) {
  int scaledBrightness = map(value, 0, 100, 0, 255);
  pixels.setBrightness(scaledBrightness);
  pixels.show();
}

// 求救信号：SOS
void handleRescue() {
  if (isRescue) {
    static int phase = 0;
    static unsigned long lastPhaseTime = 0;
    unsigned long currentTime = millis();

    if (currentTime - lastPhaseTime >= 250) { // 间隔250ms切换
      lastPhaseTime = currentTime;
      phase++;
      if (phase > 11) phase = 0;

      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, (phase % 2 == 0) ? pixels.Color(255, 0, 0) : 0);
      }
      pixels.show();
    }
  }
}

// 跑马灯效果
void handleMarquee() {
  static int offset = 0;
  if (isMarquee) {
    for (int i = 0; i < NUMPIXELS; i++) {
      int colorIndex = (i + offset) % (sizeof(colors) / sizeof(colors[0]));
      pixels.setPixelColor(i, colors[colorIndex]);
    }
    pixels.show();
    offset++;
    delay(200);
  }
}

// 处理闪烁
void handleFlashing() {
  if (isFlashing) {
    unsigned long currentTime = millis();
    if (currentTime - lastFlashTime >= 100) {
      lastFlashTime = currentTime;
      flashState = !flashState;
      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, flashState ? pixels.Color(255, 255, 255) : 0);
      }
      pixels.show();
    }
  }
}

// 处理命令
void handleCommand(String command) {
  if (command.equalsIgnoreCase("onflash")) {
    isFlashing = true;
  } else if (command.equalsIgnoreCase("offflash")) {
    isFlashing = false;
    pixels.clear();
    pixels.show();
  } else if (command.equalsIgnoreCase("onrescue")) {
    isRescue = true;
  } else if (command.equalsIgnoreCase("offrescue")) {
    isRescue = false;
    pixels.clear();
    pixels.show();
  } else if (command.equalsIgnoreCase("darker")) {
    if (brightness > 20) {
      brightness -= 20;
      setBrightness(brightness);
    } else {
      mySerial.println("darkest");
    }
  } else if (command.equalsIgnoreCase("brighter")) {
    if (brightness < 100) {
      brightness += 20;
      setBrightness(brightness);
    } else {
      mySerial.println("brightest");
    }
  } else if (command.equalsIgnoreCase("onmarquee")) {
    isMarquee = true;
  } else if (command.equalsIgnoreCase("offmarquee")) {
    isMarquee = false;
    pixels.clear();
    pixels.show();
  } else if (command.equalsIgnoreCase("on")) {
    // 开灯时确保从黑色渐变到目标颜色
    currentColor = 0;  // 将当前颜色设置为黑色
    uint32_t targetColor = colors[7];  // 白色
    colorTransition(targetColor, 30);  // 逐渐过渡到白色
  } else if (command.equalsIgnoreCase("off")) {
    // 实现逐渐熄灯的效果
    for (int i = brightness; i > 0; i -= 10) {
      setBrightness(i);
      delay(DELAYVAL);  // 每次减少亮度的延时
    }
    pixels.clear();  // 完全关闭灯带
    pixels.show();
    brightness = 80;  // 恢复为初始亮度，可以根据需要修改
    setBrightness(brightness);
    currentColor = 0;  // 关闭时将当前颜色重置为黑色
  } else if (
    command.equalsIgnoreCase("red") ||
    command.equalsIgnoreCase("orange") ||
    command.equalsIgnoreCase("yellow") ||  
    command.equalsIgnoreCase("green") ||
    command.equalsIgnoreCase("cyan") ||  
    command.equalsIgnoreCase("blue") ||
    command.equalsIgnoreCase("purple") ||
    command.equalsIgnoreCase("white")  ||
    command.equalsIgnoreCase("pink") 
  ) {
    uint32_t targetColor = getColorFromString(command);
    colorTransition(targetColor, 30);
  }
}

void setup() {
  mySerial.begin(9600);
  Serial.begin(9600);
  pixels.begin();
  pixels.clear();
  setBrightness(brightness);
}

void loop() {
  while (mySerial.available() > 0) {
    char receivedChar = mySerial.read();
    receivedString += receivedChar;

    if (receivedChar == '\n') {
      isFlashing = false;
      isMarquee = false;
      isRescue = false;
      receivedString.trim();
      handleCommand(receivedString);
      receivedString = "";
    }
  }
  handleFlashing();
  handleRescue();
  handleMarquee();
  doWiFiTick();
  doTCPClientTick();
}
