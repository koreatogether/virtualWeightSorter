/**
 * PMS7003 GUI for Arduino Sleep Mode Example
 * 
 * Features:
 * - 800x600 Window, Dark Mode
 * - Serial Connection Screen (Filters COM0-2)
 * - Tabbed Interface: Monitor & Settings
 * - Real-time Bar Graph & Demo Mode
 * - Statistics (Orange/Red Event Counts)
 */

import processing.serial.*;

// ==========================================
// Configuration
// ==========================================
int BAUD_RATE = 115200;

// Colors (Dark Theme)
color cBackground = #202020;
color cPanel = #303030;
color cText = #EEEEEE;
color cAccent = #FF5722; // Orange/Red
color cGreen = #4CAF50;
color cYellow = #FFC107;
color cRed = #F44336;
color cButton = #404040;
color cButtonHover = #505050;
color cButtonActive = #FF5722;
color cDisabled = #2A2A2A;

// ==========================================
// Global Variables
// ==========================================
PApplet app = this; // Reference to the main PApplet instance
Serial myPort;
PFont mainFont;
PFont titleFont;

// App State
int APP_STATE_CONNECT = 0;
int APP_STATE_MAIN = 1;
int currentState = APP_STATE_CONNECT;
boolean isDemoMode = false;

// Data
int pm1 = 0;
int pm25 = 0;
int pm10 = 0;
String statusMessage = "Waiting for data...";
int countdown = -1;
boolean isSleeping = false;
String lastLog = "";

// Statistics
int countOrange = 0; // PM2.5 > 35
int countRed = 0;    // PM2.5 > 75

// UI State
int currentTab = 0; // 0: Monitor, 1: Settings
ArrayList<Button> settingsButtons = new ArrayList<Button>();
ArrayList<Button> connectionButtons = new ArrayList<Button>();
String[] availablePorts;
int selectedPortIndex = -1;

void setup() {
  size(800, 600);
  
  // Font Setup
  mainFont = createFont("Malgun Gothic", 16);
  titleFont = createFont("Malgun Gothic Bold", 24);
  textFont(mainFont);
  
  // Initialize UI
  scanPorts();
  initButtons();
}

void scanPorts() {
  String[] allPorts = Serial.list();
  ArrayList<String> validPorts = new ArrayList<String>();
  
  // Filter ports (Exclude COM0, COM1, COM2)
  for (String p : allPorts) {
    if (!p.equals("COM0") && !p.equals("COM1") && !p.equals("COM2")) {
      validPorts.add(p);
    }
  }
  
  availablePorts = validPorts.toArray(new String[0]);
  selectedPortIndex = -1;
  lastLog = "Scanned " + availablePorts.length + " available ports (COM3+).";
}

void initButtons() {
  // --- Connection Screen Buttons ---
  int btnW = 120;
  int btnH = 40;
  
  // Refresh
  connectionButtons.add(new Button(100, "새로고침", width/2 - 140, 400, btnW, btnH));
  // Connect
  connectionButtons.add(new Button(101, "연결하기", width/2, 400, btnW, btnH));
  // Skip
  connectionButtons.add(new Button(102, "건너뛰기 (데모)", width/2 + 140, 400, btnW, btnH));
  
  
  // --- Settings Tab Buttons ---
  int startY = 150;
  int gap = 60;
  settingsButtons.add(new Button(1, "1분 휴식", 300, startY, 200, 40));
  settingsButtons.add(new Button(3, "3분 휴식", 300, startY + gap, 200, 40));
  settingsButtons.add(new Button(5, "5분 휴식", 300, startY + gap*2, 200, 40));
  settingsButtons.add(new Button(10, "10분 휴식", 300, startY + gap*3, 200, 40));
  settingsButtons.add(new Button(20, "20분 휴식", 300, startY + gap*4, 200, 40));
  
  // Disconnect Button
  settingsButtons.add(new Button(999, "연결 끊기 (처음으로)", 300, startY + gap*6, 200, 40));
}

