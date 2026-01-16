/**
 * Dump Truck Inclinometer (Two-Panel UI) for 11-inch Display
 *
 * Left panel : SIDE view (Pitch)
 * Right panel: REAR view (Roll)
 *
 * Notes
 * - Input is currently simulated via mouse.
 * - Threshold reference (example): 7° safe, 15° caution, 25° danger/emergency.
 */

// Current (smoothed) values
float roll = 0;
float pitch = 0;

// Target values (from input)
float targetRoll = 0;
float targetPitch = 0;

// UI constants
final float MAX_TILT_DEG = 35;
final int TOP_BAR_H = 86;
final int BOTTOM_HINT_H = 44;
final int OUTER_MARGIN = 24;
final int PANEL_GUTTER = 18;

// Fonts
PFont fontLarge;
PFont fontMed;
PFont fontSmall;

// Colors
color clrSafe = color(0, 255, 110);
color clrWarn = color(255, 220, 0);
color clrDanger = color(255, 120, 0);
color clrEmerg = color(255, 30, 30);
color clrBg = color(15, 15, 25);
color clrPanel = color(22, 22, 34);
color clrPanelEdge = color(55, 55, 80);

// Images
PImage imgSide;
PImage imgRear;

final String SIDE_IMAGE_REL = "images/Gemini_Generated_Image_jkwx05jkwx05jkwx.png";
final String SIDE_IMAGE_ABS = "E:/arduino/02_docs/아이디어/트럭_경사계_유선_디스플레이/test_src/display_11Inch/images/Gemini_Generated_Image_jkwx05jkwx05jkwx.png";

final String REAR_IMAGE_REL = "images/Gemini_Generated_Image_ibtfvibtfvibtfvi.png";
final String REAR_IMAGE_ABS = "E:/arduino/02_docs/아이디어/트럭_경사계_유선_디스플레이/test_src/display_11Inch/images/Gemini_Generated_Image_ibtfvibtfvibtfvi.png";

void setup()
{
    size(1280, 800, P2D);
    frameRate(60);
    smooth(8);

    fontLarge = createFont("Arial Bold", 96);
    fontMed = createFont("Arial Bold", 44);
    fontSmall = createFont("Arial Bold", 22);
    textAlign(CENTER, CENTER);

    // Load images
    imgSide = loadImageRobust(SIDE_IMAGE_REL, SIDE_IMAGE_ABS);
    imgRear = loadImageRobust(REAR_IMAGE_REL, REAR_IMAGE_ABS);
}

PImage loadImageRobust(String relPath, String absFallback)
{
    // Processing의 loadImage()는 기본적으로 스케치의 data/ 폴더를 기준으로 찾는 경우가 많아서
    // 1) relPath (data/ 기준)
    // 2) sketchPath(relPath) (스케치 폴더 기준)
    // 3) absFallback (절대경로)
    // 순서로 시도합니다.
    PImage img = null;

    println("[IMG] try rel: " + relPath);
    img = loadImage(relPath);

    if (img == null)
    {
        String p = sketchPath(relPath);
        println("[IMG] try sketchPath: " + p);
        img = loadImage(p);
    }

    if (img == null && absFallback != null && absFallback.length() > 0)
    {
        println("[IMG] try abs: " + absFallback);
        img = loadImage(absFallback);
    }

    if (img == null)
    {
        println("[IMG] FAILED to load side image.");
    }
    else
    {
        println("[IMG] loaded: " + img.width + "x" + img.height);
    }

    return img;
}

void draw()
{
    background(clrBg);

    // Input (simulation)
    targetRoll = map(mouseX, 0, width, -MAX_TILT_DEG, MAX_TILT_DEG);
    targetPitch = map(mouseY, 0, height, MAX_TILT_DEG, -MAX_TILT_DEG);

    // Smoothing
    roll += (targetRoll - roll) * 0.12;
    pitch += (targetPitch - pitch) * 0.12;

    float maxTilt = max(abs(roll), abs(pitch));
    color stateColor = stateColorFor(maxTilt);
    String stateText = stateTextFor(maxTilt);

    drawTopBar(stateText, stateColor);

    // Layout
    int usableH = height - TOP_BAR_H - BOTTOM_HINT_H - OUTER_MARGIN * 2;
    int panelY = TOP_BAR_H + OUTER_MARGIN;
    int panelW = (width - OUTER_MARGIN * 2 - PANEL_GUTTER) / 2;
    int panelH = usableH;
    int leftX = OUTER_MARGIN;
    int rightX = OUTER_MARGIN + panelW + PANEL_GUTTER;

    drawPanel(leftX, panelY, panelW, panelH, "SIDE (PITCH)", pitch, "PITCH", stateColor, true);
    drawPanel(rightX, panelY, panelW, panelH, "REAR (ROLL)", roll, "ROLL", stateColor, false);

    drawBottomHint();

    // Emergency flash overlay
    if (stateColor == clrEmerg && (frameCount / 14) % 2 == 0)
    {
        noStroke();
        fill(255, 0, 0, 55);
        rect(0, 0, width, height);
    }
}

