/**
 * @title animation class
 * @author Stavros Kalapothas - Pervasive Computing
 * based on code from : http://www.andrew.cmu.edu/user/dboehle/tamagotchi
 */
 
class Animation
{
  
  PImage[] frames;
  String[] info;
  int[] pauseTimes;
  int animCount;
  int pauseCount;
  int nFrames;
  String name;
  int x, y, w, h;
  
  Animation(String Name, int X, int Y, int magnification)
  {
    name = Name;
    x = X;
    y = Y;
    
    info = loadStrings(name + "/" + name + "Info.txt");
    nFrames = info.length;
    
    pauseTimes = new int[nFrames];
    for(int i=0; i<nFrames; i++)
    {
      pauseTimes[i] = Integer.parseInt(info[i]);
    }
    
    frames = new PImage[nFrames];
    for(int i=0; i < nFrames; i++)
    {
      frames[i] = loadImage(name + "/" + (i+1) + ".png");
    }
    
    w = frames[0].width * magnification;
    h = frames[0].height * magnification;
    
    animCount = 0;
    pauseCount = 0;
  }
  
  boolean canDisplay()
  {
    return (animCount != -1);
  }
  
  void reset(int frameNum)
  {
    animCount = frameNum;
    pauseCount = 0;
  }
  
  int getAnimCount()
  {
    return animCount;
  }
  
  void display(int X, int Y)
  {
    x = X;
    y = Y;
    display();
  }
  
  void display()
  {
    image(frames[animCount], x, y, w, h);
    
    if(pauseCount == pauseTimes[animCount])
    {
      pauseCount = -20;
      animCount++;
    }
    else
    {
      pauseCount++;
    }
    
    if(animCount == (nFrames))
      animCount = -1;
  }
}