void draw() {
  background(cBackground);
  
  if (currentState == APP_STATE_CONNECT) {
    drawConnectionScreen();
  } else {
    drawMainScreen();
    if (isDemoMode) updateDemoData();
  }
  
  // Footer / Log
  drawFooter();
  
  // Read Serial
  if (myPort != null) {
    while (myPort.available() > 0) {
      String inString = myPort.readStringUntil('\n');
      if (inString != null) {
        parseSerial(inString.trim());
      }
    }
  }
}

void drawConnectionScreen() {
  textAlign(CENTER, CENTER);
  
  // Title
  fill(cAccent);
  textFont(titleFont);
  text("PMS7003 GUI 연결 설정", width/2, 100);
  
  textFont(mainFont);
  fill(cText);
  text("사용할 시리얼 포트를 선택하세요 (COM3 이상)", width/2, 150);
  
  // Port List
  int listX = width/2 - 150;
  int listY = 200;
  int listW = 300;
  int itemH = 30;
  
  stroke(cPanel);
  noFill();
  rect(listX, listY, listW, 150); // Frame
  
  if (availablePorts.length == 0) {
    fill(150);
    text("검색된 포트가 없습니다.", width/2, listY + 75);
  } else {
    for (int i = 0; i < availablePorts.length; i++) {
      // Highlight selected
      if (i == selectedPortIndex) {
        fill(cAccent);
        noStroke();
        rect(listX + 2, listY + 2 + (i * itemH), listW - 4, itemH - 4);
        fill(255);
      } else {
        fill(cText);
      }
      
      textAlign(LEFT, CENTER);
      text(availablePorts[i], listX + 10, listY + itemH/2 + (i * itemH));
    }
  }
  
  // Buttons
  for (Button b : connectionButtons) {
    b.display();
  }
}

void drawMainScreen() {
  // Header / Tabs
  drawTabs();
  
  // Content
  if (currentTab == 0) {
    drawMonitor();
  } else {
    drawSettings();
  }
}

void drawFooter() {
  fill(cPanel);
  rect(0, height - 30, width, 30);
  fill(150);
  textAlign(LEFT, CENTER);
  textFont(mainFont);
  text(lastLog, 10, height - 15);
  
  if (isDemoMode) {
    fill(cYellow);
    textAlign(RIGHT, CENTER);
    text("[DEMO MODE]", width - 10, height - 15);
  }
}

void drawTabs() {
  int tabWidth = width / 2;
  int tabHeight = 50;
  
  noStroke();
  
  // Tab 1: Monitor
  fill(currentTab == 0 ? cPanel : cBackground);
  if (currentTab == 0) fill(cPanel); else fill(cButton);
  rect(0, 0, tabWidth, tabHeight);
  fill(cText);
  textAlign(CENTER, CENTER);
  textFont(titleFont);
  text("모니터링 (Monitor)", tabWidth/2, tabHeight/2);
  
  // Tab 2: Settings
  if (currentTab == 1) fill(cPanel); else fill(cButton);
  rect(tabWidth, 0, tabWidth, tabHeight);
  fill(cText);
  text("설정 (Settings)", tabWidth + tabWidth/2, tabHeight/2);
  
  // Underline active tab
  stroke(cAccent);
  strokeWeight(3);
  if (currentTab == 0) line(0, tabHeight, tabWidth, tabHeight);
  else line(tabWidth, tabHeight, width, tabHeight);
  noStroke();
}

