/**
 * TFT LCD 2.4" GUI Test (320x240 Landscape)
 * Arduino R4 WiFi + Shield TFT LCD
 *
 * Features:
 * - 320x240 Landscape Mode (Dark Theme)
 * - 3 Screens: Main / Settings / Graph
 * - PM1.0, PM2.5, PM10 + Temperature/Humidity
 * - Active/Passive Mode Toggle
 * - History Graph with Min/Max/Avg Stats
 * - Demo Mode for Testing
 */

// ==========================================
// Configuration - TFT LCD 320x240
// ==========================================
int LCD_WIDTH = 320;
int LCD_HEIGHT = 240;
int SCALE = 2; // 2x scale for better visibility on PC

// Layout Heights
int TITLE_BAR_H = 25;
int TAB_BAR_H = 25;
int CONTENT_H = LCD_HEIGHT - TITLE_BAR_H - TAB_BAR_H; // 190px

// ==========================================
// Colors (Dark Theme)
// ==========================================
color cBackground = #202020;
color cPanel = #303030;
color cText = #EEEEEE;
color cTextDim = #888888;
color cAccent = #FF5722;
color cGreen = #4CAF50;
color cYellow = #FFC107;
color cRed = #F44336;
color cBlue = #2196F3;
color cButton = #404040;
color cButtonHover = #505050;
color cButtonActive = #FF5722;

// ==========================================
// Global Variables
// ==========================================
PFont fontSmall, fontMedium, fontLarge, fontXLarge;

// Current Screen: 0=Main, 1=Settings, 2=Graph
int currentScreen = 0;

// Sensor Data
int pm1 = 12;
int pm25 = 28;
int pm10 = 35;
float temperature = 24.5;
float humidity = 58.0;

// Sensor Mode
boolean isActiveMode = true; // true=Active, false=Passive
boolean isSleeping = false;
int sleepTimeOption = 5; // 1, 3, 5, 10, 20 minutes

// History Data (for graph)
int[] pm25History = new int[30]; // Last 30 readings
int historyIndex = 0;
int pm25Min = 999, pm25Max = 0;
float pm25Avg = 0;
int pm25Sum = 0;
int pm25Count = 0;

// Time
int hours = 12;
int minutes = 34;

// Demo Mode
boolean isDemoMode = true;

// Buttons for touch simulation
ArrayList<Button> mainButtons = new ArrayList<Button>();
ArrayList<Button> settingsButtons = new ArrayList<Button>();
ArrayList<Button> graphButtons = new ArrayList<Button>();
ArrayList<Button> tabButtons = new ArrayList<Button>();

void setup()
{
  size(640, 480); // 320x240 * 2 scale

  // Font Setup
  fontSmall = createFont("Malgun Gothic", 9 * SCALE);
  fontMedium = createFont("Malgun Gothic", 11 * SCALE);
  fontLarge = createFont("Malgun Gothic Bold", 14 * SCALE);
  fontXLarge = createFont("Malgun Gothic Bold", 22 * SCALE);

  // Initialize History
  for (int i = 0; i < pm25History.length; i++)
  {
    pm25History[i] = 0;
  }

  // Initialize Buttons
  initButtons();
}

void initButtons()
{
  int s = SCALE;

  // --- Tab Buttons (Bottom) ---
  int tabY = (TITLE_BAR_H + CONTENT_H) * s;
  int tabW = LCD_WIDTH / 3 * s;
  int tabH = TAB_BAR_H * s;

  tabButtons.add(new Button(0, "메인", 0, tabY, tabW, tabH));
  tabButtons.add(new Button(1, "설정", tabW, tabY, tabW, tabH));
  tabButtons.add(new Button(2, "그래프", tabW * 2, tabY, tabW, tabH));

  // --- Settings Screen Buttons ---
  int btnW = 50 * s;
  int btnH = 22 * s;
  int startY = 55 * s;
  int gap = 28 * s;

  // Sleep Time Buttons
  settingsButtons.add(new Button(101, "1분", 10 * s, startY, btnW, btnH));
  settingsButtons.add(new Button(103, "3분", 65 * s, startY, btnW, btnH));
  settingsButtons.add(new Button(105, "5분", 120 * s, startY, btnW, btnH));
  settingsButtons.add(new Button(110, "10분", 175 * s, startY, btnW, btnH));
  settingsButtons.add(new Button(120, "20분", 230 * s, startY, btnW, btnH));

  // Mode Toggle Button
  settingsButtons.add(new Button(200, "Active", 10 * s, startY + gap, 90 * s, btnH));
  settingsButtons.add(new Button(201, "Passive", 105 * s, startY + gap, 90 * s, btnH));

  // Reset Button
  settingsButtons.add(new Button(300, "데이터 리셋", 10 * s, startY + gap * 2, 120 * s, btnH));

  // Sleep/Wake Button (for Active mode)
  settingsButtons.add(new Button(400, "Sleep Now", 200 * s, startY + gap, 90 * s, btnH));
}

