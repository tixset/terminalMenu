#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif
#include <WiFiClient.h>

#include <terminalMenu.h>

#define ESP_NAME "esp-test-telnet-menu"
#define SSID "myWiFi"
#define PASS "12345678"

#define SERIAL_SPEED 115200

#define TELNET_PORT 23
#define TELNET_MAX_CLIENTS 3
#define TELNET_REPLY_TO_ALL false // этот флаг делает телнет общим для всех клиентов 
#define TELNET_WELCOME "Welcome!"

#if defined(ESP8266)
#define DEV_TYPE "ESP8266"
#elif defined(ESP32)
#define DEV_TYPE "ESP32"
#else
#define DEV_TYPE "?"
#endif

WiFiServer serverTelnet(TELNET_PORT);
WiFiClient telnetClients[TELNET_MAX_CLIENTS];

terminalMenu tMenu;
tmenuLines tLines[46];

bool onWifiConnect;
String inString;

void setup() {
	Serial.begin(SERIAL_SPEED);

	tMenu.init((tmenuLines*)tLines, sizeof(tLines) / sizeof(tLines[0]));
  tMenu.helpAttach(tGetHelp);
  tMenu.errAttach(tGetErr);
  
	tMenu.add("?|help", tRootHelp);
	byte LId = tMenu.add("id");
    tMenu.add(LId, "get", tGetId);
	byte LSystem = tMenu.add("system");
    byte LSerialSpeed = tMenu.add(LSystem, "serial-speed");
      tMenu.add(LSerialSpeed, "get", tGetSerialSpeed);
    byte LTelnetPort = tMenu.add(LSystem, "telnet-port");
      tMenu.add(LTelnetPort, "get", tGetTelnetPort);
	byte LWiFi = tMenu.add("wifi");
    byte LWiFiMode = tMenu.add(LWiFi, "mode");
      tMenu.add(LWiFiMode, "get", tGetWifiMode);
    byte LWiFiSSID = tMenu.add(LWiFi, "ssid");
      tMenu.add(LWiFiSSID, "get", tGetWifiSSID);
    byte LWiFiPassword = tMenu.add(LWiFi, "password");
      tMenu.add(LWiFiPassword, "get", tGetWifiPassword);
    tMenu.add(LWiFi, "status", tGetWifiStatus);
    tMenu.add(LWiFi, "signal", tGetWifiSignal);
    byte LWiFiChannel = tMenu.add(LWiFi, "channel");
      tMenu.add(LWiFiChannel, "get", tGetWifiChannel);
    tMenu.add(LWiFi, "reconnect", tWifiReConnect);
	byte LIP = tMenu.add("ip");
    tMenu.add(LIP, "mac", tGetMac);
    byte LIPIP = tMenu.add(LIP, "ip");
      tMenu.add(LIPIP, "get", tGetIpIp);
    byte LIPMask = tMenu.add(LIP, "mask");
      tMenu.add(LIPMask, "get", tGetIpMask);
    byte LIPGW = tMenu.add(LIP, "gw");
      tMenu.add(LIPGW, "get", tGetIpGw);
    byte LIPDNS1 = tMenu.add(LIP, "dns1");
      tMenu.add(LIPDNS1, "get", tGetIpDns1);
    byte LIPDNS2 = tMenu.add(LIP, "dns2");
      tMenu.add(LIPDNS2, "get", tGetIpDns2);
  byte LHostname = tMenu.add("hostname");
    tMenu.add(LHostname, "get", tGetHostname);
  tMenu.add("uptime", tGetUptime);
  byte LCHIP = tMenu.add("chip");
    tMenu.add(LCHIP, "type", tGetChipType);
    byte LCHIPFreq = tMenu.add(LCHIP, "freq");
      tMenu.add(LCHIPFreq, "get", tGetChipFreq);
#if defined(ESP32)
    tMenu.add(LCHIP, "temp", tGetChipTemp);
#endif
  byte LMEM = tMenu.add("mem");
    byte LMEMRAM = tMenu.add(LMEM, "ram");
      tMenu.add(LMEMRAM, "free", tGetRAM);
    byte LMEMROM = tMenu.add(LMEM, "rom");
      tMenu.add(LMEMROM, "free", tGetROM);
  tMenu.add("reboot", tReboot);

  wifiConnect();
}

