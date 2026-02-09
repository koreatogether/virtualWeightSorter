/**
 * @title Doggy class
 * @author Stavros Kalapothas - Pervasive Computing
 * based on code from : http://www.andrew.cmu.edu/user/dboehle/tamagotchi
 */
 
class Doggy
{
  Animation idle;    // state 0
  Animation eat;     // state 1
  Animation sleep;   // state 2
  
  int state;
  
  float health;
  float food;
  
  Doggy()
  {
    idle = new Animation("idle", 200, 300, 3);
    eat = new Animation("eat", 151, 212, 4);
    sleep = new Animation("sleep",200, 300, 3);
     
    health = 100;
    food = 100;
    
    state = 0;
  }
  
  void setState(int newState)
  {    
    state = newState;
    idle.reset(0);
    eat.reset(0);
    sleep.reset(0);
  }
  
  int getState()
  {
    return state;
  }
  
  float getHealth()
  {
    return health;
  }
  
  void setHealth(float newHealth)
  {
    health = newHealth;
  }
  
   float getFood()
  {
    return food;
  } 
 
  void update(int dogHealth)
  {
    if(dogHealth==0)
      food -= 0.05;
    else
      food += 0.1;
    
    health += 0.03;    
    
    if(health > 100)
          health = 100;
    
    if(food > 100)
          food = 100;
      
    render();
  }
  
 

  void render()
  {
    switch(state)
    {
      case 0:
      {
        if(idle.canDisplay())
          idle.display();
        else
        {
          idle.reset(0);
          idle.display();
        }
        break;
      }
      case 1:
      {
        if(eat.canDisplay())
          eat.display();
        else
        {
          eat.reset(0);
          state = 0;
          idle.reset(0);
          idle.display();
        }
        break;
      }
      case 2:
      {
        if(sleep.canDisplay())
          sleep.display();
        else
        {
          sleep.reset(0);
          state = 0;
          idle.reset(0);
          idle.display();
        }
        break;
      } 
     default:
      break;
    }
  }
  
}