void draw()
{
  // Scale everything
  scale(SCALE);
  background(cBackground);

  // Draw Title Bar
  drawTitleBar();

  // Draw Content based on current screen
  if (currentScreen == 0)
  {
    drawMainScreen();
  }
  else if (currentScreen == 1)
  {
    drawSettingsScreen();
  }
  else
  {
    drawGraphScreen();
  }

  // Draw Tab Bar
  drawTabBar();

  // Demo Mode Update
  if (isDemoMode && frameCount % 60 == 0)
  {
    updateDemoData();
  }

  // Update time every second
  if (frameCount % 60 == 0)
  {
    minutes++;
    if (minutes >= 60)
    {
      minutes = 0;
      hours = (hours + 1) % 24;
    }
  }
}

// ==========================================
// Title Bar
// ==========================================
void drawTitleBar()
{
  fill(cPanel);
  noStroke();
  rect(0, 0, LCD_WIDTH, TITLE_BAR_H);

  // Project Name
  fill(cAccent);
  textFont(fontMedium);
  textAlign(LEFT, CENTER);
  text("Air Monitor", 8, TITLE_BAR_H / 2);

  // Mode Indicator
  if (isActiveMode)
  {
    fill(isSleeping ? cYellow : cGreen);
    ellipse(LCD_WIDTH - 70, TITLE_BAR_H / 2, 8, 8); // 원형 상태 표시
  }
  else
  {
    fill(cBlue);
    // 다이아몬드 모양
    pushMatrix();
    translate(LCD_WIDTH - 70, TITLE_BAR_H / 2);
    rotate(PI / 4);
    rectMode(CENTER);
    rect(0, 0, 6, 6);
    rectMode(CORNER);
    popMatrix();
  }

  // Current Time
  fill(cText);
  textFont(fontMedium);
  textAlign(RIGHT, CENTER);
  String timeStr = nf(hours, 2) + ":" + nf(minutes, 2);
  text(timeStr, LCD_WIDTH - 8, TITLE_BAR_H / 2);
}

// ==========================================
// Tab Bar (Bottom)
// ==========================================
void drawTabBar()
{
  int y = TITLE_BAR_H + CONTENT_H;
  int tabW = LCD_WIDTH / 3;

  for (int i = 0; i < 3; i++)
  {
    // Background
    if (currentScreen == i)
    {
      fill(cPanel);
    }
    else
    {
      fill(cButton);
    }
    noStroke();
    rect(i * tabW, y, tabW, TAB_BAR_H);

    // Text
    fill(currentScreen == i ? cAccent : cText);
    textFont(fontSmall);
    textAlign(CENTER, CENTER);
    String[] labels = {"메인", "설정", "그래프"};
    text(labels[i], i * tabW + tabW / 2, y + TAB_BAR_H / 2);

    // Active indicator line
    if (currentScreen == i)
    {
      stroke(cAccent);
      strokeWeight(2);
      line(i * tabW, y, (i + 1) * tabW, y);
      noStroke();
    }
  }
}

// ==========================================
// Main Screen
// ==========================================
void drawMainScreen()
{
  int y = TITLE_BAR_H;
  int contentH = CONTENT_H;

  // PM Values Area (top 110px)
  int pmAreaH = 110;
  int pmBoxW = LCD_WIDTH / 3;

  drawPMBox(0, y, pmBoxW, pmAreaH, "PM1.0", pm1);
  drawPMBox(pmBoxW, y, pmBoxW, pmAreaH, "PM2.5", pm25);
  drawPMBox(pmBoxW * 2, y, pmBoxW, pmAreaH, "PM10", pm10);

  // Separator line
  stroke(cPanel);
  strokeWeight(1);
  line(0, y + pmAreaH, LCD_WIDTH, y + pmAreaH);
  noStroke();

  // Temperature & Humidity Area (bottom 80px)
  int envY = y + pmAreaH;
  int envH = contentH - pmAreaH;
  int envW = LCD_WIDTH / 2;

  // Temperature
  fill(cPanel);
  rect(0, envY, envW - 1, envH);
  fill(cAccent);
  textFont(fontSmall);
  textAlign(CENTER, TOP);
  text("온도", envW / 2, envY + 5);
  fill(cText);
  textFont(fontXLarge);
  textAlign(CENTER, CENTER);
  text(nf(temperature, 0, 1) + "°C", envW / 2, envY + envH / 2 + 3);

  // Humidity
  fill(cPanel);
  rect(envW + 1, envY, envW - 1, envH);
  fill(cBlue);
  textFont(fontSmall);
  textAlign(CENTER, TOP);
  text("습도", envW + envW / 2, envY + 5);
  fill(cText);
  textFont(fontXLarge);
  textAlign(CENTER, CENTER);
  text(nf(humidity, 0, 1) + "%", envW + envW / 2, envY + envH / 2 + 3);
}

