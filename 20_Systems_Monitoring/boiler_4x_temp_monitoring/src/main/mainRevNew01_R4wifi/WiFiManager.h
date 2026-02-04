#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <Arduino.h>
#include <WiFiS3.h>
#include <ArduinoMDNS.h>

void initWiFi();
void handleMDNS();
extern WiFiServer server;
String getLocalIP();

#endif // WIFIMANAGER_H