void loop() {
  getWifiStatus();
  telnetHandle();
}

void wifiConnect(){
  WiFi.setAutoReconnect(true);
  WiFi.begin((char*)SSID,(char*)PASS);
  WiFi.setHostname((char*)ESP_NAME);
}

void getWifiStatus(){
  if(WiFi.status()==WL_CONNECTED){
    if(onWifiConnect==false){
      Serial.println("WIFI connected to "+String(SSID));
      Serial.println("hostname: "+String(WiFi.getHostname()));
      Serial.println("signal strength: "+String(WiFi.RSSI())+" dBm");
      Serial.println("channel: "+String(WiFi.channel()));
      Serial.println("IP address: "+WiFi.localIP().toString());
      Serial.println("subnet Mask: "+WiFi.subnetMask().toString());
      Serial.println("gateway IP: "+WiFi.gatewayIP().toString());
      Serial.println("DNS1: "+WiFi.dnsIP(0).toString());
      Serial.println("DNS2: "+WiFi.dnsIP(1).toString());
      
      serverTelnet.begin();
      serverTelnet.setNoDelay(true);
      Serial.println("starting telnet server on "+String(TELNET_PORT)+" port");

      onWifiConnect=true;
    }
    
  }else{
    onWifiConnect=false;
  }
}

void telnetHandle() {
  if (WiFi.status() == WL_CONNECTED) {
    if (serverTelnet.hasClient()) {
      int activeClientCount = 0;
      for (int i = 0; i < TELNET_MAX_CLIENTS; i++) {
        if (!telnetClients[i] || !telnetClients[i].connected()) {
          if (telnetClients[i]) {
            telnetClients[i].stop(); // если клиент отключился - не держим его в массиве
          }
          telnetClients[i]=serverTelnet.available(); // добавляем нового клиента в массив
          if (!telnetClients[i]) {
            Serial.println("telnet client available broken");
          } else {
            telnetClients[i].println(TELNET_WELCOME);
            telnetClients[i].println("Session: #" + String(i + 1) + ", " + telnetClients[i].remoteIP().toString() + " -> " + WiFi.localIP().toString());
            telnetClients[i].print(String(ESP_NAME) + ":" + String(i + 1) + "# ");
            Serial.println("new telnet client: " + String(i + 1) + " " + telnetClients[i].remoteIP().toString());
            if (telnetClients[i].available()) {
              while (telnetClients[i].available()) {
                telnetClients[i].read();
              }
            }
          }
          break;
        } else {
          activeClientCount++;
        }
      }
      if (activeClientCount + 1 >= TELNET_MAX_CLIENTS) { // не даём новому клиенту подключиться если количество подключенных клиентов равно максимальному
        serverTelnet.available().stop();
      }
    }
    for (int i = 0; i < TELNET_MAX_CLIENTS; i++) {
      if (telnetClients[i] && telnetClients[i].connected()) {
        if (telnetClients[i].available()) {
          while (telnetClients[i].available()) {
            telnetRead(telnetClients[i].read(), i);
          }
        }
      } else {
        if (telnetClients[i]) {
          telnetClients[i].stop(); // если клиент отключился - не держим его в массиве
        }
      }
    }
  } else {
    for (int i = 0; i < TELNET_MAX_CLIENTS; i++) {
      if (telnetClients[i]) {
        telnetClients[i].stop(); // если клиент отключился - не держим его в массиве
      }
    }
  }
}