void drawPMBox(int x, int y, int w, int h, String label, int value)
{
  // Background
  fill(cPanel);
  noStroke();
  rect(x + 1, y + 1, w - 2, h - 2);

  // Label (top)
  fill(cTextDim);
  textFont(fontSmall);
  textAlign(CENTER, TOP);
  text(label, x + w / 2, y + 5);

  // Value (center - larger font)
  color valColor = getAQIColor(value);
  fill(valColor);
  textFont(fontXLarge);
  textAlign(CENTER, CENTER);
  text(str(value), x + w / 2, y + h / 2 - 5);

  // Unit (below value)
  fill(cTextDim);
  textFont(fontSmall);
  textAlign(CENTER, CENTER);
  text("㎍/㎥", x + w / 2, y + h / 2 + 18);

  // Status Text (bottom)
  String status = getAQIStatus(value);
  fill(valColor);
  textAlign(CENTER, BOTTOM);
  text(status, x + w / 2, y + h - 3);
}

color getAQIColor(int pm)
{
  if (pm <= 15)
    return cGreen;
  else if (pm <= 35)
    return cYellow;
  else if (pm <= 75)
    return cAccent;
  else
    return cRed;
}

String getAQIStatus(int pm)
{
  if (pm <= 15)
    return "좋음";
  else if (pm <= 35)
    return "보통";
  else if (pm <= 75)
    return "나쁨";
  else
    return "매우나쁨";
}

// ==========================================
// Settings Screen
// ==========================================
void drawSettingsScreen()
{
  int y = TITLE_BAR_H + 3;
  int btnH = 20;
  int sectionGap = 8;

  // === Sleep 시간 설정 ===
  fill(cText);
  textFont(fontSmall);
  textAlign(LEFT, TOP);
  text("Sleep 시간 설정", 10, y);

  int btnY = y + 14;
  int[] times = {1, 3, 5, 10, 20};
  int btnW = 55;
  int gap = 4;

  for (int i = 0; i < times.length; i++)
  {
    int bx = 10 + i * (btnW + gap);
    boolean selected = (sleepTimeOption == times[i]);

    fill(selected ? cAccent : cButton);
    rect(bx, btnY, btnW, btnH, 3);

    fill(selected ? cBackground : cText);
    textFont(fontSmall);
    textAlign(CENTER, CENTER);
    text(times[i] + "분", bx + btnW / 2, btnY + btnH / 2);
  }

  // === 센서 모드 ===
  int modeY = btnY + btnH + sectionGap;
  fill(cText);
  textFont(fontSmall);
  textAlign(LEFT, TOP);
  text("센서 모드", 10, modeY);

  int modeBtnY = modeY + 14;
  int modeBtnW = 95;

  // Active Button
  fill(isActiveMode ? cGreen : cButton);
  rect(10, modeBtnY, modeBtnW, btnH, 3);
  fill(isActiveMode ? cBackground : cText);
  textFont(fontSmall);
  textAlign(CENTER, CENTER);
  text("Active", 10 + modeBtnW / 2, modeBtnY + btnH / 2);

  // Passive Button
  fill(!isActiveMode ? cBlue : cButton);
  rect(10 + modeBtnW + 5, modeBtnY, modeBtnW, btnH, 3);
  fill(!isActiveMode ? cBackground : cText);
  text("Passive", 10 + modeBtnW + 5 + modeBtnW / 2, modeBtnY + btnH / 2);

  // Sleep/Wake Button (only in Active mode)
  if (isActiveMode)
  {
    fill(isSleeping ? cYellow : cButton);
    rect(10 + (modeBtnW + 5) * 2, modeBtnY, modeBtnW, btnH, 3);
    fill(isSleeping ? cBackground : cText);
    text(isSleeping ? "Wake" : "Sleep", 10 + (modeBtnW + 5) * 2 + modeBtnW / 2, modeBtnY + btnH / 2);
  }

  // === 데이터 관리 ===
  int resetY = modeBtnY + btnH + sectionGap;
  fill(cText);
  textFont(fontSmall);
  textAlign(LEFT, TOP);
  text("데이터 관리", 10, resetY);

  int resetBtnY = resetY + 14;
  fill(cRed);
  rect(10, resetBtnY, 100, btnH, 3);
  fill(cText);
  textFont(fontSmall);
  textAlign(CENTER, CENTER);
  text("데이터 리셋", 60, resetBtnY + btnH / 2);

  // === 현재 상태 ===
  int statusY = resetBtnY + btnH + sectionGap;
  fill(cTextDim);
  textFont(fontSmall);
  textAlign(LEFT, TOP);
  String statusText = isActiveMode ? (isSleeping ? "Sleep 모드" : "Active 측정중") : "Passive 대기중";
  text("상태: " + statusText + " / Sleep: " + sleepTimeOption + "분", 10, statusY);
}