color stateColorFor(float maxTilt)
{
    if (maxTilt > 25)
        return clrEmerg;
    if (maxTilt > 15)
        return clrDanger;
    if (maxTilt > 7)
        return clrWarn;
    return clrSafe;
}

String stateTextFor(float maxTilt)
{
    if (maxTilt > 25)
        return "EMERGENCY: STOP";
    if (maxTilt > 15)
        return "DANGER";
    if (maxTilt > 7)
        return "CAUTION";
    return "STABLE";
}

void drawTopBar(String stateText, color c)
{
    noStroke();
    fill(c);
    rect(0, 0, width, TOP_BAR_H);

    fill(0);
    textFont(fontMed);
    text(stateText, width / 2, TOP_BAR_H / 2);

    // Small telemetry
    fill(0);
    textFont(fontSmall);
    text("ROLL  " + formatDeg(roll) + "    PITCH  " + formatDeg(pitch), width / 2, TOP_BAR_H - 18);
}

void drawBottomHint()
{
    fill(210);
    textFont(fontSmall);
    text("Mouse X=Roll, Mouse Y=Pitch (simulated)", width / 2, height - BOTTOM_HINT_H / 2);
}

String formatDeg(float v)
{
    String sign = (v > 0.05) ? "+" : "";
    return sign + nf(v, 1, 1) + "°";
}

void drawPanel(int x, int y, int w, int h, String title, float angleDeg, String axisLabel, color accent, boolean isSideView)
{
    // Panel background
    noStroke();
    fill(clrPanel);
    rect(x, y, w, h, 14);
    stroke(clrPanelEdge);
    strokeWeight(2);
    noFill();
    rect(x + 1, y + 1, w - 2, h - 2, 14);

    // Title
    fill(220);
    textFont(fontSmall);
    text(title, x + w / 2, y + 26);

    // Big number
    fill(255);
    textFont(fontLarge);
    text(formatDeg(angleDeg), x + w / 2, y + 95);

    fill(180);
    textFont(fontSmall);
    text(axisLabel, x + w / 2, y + 150);

    // Content region
    int contentTop = y + 170;
    int contentH = h - (contentTop - y) - 18;

    // Scale on the far right inside panel
    int scaleW = 120;
    int scaleX = x + w - scaleW - 18;
    int scaleY = contentTop + 10;
    int scaleH = contentH - 20;
    drawVerticalScale(scaleX, scaleY, scaleW, scaleH, angleDeg, accent);

    // Truck drawing area
    int truckX = x + 18;
    int truckY = contentTop + 10;
    int truckW = w - 18 - scaleW - 18 - 12;
    int truckH = contentH - 20;

    pushStyle();
    clip(truckX, truckY, truckW, truckH);
    drawTruckScene(truckX, truckY, truckW, truckH, angleDeg, accent, isSideView);
    noClip();
    popStyle();

    // Threshold legend (small)
    drawThresholdLegend(x + 18, y + h - 30);
}

void drawThresholdLegend(int x, int y)
{
    textFont(fontSmall);
    textAlign(LEFT, CENTER);

    int dot = 10;
    int gap = 12;

    noStroke();
    fill(clrSafe);
    rect(x, y - dot / 2, dot, dot, 3);
    fill(200);
    text("≤ 7°", x + dot + 8, y);

    int x2 = x + 70;
    fill(clrWarn);
    rect(x2, y - dot / 2, dot, dot, 3);
    fill(200);
    text("≤ 15°", x2 + dot + 8, y);

    int x3 = x + 155;
    fill(clrDanger);
    rect(x3, y - dot / 2, dot, dot, 3);
    fill(200);
    text("≤ 25°", x3 + dot + 8, y);

    int x4 = x + 240;
    fill(clrEmerg);
    rect(x4, y - dot / 2, dot, dot, 3);
    fill(200);
    text("> 25°", x4 + dot + 8, y);

    textAlign(CENTER, CENTER);
}

void drawVerticalScale(int x, int y, int w, int h, float valueDeg, color accent)
{
    // Scale line
    float lineX = x + w * 0.55;
    float topY = y;
    float bottomY = y + h;

    stroke(95);
    strokeWeight(3);
    line(lineX, topY, lineX, bottomY);

    // Tick marks (-30..30)
    textAlign(LEFT, CENTER);
    textFont(fontSmall);

    for (int deg = -30; deg <= 30; deg += 5)
    {
        float yy = map(deg, MAX_TILT_DEG, -MAX_TILT_DEG, topY, bottomY);
        boolean major = (deg % 10 == 0);

        stroke(major ? 170 : 120);
        strokeWeight(major ? 2 : 1);
        float tickLen = major ? 18 : 10;
        line(lineX - tickLen, yy, lineX, yy);

        if (major)
        {
            noStroke();
            fill(190);
            text(deg + "°", x + 6, yy);
        }
    }

    // Inverted triangle marker (역삼각형)
    float markerY = constrain(map(valueDeg, MAX_TILT_DEG, -MAX_TILT_DEG, topY, bottomY), topY, bottomY);
    noStroke();
    fill(accent);
    float triW = 22;
    float triH = 18;
    triangle(lineX - triW / 2, markerY - triH, lineX + triW / 2, markerY - triH, lineX, markerY + 2);

    // Marker value
    fill(255);
    textAlign(CENTER, CENTER);
    textFont(fontSmall);
    text(nf(valueDeg, 1, 1) + "°", lineX, markerY + 22);
}