void telnetRead(char inChar, int clientIndex) {
  if (inChar != ((char) 13)) {
    if (inChar == ((char) 10)) {
      inString.trim();
      for (int i = 0; i < TELNET_MAX_CLIENTS; i++) {
        if (telnetClients[i] && telnetClients[i].connected()) {
          if ((clientIndex != i) && (inString != "") && (TELNET_REPLY_TO_ALL)) {
            telnetClients[i].println("");
            telnetClients[i].println(String(ESP_NAME) + ":" + String(clientIndex + 1) + "# " + inString);
          }
          delay(1);
        }
      }

      int res = tMenu.goMenu(inString, clientIndex); // запрашиваем меню
      if ((res == -1) && (inString != "")) {
        telnetSend(inString + ": command is incorrect", clientIndex);
      }

      for (int i = 0; i < TELNET_MAX_CLIENTS; i++) {
        if (telnetClients[i] && telnetClients[i].connected()) {
          if (((inString != "") || (clientIndex == i)) && ((clientIndex == i) || (TELNET_REPLY_TO_ALL))) {
            telnetClients[i].print(String(ESP_NAME) + ":" + String(i + 1) + "# ");
          }
          delay(1);
        }
      }
      inString = "";
    } else {
      inString += inChar;
    }
  }
}

void telnetSend(String text, int clientIndex) {
  if (WiFi.status() == WL_CONNECTED) {
    if (TELNET_REPLY_TO_ALL) {
      for (int i = 0; i < TELNET_MAX_CLIENTS; i++) {
        if (telnetClients[i] && telnetClients[i].connected()) {
          telnetClients[i].println(text);
          delay(1);
        }
      }
    } else {
      telnetClients[clientIndex].println(text);
    }
  }
}

void tGetHelp(tfuncParams Params) {
  int x = 0;
  while (1) {
    String line = tMenu.getHelpLine(x, Params.menuIndex);
    if (line == "") break;
    telnetSend(line, Params.clientIndex);
    x++;
  }
}

void tGetErr(tfuncParams Params) {
  telnetSend(Params.param + ": command is incorrect", Params.clientIndex);
}

void tRootHelp(tfuncParams Params) {
  int x = 0;
  while (1) {
    String line = tMenu.getHelpLine(x);
    if (line == "") break;
    telnetSend(line, Params.clientIndex);
    x++;
  }
}

void tGetId(tfuncParams Params) {
  telnetSend((String)ESP_NAME, Params.clientIndex);
}

void tGetSerialSpeed(tfuncParams Params) {
  telnetSend((String)SERIAL_SPEED, Params.clientIndex);
}

void tGetTelnetPort(tfuncParams Params) {
  telnetSend((String)TELNET_PORT, Params.clientIndex);
}

void tGetWifiMode(tfuncParams Params) {
  telnetSend(WiFiGetMode(WiFi.getMode()), Params.clientIndex);
}

void tGetWifiSSID(tfuncParams Params) {
  telnetSend((String)SSID, Params.clientIndex);
}

void tGetWifiPassword(tfuncParams Params) {
  telnetSend((String)PASS, Params.clientIndex);
}

void tGetWifiStatus(tfuncParams Params) {
  telnetSend(WiFiStatus(WiFi.status()), Params.clientIndex);
}

void tGetWifiSignal(tfuncParams Params) {
  telnetSend((String)WiFi.RSSI() + " dBm", Params.clientIndex);
}

void tGetWifiChannel(tfuncParams Params) {
  telnetSend((String)WiFi.channel(), Params.clientIndex);
}

void tWifiReConnect(tfuncParams Params) {
  telnetSend("ok", Params.clientIndex);
  WiFi.reconnect();
}

void tGetMac(tfuncParams Params) {
  telnetSend((String)WiFi.macAddress(), Params.clientIndex);
}

void tGetIpIp(tfuncParams Params) {
  telnetSend(WiFi.localIP().toString(), Params.clientIndex);
}

void tGetIpMask(tfuncParams Params) {
  telnetSend(WiFi.subnetMask().toString(), Params.clientIndex);
}

void tGetIpGw(tfuncParams Params) {
  telnetSend(WiFi.gatewayIP().toString(), Params.clientIndex);
}

void tGetIpDns1(tfuncParams Params) {
  telnetSend(WiFi.dnsIP(0).toString(), Params.clientIndex);
}

