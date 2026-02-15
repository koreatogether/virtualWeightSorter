#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>

/**
 * @brief WiFi 및 네트워크 서비스 초기화
 */
void initWiFi();

/**
 * @brief WiFi 및 웹 서버 클라이언트 핸들링 루프
 */
void handleWiFi();

/**
 * @brief 현재 할당된 로컬 IP 주소 반환
 */
String getLocalIP();

extern WebServer server;

#endif // WIFIMANAGER_H