void drawMonitor() {
  // Status Area
  fill(cPanel);
  rect(20, 70, width - 40, 100, 10);
  
  fill(cText);
  textAlign(LEFT, TOP);
  textFont(titleFont);
  text("현재 상태", 40, 85);
  
  textFont(mainFont);
  if (isSleeping) {
    fill(cYellow);
    textSize(20);
    text("절전 모드 (Sleep Mode)", 40, 120);
    if (countdown >= 0) {
      fill(cAccent);
      textSize(30);
      textAlign(RIGHT, CENTER);
      text("남은 시간: " + countdown + "초", width - 60, 120);
    }
  } else {
    fill(cGreen);
    textSize(20);
    text("측정 중 (Active Mode)", 40, 120);
    fill(150);
    textSize(14);
    textAlign(RIGHT, CENTER);
    text("실시간 데이터 수신 중...", width - 60, 120);
  }
  
  // --- Statistics Area (New) ---
  int statsY = 180;
  int statsH = 60;
  
  // Orange Count Box
  fill(cPanel);
  rect(20, statsY, (width - 50)/2, statsH, 10);
  fill(cYellow);
  textAlign(LEFT, CENTER);
  textSize(16);
  text("나쁨 (Orange) 횟수: " + countOrange + "회", 40, statsY + statsH/2);
  
  // Red Count Box
  fill(cPanel);
  rect(width/2 + 5, statsY, (width - 50)/2, statsH, 10);
  fill(cRed);
  text("위험 (Red) 횟수: " + countRed + "회", width/2 + 25, statsY + statsH/2);
  
  
  // Bar Graph Area
  int graphX = 50;
  int graphY = 260; // Moved down slightly
  int graphW = width - 100;
  int graphH = 240; // Adjusted height
  
  // Graph Background
  fill(0, 50);
  rect(graphX, graphY, graphW, graphH);
  
  // Axes
  stroke(100);
  strokeWeight(1);
  line(graphX, graphY + graphH, graphX + graphW, graphY + graphH); // X axis
  line(graphX, graphY, graphX, graphY + graphH); // Y axis
  
  // Bars
  int barWidth = graphW / 4;
  int maxVal = 150; // Scale max
  
  drawBar("PM 1.0", pm1, graphX + barWidth * 0.5, graphY, graphH, maxVal);
  drawBar("PM 2.5", pm25, graphX + barWidth * 1.5, graphY, graphH, maxVal);
  drawBar("PM 10", pm10, graphX + barWidth * 2.5, graphY, graphH, maxVal);
  
  // Alert Overlay
  if (pm25 > 35 && !isSleeping) {
    noFill();
    stroke(cRed, 100 + 100 * sin(millis() / 200.0));
    strokeWeight(10);
    rect(0, 0, width, height);
    noStroke();
  }
}

void drawBar(String label, int val, float x, int y, int h, int maxVal) {
  float barH = map(val, 0, maxVal, 0, h);
  barH = constrain(barH, 0, h);
  
  // Color based on value
  color barColor = cGreen;
  if (val > 35) barColor = cYellow;
  if (val > 75) barColor = cRed;
  
  fill(barColor);
  rect(x, y + h - barH, 60, barH);
  
  fill(cText);
  textAlign(CENTER, BOTTOM);
  textSize(16);
  text(val, x + 30, y + h - barH - 5);
  
  textAlign(CENTER, TOP);
  text(label, x + 30, y + h + 10);
}

void drawSettings() {
  fill(cText);
  textAlign(CENTER, TOP);
  textSize(20);
  text("휴식 시간 설정 (Sleep Interval)", width/2, 100);
  
  for (Button b : settingsButtons) {
    b.display();
  }
  
  fill(100);
  textSize(14);
  text("버튼을 클릭하면 아두이노로 명령을 전송합니다.", width/2, 550);
}

void updateDemoData() {
  if (frameCount % 60 == 0) { // Every second
    if (isSleeping) {
      countdown--;
      if (countdown < 0) {
        isSleeping = false;
        statusMessage = "Measuring (Demo)...";
      }
    } else {
      // Randomize PM values
      pm1 = int(noise(frameCount * 0.01) * 50);
      pm25 = int(noise(frameCount * 0.02) * 100); // Increased range for demo
      pm10 = int(noise(frameCount * 0.03) * 120);
      
      // Update Stats
      if (pm25 > 75) countRed++;
      else if (pm25 > 35) countOrange++;
      
      // Randomly go to sleep
      if (random(1) < 0.01) {
        isSleeping = true;
        countdown = 10;
        statusMessage = "Sleeping (Demo)...";
      }
    }
  }
}