// ==========================================
// Graph Screen
// ==========================================
void drawGraphScreen()
{
  int y = TITLE_BAR_H + 3;

  // Title
  fill(cText);
  textFont(fontSmall);
  textAlign(LEFT, TOP);
  text("PM2.5 히스토리", 10, y);

  // Stats Box
  int statsY = y + 14;
  int statsH = 22;
  fill(cPanel);
  rect(5, statsY, LCD_WIDTH - 10, statsH, 3);

  // Min / Avg / Max
  textFont(fontSmall);
  int statW = (LCD_WIDTH - 10) / 3;

  fill(cGreen);
  textAlign(CENTER, CENTER);
  text("최저: " + (pm25Min == 999 ? "-" : pm25Min), 5 + statW / 2, statsY + statsH / 2);

  fill(cYellow);
  text("평균: " + (pm25Count > 0 ? nf(pm25Avg, 0, 1) : "-"), 5 + statW + statW / 2, statsY + statsH / 2);

  fill(cRed);
  text("최고: " + (pm25Max == 0 ? "-" : pm25Max), 5 + statW * 2 + statW / 2, statsY + statsH / 2);

  // Graph Area
  int graphX = 25; // 왼쪽 여백 (Y축 라벨 공간)
  int graphY = statsY + statsH + 5;
  int graphW = LCD_WIDTH - graphX - 5;
  int graphH = CONTENT_H - (graphY - TITLE_BAR_H) - 5;

  // 동적 Y축 범위 계산
  int yMax = 50; // 기본 최대값
  if (pm25Max > 0)
  {
    // 최대값에 따라 범위 조정 (10 단위로 올림)
    yMax = ((pm25Max / 25) + 1) * 25;
    yMax = max(yMax, 50);  // 최소 50
    yMax = min(yMax, 200); // 최대 200
  }
  int yMid = yMax / 2;

  // Graph Background
  fill(0, 50);
  noStroke();
  rect(graphX, graphY, graphW, graphH);

  // Grid lines (3등분)
  stroke(cPanel);
  strokeWeight(1);
  for (int i = 1; i < 4; i++)
  {
    int lineY = graphY + (graphH * i / 4);
    line(graphX, lineY, graphX + graphW, lineY);
  }

  // Y-axis labels (동적)
  fill(cTextDim);
  textFont(fontSmall);
  textAlign(RIGHT, CENTER);
  text(str(yMax), graphX - 3, graphY + 5);
  text(str(yMid), graphX - 3, graphY + graphH / 2);
  text("0", graphX - 3, graphY + graphH - 5);

  // Threshold lines (35, 75 기준선)
  if (35 < yMax)
  {
    stroke(cYellow, 150);
    strokeWeight(1);
    float y35 = graphY + graphH - map(35, 0, yMax, 0, graphH);
    line(graphX, y35, graphX + graphW, y35);
  }

  if (75 < yMax)
  {
    stroke(cRed, 150);
    float y75 = graphY + graphH - map(75, 0, yMax, 0, graphH);
    line(graphX, y75, graphX + graphW, y75);
  }

  // Draw history line
  stroke(cAccent);
  strokeWeight(2);
  noFill();
  beginShape();
  for (int i = 0; i < pm25History.length; i++)
  {
    int idx = (historyIndex + i) % pm25History.length;
    if (pm25History[idx] > 0)
    {
      float px = graphX + (i * graphW / (pm25History.length - 1));
      float py = graphY + graphH - map(pm25History[idx], 0, yMax, 0, graphH);
      py = constrain(py, graphY, graphY + graphH); // 범위 내로 제한
      vertex(px, py);
    }
  }
  endShape();
  noStroke();
}

