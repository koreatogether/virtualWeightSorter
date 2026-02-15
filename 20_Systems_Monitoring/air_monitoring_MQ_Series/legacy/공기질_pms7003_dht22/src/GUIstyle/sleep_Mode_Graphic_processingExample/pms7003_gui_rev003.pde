/**
 * TFT LCD 2.4" GUI Test (320x240 Landscape) - Rev003
 * Arduino R4 WiFi + Shield TFT LCD
 *
 * Rev003 Changes:
 * - Sleep 시간 설정: Passive 모드에서만 활성화
 * - Sleep 모드 카운트다운 표시 (타이틀바)
 * - Active/Sleep 남은 시간 표시
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
color cDisabled = #2A2A2A;

// ==========================================
// Global Variables
// ==========================================
PFont fontSmall, fontMedium, fontLarge, fontXLarge, fontTiny;

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

// Sleep/Active Cycle Timer (for Active mode)
int sleepCountdown = 0;       // Sleep 남은 시간 (초)
int activeCountdown = 0;      // Active 남은 시간 (초)
int activeDuration = 30;      // Active 측정 시간 (초) - 고정 30초
boolean inSleepCycle = false; // 현재 Sleep 사이클인지

// Passive Mode Measurement
boolean isPassiveMeasuring = false; // Passive 측정 중인지
int passiveMeasureCountdown = 0;    // 남은 시간 (초)
int passiveStabilizeTime = 10;      // 안정화 시간 (초)
int passiveCollectTime = 10;        // 수집 시간 (초)
int passiveTotalTime = 20;          // 총 측정 시간 (초)
int[] passiveSamples = new int[10]; // 수집된 샘플 (10개)
int passiveSampleIndex = 0;         // 현재 샘플 인덱스
boolean passiveStabilizing = true;  // 안정화 중인지

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
int seconds = 0;

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
    fontTiny = createFont("Malgun Gothic", 7 * SCALE);
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

    // Draw Modal (if measuring in Passive mode)
    if (isPassiveMeasuring)
    {
        drawMeasuringModal();
    }

    // Demo Mode Update (every second)
    if (isDemoMode && frameCount % 60 == 0)
    {
        updateDemoData();
        updateSleepCycle();
        updatePassiveMeasurement();
    }

    // Update time every second
    if (frameCount % 60 == 0)
    {
        seconds++;
        if (seconds >= 60)
        {
            seconds = 0;
            minutes++;
            if (minutes >= 60)
            {
                minutes = 0;
                hours = (hours + 1) % 24;
            }
        }
    }
}

// ==========================================
// Sleep Cycle Management
// ==========================================
void updateSleepCycle()
{
    if (!isActiveMode)
        return; // Passive 모드에서는 작동 안함
    if (!isSleeping)
        return; // Sleep 모드가 아니면 작동 안함

    if (inSleepCycle)
    {
        // Sleep 중
        sleepCountdown--;
        if (sleepCountdown <= 0)
        {
            // Sleep 끝 -> Active로 전환
            inSleepCycle = false;
            activeCountdown = activeDuration;
        }
    }
    else
    {
        // Active 측정 중
        activeCountdown--;
        if (activeCountdown <= 0)
        {
            // Active 끝 -> Sleep으로 전환
            inSleepCycle = true;
            sleepCountdown = sleepTimeOption * 60; // 분 -> 초
        }
    }
}

void startSleepMode()
{
    isSleeping = true;
    inSleepCycle = false; // Active부터 시작
    activeCountdown = activeDuration;
    sleepCountdown = sleepTimeOption * 60;
}

void stopSleepMode()
{
    isSleeping = false;
    inSleepCycle = false;
    activeCountdown = 0;
    sleepCountdown = 0;
}

// ==========================================
// Passive Mode Measurement
// ==========================================
void startPassiveMeasurement()
{
    isPassiveMeasuring = true;
    passiveMeasureCountdown = passiveTotalTime;
    passiveStabilizing = true;
    passiveSampleIndex = 0;

    // 샘플 초기화
    for (int i = 0; i < passiveSamples.length; i++)
    {
        passiveSamples[i] = 0;
    }
}

void updatePassiveMeasurement()
{
    if (!isPassiveMeasuring)
        return;

    passiveMeasureCountdown--;

    if (passiveMeasureCountdown <= passiveCollectTime)
    {
        // 수집 단계
        passiveStabilizing = false;

        // 1초마다 샘플 수집
        if (passiveSampleIndex < passiveSamples.length)
        {
            // 데모 모드: 랜덤 값 생성
            passiveSamples[passiveSampleIndex] = int(random(15, 80));
            passiveSampleIndex++;
        }
    }

    if (passiveMeasureCountdown <= 0)
    {
        // 측정 완료 - 결과 계산
        finishPassiveMeasurement();
    }
}

void finishPassiveMeasurement()
{
    isPassiveMeasuring = false;

    // 샘플 정렬
    int[] sorted = new int[passiveSamples.length];
    arrayCopy(passiveSamples, sorted);
    java.util.Arrays.sort(sorted);

    // 최고 1개, 최저 1개 제거 후 나머지 8개 평균
    int sum = 0;
    for (int i = 1; i < sorted.length - 1; i++)
    {
        sum += sorted[i];
    }
    int avgValue = sum / (sorted.length - 2);

    // PM 값 업데이트 (PM2.5 기준으로 비율 적용)
    pm25 = avgValue;
    pm1 = int(avgValue * 0.5);
    pm10 = int(avgValue * 1.3);

    // 히스토리 업데이트
    pm25History[historyIndex] = pm25;
    historyIndex = (historyIndex + 1) % pm25History.length;

    // 통계 업데이트
    if (pm25 < pm25Min)
        pm25Min = pm25;
    if (pm25 > pm25Max)
        pm25Max = pm25;
    pm25Sum += pm25;
    pm25Count++;
    pm25Avg = (float)pm25Sum / pm25Count;
}

// ==========================================
// Measuring Modal (Passive Mode)
// ==========================================
void drawMeasuringModal()
{
    // 반투명 오버레이
    fill(0, 180);
    noStroke();
    rect(0, 0, LCD_WIDTH, LCD_HEIGHT);

    // 모달 박스
    int modalW = 200;
    int modalH = 80;
    int modalX = (LCD_WIDTH - modalW) / 2;
    int modalY = (LCD_HEIGHT - modalH) / 2;

    fill(cPanel);
    stroke(cAccent);
    strokeWeight(2);
    rect(modalX, modalY, modalW, modalH, 5);
    noStroke();

    // 제목
    fill(cAccent);
    textFont(fontMedium);
    textAlign(CENTER, TOP);
    text("데이터 수집 중", LCD_WIDTH / 2, modalY + 10);

    // 상태 텍스트
    fill(cText);
    textFont(fontSmall);
    textAlign(CENTER, CENTER);

    String statusStr;
    if (passiveStabilizing)
    {
        statusStr = "센서 안정화 중...";
    }
    else
    {
        statusStr = "샘플 수집 중 (" + passiveSampleIndex + "/" + passiveSamples.length + ")";
    }
    text(statusStr, LCD_WIDTH / 2, modalY + 35);

    // 카운트다운
    fill(cGreen);
    textFont(fontLarge);
    text(passiveMeasureCountdown + "초 후 완료", LCD_WIDTH / 2, modalY + 58);

    // 프로그레스 바
    int barX = modalX + 15;
    int barY = modalY + modalH - 12;
    int barW = modalW - 30;
    int barH = 6;

    fill(cButton);
    rect(barX, barY, barW, barH, 3);

    float progress = 1.0 - (float)passiveMeasureCountdown / passiveTotalTime;
    fill(cAccent);
    rect(barX, barY, barW * progress, barH, 3);
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

    // Sleep Mode Countdown (Active 모드에서 Sleep 중일 때)
    if (isActiveMode && isSleeping)
    {
        fill(cGreen);
        textFont(fontTiny);
        textAlign(CENTER, CENTER);

        String countdownStr;
        if (inSleepCycle)
        {
            countdownStr = "S:" + sleepCountdown + "s";
        }
        else
        {
            countdownStr = "A:" + activeCountdown + "s";
        }
        // 타이틀바 중앙에 표시 (Air Monitor와 시간 사이)
        text(countdownStr, LCD_WIDTH / 2, TITLE_BAR_H / 2);
    }

    // Passive 모드일 때 측정 버튼 (메인 화면에서만)
    if (!isActiveMode && !isPassiveMeasuring && currentScreen == 0)
    {
        int btnW = 40;
        int btnH = 16;
        int btnX = LCD_WIDTH / 2 - btnW / 2;
        int btnY = (TITLE_BAR_H - btnH) / 2;

        fill(cBlue);
        rect(btnX, btnY, btnW, btnH, 3);

        fill(cText);
        textFont(fontTiny);
        textAlign(CENTER, CENTER);
        text("측정", LCD_WIDTH / 2, TITLE_BAR_H / 2);
    }

    // Mode Indicator
    if (isActiveMode)
    {
        if (isSleeping)
        {
            fill(inSleepCycle ? cYellow : cGreen);
        }
        else
        {
            fill(cGreen);
        }
        noStroke();
        ellipse(LCD_WIDTH - 70, TITLE_BAR_H / 2, 8, 8);
    }
    else
    {
        fill(cBlue);
        pushMatrix();
        translate(LCD_WIDTH - 70, TITLE_BAR_H / 2);
        rotate(PI / 4);
        rectMode(CENTER);
        noStroke();
        rect(0, 0, 6, 6);
        rectMode(CORNER);
        popMatrix();
    }

    // Current Time
    fill(cText);
    textFont(fontTiny);
    textAlign(RIGHT, CENTER);
    String timeStr = nf(hours, 2) + ":" + nf(minutes, 2) + ":" + nf(seconds, 2);
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

        fill(currentScreen == i ? cAccent : cText);
        textFont(fontSmall);
        textAlign(CENTER, CENTER);
        String[] labels = {"메인", "설정", "그래프"};
        text(labels[i], i * tabW + tabW / 2, y + TAB_BAR_H / 2);

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
    // Active 모드일 때는 비활성화 (isSleeping일 때만 활성화)
    boolean sleepTimeEnabled = isActiveMode && isSleeping;

    fill(sleepTimeEnabled ? cText : cTextDim);
    textFont(fontSmall);
    textAlign(LEFT, TOP);
    text("Sleep 시간 설정" + (sleepTimeEnabled ? "" : " (Sleep 모드에서만)"), 10, y);

    int btnY = y + 14;
    int[] times = {1, 3, 5, 10, 20};
    int btnW = 55;
    int gap = 4;

    for (int i = 0; i < times.length; i++)
    {
        int bx = 10 + i * (btnW + gap);
        boolean selected = (sleepTimeOption == times[i]);

        if (!sleepTimeEnabled)
        {
            // 비활성화 상태
            fill(cDisabled);
            rect(bx, btnY, btnW, btnH, 3);
            fill(cTextDim);
        }
        else
        {
            fill(selected ? cAccent : cButton);
            rect(bx, btnY, btnW, btnH, 3);
            fill(selected ? cBackground : cText);
        }

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

    String statusText;
    if (isActiveMode)
    {
        if (isSleeping)
        {
            if (inSleepCycle)
            {
                statusText = "Sleep 중 (" + sleepCountdown + "초 남음)";
            }
            else
            {
                statusText = "Active 측정 중 (" + activeCountdown + "초 남음)";
            }
        }
        else
        {
            statusText = "Active 연속 측정중";
        }
    }
    else
    {
        statusText = "Passive 대기중";
    }

    text("상태: " + statusText, 10, statusY);
    text("Sleep: " + sleepTimeOption + "분 / Active: " + activeDuration + "초", 10, statusY + 14);
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
    int graphX = 25;
    int graphY = statsY + statsH + 5;
    int graphW = LCD_WIDTH - graphX - 5;
    int graphH = CONTENT_H - (graphY - TITLE_BAR_H) - 5;

    // 동적 Y축 범위 계산
    int yMax = 50;
    if (pm25Max > 0)
    {
        yMax = ((pm25Max / 25) + 1) * 25;
        yMax = max(yMax, 50);
        yMax = min(yMax, 200);
    }
    int yMid = yMax / 2;

    // Graph Background
    fill(0, 50);
    noStroke();
    rect(graphX, graphY, graphW, graphH);

    // Grid lines
    stroke(cPanel);
    strokeWeight(1);
    for (int i = 1; i < 4; i++)
    {
        int lineY = graphY + (graphH * i / 4);
        line(graphX, lineY, graphX + graphW, lineY);
    }

    // Y-axis labels
    fill(cTextDim);
    textFont(fontSmall);
    textAlign(RIGHT, CENTER);
    text(str(yMax), graphX - 3, graphY + 5);
    text(str(yMid), graphX - 3, graphY + graphH / 2);
    text("0", graphX - 3, graphY + graphH - 5);

    // Threshold lines
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
            py = constrain(py, graphY, graphY + graphH);
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
    // Passive 모드에서는 자동 갱신 안함 (측정 버튼으로만 갱신)
    if (!isActiveMode)
        return;

    // Sleep 사이클 중 inSleepCycle이면 데이터 업데이트 안함
    if (isActiveMode && isSleeping && inSleepCycle)
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
    // 모달이 떠있으면 클릭 무시
    if (isPassiveMeasuring)
        return;

    int mx = mouseX / SCALE;
    int my = mouseY / SCALE;

    // Passive 모드 측정 버튼 클릭
    if (!isActiveMode && my < TITLE_BAR_H)
    {
        int btnW = 40;
        int btnX = LCD_WIDTH / 2 - btnW / 2;
        if (mx >= btnX && mx < btnX + btnW)
        {
            startPassiveMeasurement();
            return;
        }
    }

    // Tab Bar Click
    int tabY = TITLE_BAR_H + CONTENT_H;
    if (my >= tabY && my < tabY + TAB_BAR_H)
    {
        int tabW = LCD_WIDTH / 3;
        currentScreen = mx / tabW;
        currentScreen = constrain(currentScreen, 0, 2);
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

    // Sleep Time Buttons (Sleep 모드일 때만 활성화)
    boolean sleepTimeEnabled = isActiveMode && isSleeping;
    if (sleepTimeEnabled)
    {
        for (int i = 0; i < times.length; i++)
        {
            int bx = 10 + i * (btnW + gap);
            if (mx >= bx && mx < bx + btnW && my >= btnY && my < btnY + btnH)
            {
                sleepTimeOption = times[i];
                // Sleep 시간 변경 시 카운트다운 업데이트
                if (inSleepCycle)
                {
                    sleepCountdown = sleepTimeOption * 60;
                }
            }
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
        stopSleepMode();
    }

    // Sleep/Wake Button
    if (isActiveMode && mx >= 10 + (modeBtnW + 5) * 2 && mx < 10 + (modeBtnW + 5) * 2 + modeBtnW && my >= modeBtnY && my < modeBtnY + btnH)
    {
        if (isSleeping)
        {
            stopSleepMode();
        }
        else
        {
            startSleepMode();
        }
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
