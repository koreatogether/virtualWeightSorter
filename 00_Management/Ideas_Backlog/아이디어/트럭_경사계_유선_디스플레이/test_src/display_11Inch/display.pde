/**
 * Dump Truck Inclinometer for 11-inch Display
 * Designed for Roll/Pitch monitoring and Overturn Prevention (ISO 22138 Reference)
 * 
 * Target Resolution: 1280 x 800 (Common 11-inch aspect ratio)
 * Safe Thresholds: 7 deg (Green), 15 deg (Yellow), 25 deg (Orange/Red)
 */

float roll = 0;
float pitch = 0;
float targetRoll = 0;
float targetPitch = 0;

// Fonts
PFont fontLarge, fontMed, fontSmall;

// Colors
color clrSafe = color(0, 255, 100);    // Green
color clrWarn = color(255, 220, 0);    // Yellow
color clrDanger = color(255, 100, 0);  // Orange
color clrEmerg = color(255, 0, 0);     // Red
color clrBg = color(15, 15, 25);       // Dark Blue-Black

void setup() {
  size(1280, 800, P2D);
  frameRate(60);
  smooth(8);
  
  // Create fonts for display
  fontLarge = createFont("Arial Bold", 120);
  fontMed = createFont("Arial Bold", 48);
  fontSmall = createFont("Arial Bold", 24);
  
  textAlign(CENTER, CENTER);
}

void draw() {
  background(clrBg);
  
  // 1. Update inclination values (Simulation via Mouse)
  // Mouse X -> Roll (-30 to 30)
  // Mouse Y -> Pitch (-30 to 30)
  targetRoll = map(mouseX, 0, width, -35, 35);
  targetPitch = map(mouseY, 0, height, 35, -35); // Invert Y
  
  // Smoothing (Linear Interpolation)
  roll += (targetRoll - roll) * 0.1;
  pitch += (targetPitch - pitch) * 0.1;
  
  // 2. Determine State based on Max(abs(roll), abs(pitch))
  float maxTilt = max(abs(roll), abs(pitch));
  color currentStateColor = clrSafe;
  String stateText = "SAFE";
  
  if (maxTilt > 25) {
    currentStateColor = clrEmerg;
    stateText = "EMERGENCY: STOP OPERATING";
  } else if (maxTilt > 15) {
    currentStateColor = clrDanger;
    stateText = "DANGER: HIGH RISK";
  } else if (maxTilt > 7) {
    currentStateColor = clrWarn;
    stateText = "CAUTION: WATCH STABILITY";
  } else {
    currentStateColor = clrSafe;
    stateText = "STABLE";
  }

  // Draw Background Grid
  drawGrid();

  // 3. Main Center Display (Truck Visualization)
  pushMatrix();
  translate(width/2, height/2 - 50);
  drawTruckIcon(roll, pitch, currentStateColor);
  popMatrix();

  // 4. Roll Display (Left Side)
  drawGauge("ROLL", roll, 150, height/2, currentStateColor);
  
  // 5. Pitch Display (Right Side)
  drawGauge("PITCH", pitch, width - 150, height/2, currentStateColor);

  // 6. Threshold Indicators
  drawThresholds();

  // 7. Status UI
  drawStatusUI(stateText, currentStateColor);
  
  // Instruction
  fill(200);
  textFont(fontSmall);
  text("Move MOUSE to simulate Sensor Input", width/2, height - 30);
}

void drawGrid() {
  stroke(40, 40, 60);
  strokeWeight(1);
  for (int i = 0; i <= width; i += 100) line(i, 0, i, height);
  for (int j = 0; j <= height; j += 100) line(0, j, width, j);
}

void drawGauge(String label, float angle, float x, float y, color c) {
  // Gauge Circle
  noFill();
  stroke(60);
  strokeWeight(10);
  ellipse(x, y, 200, 200);
  
  // Active Arc
  stroke(c);
  strokeWeight(12);
  float arcEnd = map(angle, -35, 35, -PI, PI);
  arc(x, y, 200, 200, min(-HALF_PI, arcEnd-HALF_PI), max(-HALF_PI, arcEnd-HALF_PI));
  
  // Pointer
  pushMatrix();
  translate(x, y);
  rotate(radians(angle));
  stroke(255);
  strokeWeight(4);
  line(0, 0, 0, -90);
  fill(255);
  ellipse(0, -90, 8, 8);
  popMatrix();

  // Text
  fill(255);
  textFont(fontMed);
  text(nf(angle, 1, 1) + "°", x, y + 140);
  
  fill(180);
  textFont(fontSmall);
  text(label, x, y - 140);
}

void drawTruckIcon(float r, float p, color c) {
  // Simple Visualization of Truck from Rear
  pushMatrix();
  rotate(radians(r));
  
  // Road surface
  stroke(100);
  strokeWeight(4);
  line(-250, 100, 250, 100);
  
  // Truck Chassis
  fill(50);
  stroke(c);
  strokeWeight(4);
  rect(-120, 20, 240, 60, 5);
  
  // Wheels
  fill(20);
  noStroke();
  ellipse(-90, 80, 40, 40);
  ellipse(90, 80, 40, 40);
  
  // Tipper Body (Affected by Pitch - Simplified as getting larger/smaller or tilting internally)
  pushMatrix();
  float pitchScale = map(p, -35, 35, 0.8, 1.2);
  scale(pitchScale);
  fill(c, 100); // semi-transparent
  stroke(c);
  rect(-110, -110, 220, 130, 10);
  
  // Cabin
  fill(70);
  rect(-70, -40, 140, 60, 5);
  popMatrix();
  
  popMatrix();
}

void drawThresholds() {
  int[] marks = {7, 15, 25};
  color[] colors = {clrSafe, clrWarn, clrDanger};
  
  float barW = 300;
  float barX = width/2 - barW/2;
  float barY = height - 120;
  
  noStroke();
  for(int i=0; i<3; i++) {
    fill(colors[i]);
    float xPos = map(marks[i], 0, 35, 0, barW);
    rect(barX + xPos, barY, 3, 20);
    textFont(fontSmall);
    text(marks[i]+"°", barX + xPos, barY + 40);
  }
}

void drawStatusUI(String msg, color c) {
  // Top Banner
  fill(c);
  rect(0, 0, width, 80);
  
  fill(0);
  textFont(fontMed);
  text(msg, width/2, 40);
  
  // Warning Flash if Emergency
  if (c == clrEmerg && (frameCount / 15) % 2 == 0) {
    fill(255, 0, 0, 100);
    rect(0, 0, width, height);
  }
}