void drawTruckScene(int x, int y, int w, int h, float angleDeg, color accent, boolean isSideView)
{
    // Background subtle grid
    stroke(35, 35, 55);
    strokeWeight(1);
    for (int i = 0; i <= w; i += 80)
        line(x + i, y, x + i, y + h);
    for (int j = 0; j <= h; j += 80)
        line(x, y + j, x + w, y + j);

    // Road line (fixed)
    float roadY = y + h * 0.78;
    stroke(120);
    strokeWeight(4);
    line(x + w * 0.08, roadY, x + w * 0.92, roadY);

    // Truck drawing (rotates)
    pushMatrix();
    float cx = x + w * 0.50;
    float cy = y + h * 0.62;
    translate(cx, cy);
    rotate(radians(angleDeg));

    if (isSideView)
    {
        if (imgSide != null)
        {
            imageMode(CENTER);
            image(imgSide, 0, 0, w * 0.9, h * 0.6); // Adjust scale as needed
        }
        else
        {
            drawTruckSide(0, 0, w * 0.72, h * 0.45, accent);

            // Debug hint on-screen when image is missing
            resetMatrix();
            fill(255, 80, 80);
            textFont(fontSmall);
            textAlign(LEFT, CENTER);
            text("SIDE PNG not loaded (check data/ or images/)", x + 10, y + h - 14);
            textAlign(CENTER, CENTER);
        }
    }
    else
    {
        if (imgRear != null)
        {
            imageMode(CENTER);
            image(imgRear, 0, 0, w * 0.85, h * 0.65); // Adjust scale as needed
        }
        else
        {
            drawTruckRear(0, 0, w * 0.60, h * 0.48, accent);

            // Debug hint on-screen when image is missing
            resetMatrix();
            fill(255, 80, 80);
            textFont(fontSmall);
            textAlign(LEFT, CENTER);
            text("REAR PNG not loaded (check data/ or images/)", x + 10, y + h - 14);
            textAlign(CENTER, CENTER);
        }
    }
    popMatrix();

    // Caption inside scene
    fill(200);
    textFont(fontSmall);
    textAlign(CENTER, CENTER);
    text(isSideView ? "SIDE VIEW" : "REAR VIEW", x + w / 2, y + 20);
    textAlign(CENTER, CENTER);
}

void drawTruckSide(float ox, float oy, float w, float h, color accent)
{
    float bodyW = w;
    float bodyH = h * 0.28;
    float wheelR = h * 0.18;

    // Wheels
    noStroke();
    fill(15);
    ellipse(ox - bodyW * 0.30, oy + bodyH * 0.70, wheelR, wheelR);
    ellipse(ox + bodyW * 0.30, oy + bodyH * 0.70, wheelR, wheelR);

    // Chassis
    stroke(accent);
    strokeWeight(4);
    fill(55);
    rectMode(CENTER);
    rect(ox, oy + bodyH * 0.25, bodyW * 0.92, bodyH, 10);

    // Cabin (front)
    fill(70);
    rect(ox + bodyW * 0.23, oy - bodyH * 0.15, bodyW * 0.28, bodyH * 0.85, 10);
    fill(90);
    rect(ox + bodyW * 0.27, oy - bodyH * 0.22, bodyW * 0.18, bodyH * 0.32, 8);

    // Bed (rear)
    fill(50, 50, 65);
    stroke(accent);
    rect(ox - bodyW * 0.16, oy - bodyH * 0.12, bodyW * 0.56, bodyH * 0.70, 10);

    // Accent outline
    noFill();
    stroke(accent);
    strokeWeight(3);
    rect(ox, oy + bodyH * 0.25, bodyW * 0.92, bodyH, 10);
}

void drawTruckRear(float ox, float oy, float w, float h, color accent)
{
    float bodyW = w;
    float bodyH = h * 0.46;
    float wheelW = w * 0.18;
    float wheelH = h * 0.22;

    // Wheels
    noStroke();
    fill(15);
    rectMode(CENTER);
    rect(ox - bodyW * 0.38, oy + bodyH * 0.35, wheelW, wheelH, 10);
    rect(ox + bodyW * 0.38, oy + bodyH * 0.35, wheelW, wheelH, 10);

    // Frame
    stroke(accent);
    strokeWeight(4);
    fill(55);
    rect(ox, oy + bodyH * 0.22, bodyW * 0.92, bodyH * 0.55, 12);

    // Bed
    fill(50, 50, 65);
    rect(ox, oy - bodyH * 0.15, bodyW * 0.80, bodyH * 0.65, 12);

    // Cabin window
    fill(80);
    rect(ox, oy + bodyH * 0.02, bodyW * 0.34, bodyH * 0.22, 10);

    // Center line
    stroke(150);
    strokeWeight(2);
    line(ox, oy - bodyH * 0.45, ox, oy + bodyH * 0.55);
}