void parseSerial(String line) {
  // 1. Data Line
  if (line.indexOf("PM1.0:") != -1) {
    isSleeping = false;
    countdown = -1;
    statusMessage = "Measuring...";
    
    try {
      String[] parts = splitTokens(line, ":|");
      if (parts.length >= 6) {
        pm1 = int(parts[1].trim());
        pm25 = int(parts[3].trim());
        pm10 = int(parts[5].trim());
        
        // Update Stats
        if (pm25 > 75) countRed++;
        else if (pm25 > 35) countOrange++;
      }
    } catch (Exception e) { }
  }
  // 2. Countdown
  else if (line.indexOf("남은 시간:") != -1) {
    isSleeping = true;
    try {
      String[] parts = split(line, "남은 시간:");
      if (parts.length > 1) {
        String numStr = parts[1].replace("초", "").replace("(설정 변경 가능: 1,3,5,10,20)", "").trim();
        countdown = int(numStr);
      }
    } catch (Exception e) { }
  }
  // 3. State Transitions
  else if (line.indexOf("Wake Up") != -1) {
    isSleeping = false;
    countdown = -1;
    lastLog = "Sensor Waking Up...";
  }
  else if (line.indexOf("Sleep") != -1) {
    isSleeping = true;
    lastLog = "Sensor Going to Sleep...";
  }
  else if (line.indexOf("변경되었습니다") != -1) {
    lastLog = line;
  }
}

void mousePressed() {
  if (currentState == APP_STATE_CONNECT) {
    // Port List Selection
    int listX = width/2 - 150;
    int listY = 200;
    int itemH = 30;
    
    if (mouseX > listX && mouseX < listX + 300 && mouseY > listY && mouseY < listY + 150) {
      int index = (mouseY - listY) / itemH;
      if (index >= 0 && index < availablePorts.length) {
        selectedPortIndex = index;
      }
    }
    
    // Buttons
    for (Button b : connectionButtons) {
      if (b.isMouseOver()) b.onClick();
    }
    
  } else {
    // Main App
    // Tab Switching
    if (mouseY < 50) {
      if (mouseX < width/2) currentTab = 0;
      else currentTab = 1;
    }
    
    // Settings Buttons
    if (currentTab == 1) {
      for (Button b : settingsButtons) {
        if (b.isMouseOver()) b.onClick();
      }
    }
  }
}

// Button Class
class Button {
  int id;
  String label;
  float x, y, w, h;
  
  Button(int id, String l, float x, float y, float w, float h) {
    this.id = id;
    this.label = l;
    this.x = x;
    this.y = y;
    this.w = w;
    this.h = h;
  }
  
  void display() {
    // Disable "Connect" if no port selected
    boolean disabled = (id == 101 && selectedPortIndex == -1);
    
    if (disabled) fill(cDisabled);
    else if (isMouseOver()) fill(cButtonHover);
    else fill(cButton);
    
    stroke(0);
    rect(x, y, w, h, 5);
    
    fill(disabled ? 100 : cText);
    textAlign(CENTER, CENTER);
    textSize(14);
    text(label, x + w/2, y + h/2);
  }
  
  boolean isMouseOver() {
    return mouseX > x && mouseX < x + w && mouseY > y && mouseY < y + h;
  }
  
  void onClick() {
    if (id == 100) { // Refresh
      scanPorts();
    }
    else if (id == 101) { // Connect
      if (selectedPortIndex != -1) {
        try {
          myPort = new Serial(app, availablePorts[selectedPortIndex], BAUD_RATE);
          currentState = APP_STATE_MAIN;
          isDemoMode = false;
          lastLog = "Connected to " + availablePorts[selectedPortIndex];
        } catch (Exception e) {
          lastLog = "Connection Failed: " + e.getMessage();
        }
      }
    }
    else if (id == 102) { // Skip
      currentState = APP_STATE_MAIN;
      isDemoMode = true;
      lastLog = "Entered Demo Mode";
    }
    else if (id == 999) { // Disconnect
      if (myPort != null) {
        myPort.stop();
        myPort = null;
      }
      currentState = APP_STATE_CONNECT;
      scanPorts();
      lastLog = "Disconnected";
    }
    else { // Command Buttons (1, 3, 5...)
      if (myPort != null) {
        myPort.write(str(id));
        lastLog = "Sent command: " + id;
      } else if (isDemoMode) {
        lastLog = "[Demo] Simulated command: " + id;
        // Simulate response in demo mode
        if (id == 1) countdown = 60;
        else if (id == 3) countdown = 180;
        // ...
      }
    }
  }
}
