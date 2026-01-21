#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <Arduino.h>
#include <WiFiS3.h>
#include <ArduinoMDNS.h>

// WiFi 상태 초기화 및 연결
void initWiFi();

// MDNS 갱신 처리 (루프에서 호출 필요)
void handleMDNS();

// 웹 서버 서버 객체 외부 노출
extern WiFiServer server;

// 현재 할당된 IP 주소 출력
String getLocalIP();

#endif // WIFIMANAGER_H
