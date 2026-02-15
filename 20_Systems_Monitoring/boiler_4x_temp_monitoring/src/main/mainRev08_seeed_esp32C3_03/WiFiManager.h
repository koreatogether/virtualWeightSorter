#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>

void initWiFi();
void handleWiFi();
extern WebServer server;
String getLocalIP();

#endif // WIFIMANAGER_H