void tGetIpDns2(tfuncParams Params) {
  telnetSend(WiFi.dnsIP(1).toString(), Params.clientIndex);
}

void tGetHostname(tfuncParams Params) {
  telnetSend(WiFi.getHostname(), Params.clientIndex);
}

void tGetUptime(tfuncParams Params) {
  String res = "";
  if ((Params.param == "-u") || (Params.param == "")) {
    res = getUptime();
  } else {
    if (Params.param == "-t") {
      res = (String)millis();
    } else {
      res = Params.param + ": invalid parameter";
    }
  }
  telnetSend(res, Params.clientIndex);
}

void tGetChipType(tfuncParams Params) {
  telnetSend((String)DEV_TYPE, Params.clientIndex);
}

void tGetChipFreq(tfuncParams Params) {
  telnetSend((String)ESP.getCpuFreqMHz() + " MHz", Params.clientIndex);
}

#if defined(ESP32)
void tGetChipTemp(tfuncParams Params) {
  telnetSend((String)temperatureRead() + " °C", Params.clientIndex);
}
#endif

void tGetRAM(tfuncParams Params) {
#if defined(ESP8266)
  telnetSend(String(ESP.getFreeHeap() / 1024) + " Kb", Params.clientIndex);
#elif defined(ESP32)
  telnetSend(String(ESP.getFreeHeap() / 1024) + "/" + String(ESP.getHeapSize() / 1024) + " Kb", Params.clientIndex);
#endif
}

void tGetROM(tfuncParams Params) {
  unsigned long ESPFreeSketchSpace = ESP.getFreeSketchSpace();
  telnetSend(String(ESPFreeSketchSpace / 1024) + "/" + String((ESP.getSketchSize() + ESPFreeSketchSpace) / 1024) + " Kb", Params.clientIndex);
}

void tReboot(tfuncParams Params) {
  telnetSend("ok", Params.clientIndex);
  ESP.restart();
}

String getUptime() {
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  int day = hr / 24;
  String res = "";
  res += String(day) + " days, ";
  if (hr < 10) {
    res += "0" + String((hr % 24)) + ":";
  } else {
    res += String((hr % 24)) + ":";
  }
  if ((min % 60) < 10) {
    res += "0" + String(min % 60) + ":";
  } else {
    res += String(min % 60) + ":";
  }
  if ((sec % 60) < 10) {
    res += "0" + String(sec % 60);
  } else {
    res += String(sec % 60);
  }
  return res;
}

String WiFiStatus(int status) {
  switch (status) {
    case WL_NO_SHIELD: // 255
      return "WL_NO_SHIELD";
      break;
    case WL_IDLE_STATUS:  // 0
      return "WL_IDLE_STATUS";
      break;
    case WL_NO_SSID_AVAIL: // 1
      return "WL_NO_SSID_AVAIL";
      break;
    case WL_SCAN_COMPLETED: // 2
      return "WL_SCAN_COMPLETED";
      break;
    case WL_CONNECTED: // 3
      return "WL_CONNECTED";
      break;
    case WL_CONNECT_FAILED: // 4
      return "WL_CONNECT_FAILED";
      break;
    case WL_CONNECTION_LOST: // 5
      return "WL_CONNECTION_LOST";
      break;
#if defined(ESP8266)
    case WL_WRONG_PASSWORD: // 6 - esp8266
      return "WL_WRONG_PASSWORD";
      break;
#endif
    case WL_DISCONNECTED: // 7 - esp8266, 6 - esp32
      return "WL_DISCONNECTED";
      break;
    default:
      return "UNKNOWN (" + (String)status + ")";
  }
  return "";
}

String WiFiGetMode(int mode) {
  switch (mode) {
    case WIFI_OFF: // 0
      return "WIFI_OFF";
      break;
    case WIFI_STA: // 1
      return "WIFI_STA";
      break;
    case WIFI_AP: // 2
      return "WIFI_AP";
      break;
    case WIFI_AP_STA: // 3
      return "WIFI_AP_STA";
      break;
    default:
      return "UNKNOWN (" + (String)mode + ")";
  }
  return "";
}
