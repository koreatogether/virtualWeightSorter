/**
 * @title TamaDoggy - Main Application
 * @author Stavros Kalapothas - Pervasive Computing
 * based on code from : http://www.andrew.cmu.edu/user/dboehle/tamagotchi
 */
 
PImage[] shells;
PImage bground;
int curShell, shellLag;

Doggy doggy;
int dogHealth;


void setup()
{
  shells = new PImage[5];
  shells[0] = loadImage("back.png");
  shells[1] = loadImage("idle.png");
  shells[2] = loadImage("right.png");
  shells[3] = loadImage("left.png");
  shells[4] = loadImage("backCover.png");
  curShell = 0;
  shellLag = 0;
  
  bground = loadImage("background.png");
  
  doggy = new Doggy();
  dogHealth = 2;
  
  size(526, 640);
  frameRate(30);
  noStroke();
}

void draw()
{
  // Draw background
  image(shells[0], 0, 0);
  displayBars(doggy.getHealth(), doggy.getFood());
  image(shells[4], 0, 0);
  image(bground, 143, 200, bground.width*2, bground.height*2);
  
  if(shellLag > 0)
    shellLag--;
  else
  {
    if(curShell!=1)
      curShell = 1;
  }
  doggy.update(dogHealth);
  image(shells[curShell], 0, 0);
}

void mousePressed()
{
  println(mouseX + ", " + mouseY);
  if (mouseX > 200) {
  curShell = 2;
        if( doggy.getState()==0 )
        {
          if( dogHealth > 0 )
          {
            doggy.setState(1);
          }
          else
          {
            doggy.setState(0);
          }
        }
  }
  if (mouseX < 200) {
  curShell = 3;
        if( doggy.getState()==0 )
        {
          if( dogHealth > 0 )
          {
            doggy.setState(2);
          }
          else
          {
            doggy.setState(0);
          }
        }
  }
  shellLag = 3;
}

void displayBars(float a, float b)
{
  fill(205, 00, 00);
  rect(149, 150, (a/100)*(260-149), 20);
  
  fill(40, 163, 69);
  rect(272, 157, (b/100)*(374-272), 4);
  fill(90, 52, 16);
  rect(272, 161, (b/100)*(374-272), 8);
}