// ==========================================
// Demo Mode Data Update
// ==========================================
void updateDemoData()
{
  if (isSleeping)
    return;

  // Random PM values with some noise
  pm1 = constrain(pm1 + int(random(-3, 4)), 5, 50);
  pm25 = constrain(pm25 + int(random(-5, 6)), 10, 120);
  pm10 = constrain(pm10 + int(random(-4, 5)), 15, 150);

  // Random temperature/humidity
  temperature = constrain(temperature + random(-0.3, 0.3), 18, 32);
  humidity = constrain(humidity + random(-1, 1), 30, 80);

  // Update history
  pm25History[historyIndex] = pm25;
  historyIndex = (historyIndex + 1) % pm25History.length;

  // Update stats
  if (pm25 < pm25Min)
    pm25Min = pm25;
  if (pm25 > pm25Max)
    pm25Max = pm25;
  pm25Sum += pm25;
  pm25Count++;
  pm25Avg = (float)pm25Sum / pm25Count;
}

// ==========================================
// Mouse Interaction
// ==========================================
void mousePressed()
{
  // Convert mouse position to LCD coordinates
  int mx = mouseX / SCALE;
  int my = mouseY / SCALE;

  // Tab Bar Click
  int tabY = TITLE_BAR_H + CONTENT_H;
  if (my >= tabY && my < tabY + TAB_BAR_H)
  {
    int tabW = LCD_WIDTH / 3;
    currentScreen = mx / tabW;
    currentScreen = constrain(currentScreen, 0, 2);
  }

  // Settings Icon Click
  if (mx > LCD_WIDTH - 55 && mx < LCD_WIDTH - 35 && my < TITLE_BAR_H)
  {
    currentScreen = 1; // Go to settings
  }

  // Settings Screen Buttons
  if (currentScreen == 1)
  {
    handleSettingsClick(mx, my);
  }
}

void handleSettingsClick(int mx, int my)
{
  int y = TITLE_BAR_H + 3;
  int btnH = 20;
  int sectionGap = 8;

  int btnY = y + 14;
  int btnW = 55;
  int gap = 4;
  int[] times = {1, 3, 5, 10, 20};

  // Sleep Time Buttons
  for (int i = 0; i < times.length; i++)
  {
    int bx = 10 + i * (btnW + gap);
    if (mx >= bx && mx < bx + btnW && my >= btnY && my < btnY + btnH)
    {
      sleepTimeOption = times[i];
    }
  }

  // Mode Buttons
  int modeY = btnY + btnH + sectionGap;
  int modeBtnY = modeY + 14;
  int modeBtnW = 95;

  // Active Button
  if (mx >= 10 && mx < 10 + modeBtnW && my >= modeBtnY && my < modeBtnY + btnH)
  {
    isActiveMode = true;
  }

  // Passive Button
  if (mx >= 10 + modeBtnW + 5 && mx < 10 + modeBtnW * 2 + 5 && my >= modeBtnY && my < modeBtnY + btnH)
  {
    isActiveMode = false;
    isSleeping = false;
  }

  // Sleep/Wake Button
  if (isActiveMode && mx >= 10 + (modeBtnW + 5) * 2 && mx < 10 + (modeBtnW + 5) * 2 + modeBtnW && my >= modeBtnY && my < modeBtnY + btnH)
  {
    isSleeping = !isSleeping;
  }

  // Reset Button
  int resetY = modeBtnY + btnH + sectionGap;
  int resetBtnY = resetY + 14;
  if (mx >= 10 && mx < 110 && my >= resetBtnY && my < resetBtnY + btnH)
  {
    resetData();
  }
}

void resetData()
{
  pm25Min = 999;
  pm25Max = 0;
  pm25Sum = 0;
  pm25Count = 0;
  pm25Avg = 0;
  for (int i = 0; i < pm25History.length; i++)
  {
    pm25History[i] = 0;
  }
  historyIndex = 0;
}

// ==========================================
// Button Class (for future expansion)
// ==========================================
class Button
{
  int id;
  String label;
  float x, y, w, h;

  Button(int id, String l, float x, float y, float w, float h)
  {
    this.id = id;
    this.label = l;
    this.x = x;
    this.y = y;
    this.w = w;
    this.h = h;
  }

  void display(boolean selected)
  {
    fill(selected ? cAccent : cButton);
    rect(x, y, w, h, 3);

    fill(selected ? cBackground : cText);
    textFont(fontSmall);
    textAlign(CENTER, CENTER);
    text(label, x + w / 2, y + h / 2);
  }

  boolean isClicked(int mx, int my)
  {
    return mx >= x && mx < x + w && my >= y && my < y + h;
  }
}