

#include "FEHLCD.h"
#include "FEHUtility.h"
#include "FEHImages.h"
#include <vector>
#include <string>
#include <cmath>
#include <iostream>




//320 by 240


// Function Prototypes
void menuButtons();
void stats();
void play();
void instructions();
void credits();
bool backButton();
void victoryScreen();


int dropletpts = 0;


// Global Variables
#define halfWidth 160  // Example width
#define halfHeight 120 // Example height
int currentState = 0; // 0: Main Menu, 1: Stats, 2: Play, 3: Instructions, 4: Credits
float velocity = 0, px, py;



class Terrain {
public:
   float x, y;   // top left
   float width, height;
   int color;




   Terrain(float posX, float posY, float w, float h, int col)
       : x(posX), y(posY), width(w), height(h), color(col){}




   // Check if the terrain intersects with another object (e.g., player)
   bool intersects(float px, float py, float pw, float ph) {
       return px < x + width && px + pw > x &&
              py < y + height && py + ph > y;
   }
   void draw() const {
       LCD.SetFontColor(color);
       LCD.FillRectangle(x,y,width,height);
   }
};
/*
class gameState {
   private:
   int level, condition;
   bool finish = false;


   public:
   bool gameOver() {
       finish = true;


       LCD.Clear();


       LCD.Write("You Died");




       drawButton(50, 50, 50, 25, WHITE);




       if (isButtonPressed(x, y, 50, 50, 50, 25))
  return x >= btnX && x <= btnX + btnWidth && y >= btnY && y <= btnY + btnHeight;



   }





};*/


class WaterDrop {
private:
   int x;       // x-coordinate of the droplet
   int y;       // y-coordinate of the droplet
   int radius;  // Radius of the droplet
   bool visible; // Visibility flag


public:
   // Constructor to initialize the droplet
   WaterDrop(int initX, int initY, int initRadius) {
       x = initX;
       y = initY;
       radius = initRadius;
       visible = true; // Initially visible
   }


   // Method to draw the droplet
   void draw() {
       if (visible) {
           FEHImage water("waterDroplet1.png");
           water.Draw(x,y);
       }
   }
  
   // Method to detect collision with EarthBoi
   bool detectCollision(int earthBoiX, int earthBoiY, int earthBoiRadius, int waterX, int waterY) {
       if (!visible) return false; // skip collision check if not visible


       // calculate distance between EarthBoi and the droplet
       int dx = earthBoiX - x;
       int dy = earthBoiY - y;
       int distanceSquared = dx * dx + dy * dy;


       // check for collision (distance < sum of radii)
       int combinedRadius = radius + earthBoiRadius;
       if (distanceSquared <= combinedRadius * combinedRadius) {
           visible = false; // Make the droplet disappear
        
           return true;     // Collision detected
       }
       return false;
   }


   // Getter for visibility
   bool isVisible() {
       return visible;
   }
};


class Timer {
   private:


   float startTime;




   public:
   Timer() : startTime(TimeNow()) {} // Set start time during object construction




   int getElapsed() const {
       return TimeNow() - startTime;
   }




   void displayTimer() {
       LCD.SetFontColor(0x352100);
       LCD.FillRectangle(119,10, 95, 20);




       LCD.SetFontColor(WHITE);
       LCD.WriteAt(TimeNow()-startTime, 120, 10);
   }
};
void drawTerrain(const std::vector<Terrain>& terrain) {
   for (size_t i = 0; i < terrain.size(); ++i) {
       std::cout<<terrain[i].x,terrain[i].y,terrain[i].width, terrain[i].height;
       terrain[i].draw(); // Call the draw method for each terrain object
   }
}

class Button {
public:
    float x, y;        // top-left corner
    float width, height;
    FEHImage image;    // Button image
    std::string action; // e.g., "PLAY", "STATS"

    Button(float posX, float posY, float w, float h, const std::string &imgFile, const std::string &act)
        : x(posX), y(posY), width(w), height(h), action(act) {
            //imgFile is std::string
            //.c_str() converts the std::string to const char*
        image.Open(imgFile.c_str());
    }

    void draw() {
        image.Draw(x, y);
    }

    //Checks whether a touch is inside the button’s rectangular area
    bool isPressed(float touchX, float touchY) {
        return touchX >= x && touchX <= x + width &&
               touchY >= y && touchY <= y + height;
    }
};

class Menu {
public:
    std::vector<Button> buttons;

    // add a button
    void addButton(Button b) {
        buttons.push_back(b);
    }

    // Draws all buttons
    void draw() {
        // Can do "Button &b : buttons" as well
        for (auto &b : buttons) {
            b.draw();
        }
        //??
        LCD.Update();
    }

    // Check if a button was pressed, returns the action string
    // loops through all buttons in the menu
    std::string checkTouch(float touchX, float touchY) {
        for (auto &b : buttons) {
            if (b.isPressed(touchX, touchY)) {
                return b.action;
            }
        }
        return "";
    }
};



class EarthBoi {
public:
   float x, y, width = 23, height = 25;  // Position and size
   float velocityX, velocityY;  // Velocity in x and y directions
   float gravity;               // Gravity strength
   bool isJumping;// Flag to check if the player is jumping

   int animationCycle = 1; //Keeps track of which animation frame to use
   // bool scanningWinBlock = 0, touchingSomething = 0;



   EarthBoi(float posX, float posY)
       : x(posX), y(posY), velocityX(0), velocityY(0), gravity(0.5), isJumping(false) {}




   void moveLeft(const std::vector<Terrain>& terrain) {
       LCD.SetFontColor(0x352100);
       LCD.FillRectangle(x,y,10,10) ;




       if (!isTouchingTerrainLeft(terrain)) {
           x--; velocityX = -5;
       }
   }
   void moveRight(const std::vector<Terrain>& terrain)  {
       LCD.SetFontColor(0x352100);
       LCD.FillRectangle(x,y,10,10) ;

       if (!isTouchingTerrainRight(terrain)) {
           x++; velocityX = 5;
       }
   }
   void jump() {
       if (!isJumping) {
           LCD.SetFontColor(0x352100);
           LCD.FillRectangle(x,y,23,25) ;
           y--;
           velocityY = -8;
           isJumping = true;
       }
   }


   // Check if the player is touching terrain in the specified directi.\on
   bool isTouchingTerrainBelow(const std::vector<Terrain>& terrain) {
       for (auto& t : terrain) {
           if (y + height == t.y && x + width > t.x && x < t.x + t.width) { // Check if player is within horizontal bounds

                return true;
           }
       }
       return false;
   }
   bool isTouchingTerrainAbove(const std::vector<Terrain>& terrain) {
       for (auto& t : terrain) {
           if (y == t.y +t.height && x + width > t.x && x < t.x + t.width) { // Check if player is within horizontal bounds
                return true;
           }
       }
       return false;
   }




   bool isTouchingTerrainLeft(const std::vector<Terrain>& terrain) {
       for (auto& t : terrain) {
           if (x == t.x + t.width && y + height > t.y && y < t.y + t.height) {
               if (t.color == RED) {

               }
               return true;
           }
       }
       return false;
   }




   bool isTouchingTerrainRight(const std::vector<Terrain>& terrain) {
       for (auto& t : terrain) {
           if (x + width == t.x && y + height > t.y && y < t.y + t.height) {
               return true;
           }
       }
       return false;
   }




   void applyPhysics() {
       if (velocityY < 9) {
           velocityY+= gravity;
       }
       if (velocityX > 0) {
           velocityX--;
       }
       else if (velocityX < 0) {
           velocityX++;
       }
   }
 
   void handleCollisions(std::vector<Terrain>& terrain) {

       // Handle vertical movement (gravity or jumping)
       if (isTouchingTerrainBelow(terrain)) {
           velocityY = 0;  // Cancel downward velocity
           isJumping = false;  // Player is no longer jumping
       }
       if (isTouchingTerrainAbove(terrain)) {
           velocityY = 1;
       }
       // Handle horizontal movement (left/right walls)
       if (isTouchingTerrainLeft(terrain)) {
           velocityX = 0;  // Stop horizontal left movement
       }
       if (isTouchingTerrainRight(terrain)) {
           velocityX = 0;  // Stop horizontal right movement
       }
   }
 
   void updatePosition(std::vector<Terrain>& terrain) {
       applyPhysics();
       handleCollisions(terrain);
       float oldx = x, oldy = y;




       if (!(isTouchingTerrainLeft(terrain) || isTouchingTerrainRight(terrain))) {
           for (int i = 0; i < std::abs(velocityX); i++) {
           handleCollisions(terrain);
         
           if (velocityX > 0) {
               x++;
           }
           else if (velocityX < 0) {
               x--;
           }
       }




       }
       if (isTouchingTerrainBelow(terrain)) {
         //  std::cout<<y<<std::endl;
         //  std::cout<<isJumping;
       }
       else {
           for (int i = 0; i <= std::abs(velocityY); i++) {
               handleCollisions(terrain);
               if (velocityY < 0) {
                   y--;
               }
               else if (velocityY > 0) {
                   y++;
               }
           }
       }




       LCD.SetFontColor(0x352100); //Replacement Color
       LCD.FillRectangle(oldx,oldy,23,25);


       //TRY TO INTEGRATE

    //    LCD.SetFontColor(GREEN);
    //    LCD.DrawRectangle(x,y,23,25);

       FEHImage EarthboyWalk1r, EarthboyWalk2r, EarthboyWalk3r, EarthboyBase;
        FEHImage EarthboyWalk1l, EarthboyWalk2l, EarthboyWalk3l;


        //Right-facing walking frames
        EarthboyWalk1r.Open("EarthboyWalk1r.png");
        EarthboyWalk2r.Open("EarthboyWalk2r.png");
        EarthboyWalk3r.Open("EarthboyWalk3r.png");
        //Left-facing walking frames
        EarthboyWalk1l.Open("EarthboyWalk1l.png");
        EarthboyWalk2l.Open("EarthboyWalk2l.png");
        EarthboyWalk3l.Open("EarthboyWalk3l.png");


        EarthboyBase.Open("EarthboyBase.png");


        if (velocityX > 0) { //Checks direction of movement to choose appropriate animation
            if (animationCycle == 1) {EarthboyWalk1r.Draw(x,y);}
            if (animationCycle == 2) {EarthboyWalk2r.Draw(x,y);}
            if (animationCycle == 3) {EarthboyWalk3r.Draw(x,y);}
        }
        else if (velocityX < 0) {
            if (animationCycle == 1) {EarthboyWalk1l.Draw(x,y);}
            if (animationCycle == 2) {EarthboyWalk2l.Draw(x,y);}
            if (animationCycle == 3) {EarthboyWalk3l.Draw(x,y);}
        }
        else {
            EarthboyBase.Draw(x,y); //If standing still, faces camera
        }


        if (animationCycle < 3) {animationCycle++;} else {animationCycle = 1;}
    }


};








class Plant {

  private:
      FEHImage deadPlant;
      FEHImage livePlant;


      //position
      int plantx = 275, planty = 92;

      //flag to track whether the plan has been harvested
      bool harvested = false;





  public:
      Plant(){
          deadPlant.Open("deadplant.png");
          livePlant.Open("liveplant.png");
      }


      // Method to start the animation
  void animate(EarthBoi &earthboi) {
          // mark harvested to prevent re-trigger while animation/state persists
          harvested = true;
      //display deadplant
      deadPlant.Draw(plantx, planty);
      LCD.Update();
      // Wait for a short period
      Sleep(1);
      //clear dead plany
      LCD.SetFontColor(0x352100);
      LCD.FillCircle(275,92,15);


      // Switch to live plant image
      livePlant.Draw(plantx, planty);
      LCD.Update();
   
      earthboi.velocityY = -50;
  }




  void reset() {
      // reset harvested flag and draw dead plant
      harvested = false;
      deadPlant.Draw(plantx, planty);
      LCD.Update();
  }

  // Try to harvest the plant - returns true if the harvest (win) was triggered
  bool tryHarvest(EarthBoi &earthBoi) {
      if (harvested) return false;
      float earthX = earthBoi.x;
      float earthY = earthBoi.y;
      float plantX = plantx;
      float plantY = planty;
    
      if (earthX + 10 >= plantX && earthX - 10 <= plantX + 30 &&
          earthY + 10 >= plantY && earthY - 10 <= plantY + 26) {
          // trigger animation and mark harvested
          animate(earthBoi);
          return true;
      }
      return false;
  }
};


// Function to check collision between earthboy and plant
void touchPlant(EarthBoi &earthBoi, Plant &psprite) {
    // Delegate to Plant::tryHarvest which will only trigger once
    if (psprite.tryHarvest(earthBoi)) {
        currentState = 5;
    }
}








//BUTTONS
void buttonUp(EarthBoi &earthBoi){
  float upx = 250, upy = 200;
  float upWidth = 20, upHeight = 20;



  LCD.SetFontColor(LIGHTBLUE);
  LCD.DrawRectangle(upx, upy, upWidth, upHeight);
  LCD.FillRectangle(upx, upy, upWidth, upHeight);
  LCD.Update();


  float x, y;
  float x_trash, y_trash;
  while (!LCD.Touch(&x,&y)) {}
  while (LCD.Touch(&x_trash,&y_trash)) {}





  // Check for collision (bounding box check)
  if (x >= upx && x <= upx + upWidth &&
      y >= upy && y <= upy + upHeight) {
      // If collision occurs, push both objects
     // earthBoi.moveUp(10);    // Move Block 5 units to the right
  }




}








void buttonRight(EarthBoi &earthBoi){
  float rightx = 270, righty = 175;
  float rightWidth = 20, rightHeight = 20;



  LCD.SetFontColor(LIGHTBLUE);
  LCD.DrawRectangle(rightx, righty, rightWidth, rightHeight);
  LCD.FillRectangle(rightx, righty, rightWidth, rightHeight);
  LCD.Update();





  float x, y;
  float x_trash, y_trash;
  while (!LCD.Touch(&x,&y)) {}
  while (LCD.Touch(&x_trash,&y_trash)) {}



  // Check for collision (bounding box check)
  if (x >= rightx && x <= rightx + rightWidth &&
      y >= righty && y <= righty + rightHeight) {
      // If collision occurs, push both objects
     // earthBoi.moveRIGHT(10);    // Move Block 5 units to the right
  }




}








void buttonLeft(EarthBoi &earthBoi){
  float leftx = 230, lefty = 175;
  float leftWidth = 20, leftHeight = 20;




  LCD.SetFontColor(LIGHTBLUE);
  LCD.DrawRectangle(leftx, lefty, leftWidth, leftHeight);
  LCD.FillRectangle(leftx, lefty, leftWidth, leftHeight);
  LCD.Update();








  float x, y;
  float x_trash, y_trash;
  while (!LCD.Touch(&x,&y)) {}
  while (LCD.Touch(&x_trash,&y_trash)) {}








  // Check for collision (bounding box check)
  if (x >= leftx && x <= leftx + leftWidth &&
      y >= lefty && y <= lefty + leftHeight) {
      // If collision occurs, push both objects
    //  earthBoi.moveLEFT(10);    // Move Block 5 units to the right
  }



}




// Generic function to draw a button
void drawButton(float x, float y, float width, float height, int color) {
  LCD.SetFontColor(color);
  LCD.DrawRectangle(x, y, width, height);
  LCD.FillRectangle(x, y, width, height);
}



// Generic function to check if a touch is within a button
bool isButtonPressed(float x, float y, float btnX, float btnY, float btnWidth, float btnHeight) {
  return x >= btnX && x <= btnX + btnWidth && y >= btnY && y <= btnY + btnHeight;
}



// Button handlers
// void handleButtonUp(EarthBoi &earthBoi) {
//  // earthBoi.moveUp(10);
// }



// void handleButtonRight(EarthBoi &earthBoi) {
//   //earthBoi.moveRIGHT(10);
// }


// void handleButtonLeft(EarthBoi &earthBoi){
// //  earthBoi.moveLEFT(10);
// }


// void handleButtonUpRight(EarthBoi &earthBoi, float land) {
//  // earthBoi.moveUPRIGHT(10, 20, land);
// }



// void handleButtonUpLeft(EarthBoi &earthBoi, float land){
//  // earthBoi.moveUPLEFT(10, 20, land);
// }


int main() {
  while (true) {
      switch (currentState) {
          case 0: // Main Menu
              menuButtons();
              break;
          case 1: // Stats
              stats();
              break;
          case 2: // Play
              play();
              break;
          case 3: // Instructions
              instructions();
              break;
          case 4: // Credits
              credits();
              break;
       case 5: // victory
              victoryScreen();
              break;
      }
  }


  return 0;
}



void menuButtons() {
    LCD.Clear();

    // Draw background and title
    FEHImage background("background.png");
    background.Draw(0, 0);
    FEHImage title("logo.png");
    title.Draw(60, 10);

    // Create menu
    Menu mainMenu;

    // posX, posY, width, height, image file, text string
    mainMenu.addButton(Button(halfWidth - 30, halfHeight, 30, 15, "play.png", "PLAY"));
    mainMenu.addButton(Button((halfWidth / 2) - 30, halfHeight + 40, 70, 15, "credit.png", "CREDITS"));
    mainMenu.addButton(Button((halfWidth + (halfWidth / 2)) - 40, halfHeight + 40, 30, 15, "stats.png", "STATS"));
    mainMenu.addButton(Button(halfWidth + 110, halfHeight + 85, 22, 20, "gear.png", "INSTRUCTIONS"));

    mainMenu.draw();

    float x, y, x_trash, y_trash;

    // Wait for touch
    while (!LCD.Touch(&x, &y)) {}
    while (LCD.Touch(&x_trash, &y_trash)) {}

    // Determine action
    std::string action = mainMenu.checkTouch(x, y);
    if (action == "PLAY") currentState = 2;
    else if (action == "STATS") currentState = 1;
    else if (action == "CREDITS") currentState = 4;
    else if (action == "INSTRUCTIONS") currentState = 3;
}


// Stats Page
void stats() {
  LCD.Clear();

  FEHImage background("background.png");
    background.Draw(0, 0);

FEHImage statsTitle("statsTitle.png");
   statsTitle.Draw(0,0);

    LCD.SetFontColor(GOLDENROD);
  LCD.WriteAt("LEVEL: 1", halfWidth - 70, halfHeight-20);
  LCD.WriteAt("DROPS: ", halfWidth - 70, halfHeight+10);
  LCD.WriteAt(dropletpts, halfWidth+15, halfHeight+10);

  if (backButton()) {
      currentState = 0; // Return to Main Menu
  }
}



// Instructions Page
void instructions() {
  LCD.Clear();

  FEHImage background("background.png");
    background.Draw(0, 0);
 
   FEHImage instruc("instruction.png");
   instruc.Draw(0,0);

   LCD.SetFontColor(GOLDENROD);
  if (backButton()) {
      currentState = 0; // Return to Main Menu
  }
}

// Credits Page
void credits() {
  LCD.Clear();

  FEHImage background("background.png");
background.Draw(0, 0);

    FEHImage creditsTitle("creditsTitle.png");
    creditsTitle.Draw(0,0);

    LCD.SetFontColor(GOLDENROD);
  if (backButton()) {
      currentState = 0; // Return to Main Menu
  }
}




// Function to display the victory screen
void victoryScreen() {
   LCD.Clear();


   FEHImage victory("WINNER.png");
   victory.Draw(0,0);

if (backButton()) {
      currentState = 0; // Return to Main Menu
  }


//    LCD.Update();


//    float x, y;
//    float x_trash, y_trash;


//    // Wait for touch and check if it's on the back button
//    while (!LCD.Touch(&x, &y)) {}
//    while (LCD.Touch(&x_trash, &y_trash)) {}


//    if (x > (halfWidth - 50) && x < (halfWidth + 50) &&
//        y > (halfHeight + 30) && y < (halfHeight + 60)) {
//        currentState = 0; // Return to main menu
//    }
}


//PLAY page
void play(){


   Timer gameTimer;

   std::vector<Terrain> terrain;
       //back border
       terrain.push_back(Terrain(0,0,319,239, 0x7E481C)),
       //front border
       terrain.push_back(Terrain(4,4,312,232, 0x352100)),
       //Terrain(4,4,312,232, 0x352100),
       //Bottom w/ Wall
       terrain.push_back(Terrain(7,182,127,28, TAN)),
       //Terrain(7,182,127,28, TAN),
       //Left bound
       terrain.push_back(Terrain(5,5,2,232, TAN)),
       //Terrain(5,5,2,232, TAN),
       //Right bound
       terrain.push_back(Terrain(313,5,2,232, TAN)),
       //Terrain(313,5,2,232, TAN),
       //Ceiling
       terrain.push_back(Terrain(0,0,318,5,TAN)),
       //Terrain(0,0,318,5,TAN),
       //Wall
       terrain.push_back(Terrain(134,134,26,76, TAN)),
       //Terrain(134,134,26,76, TAN),
       terrain.push_back(Terrain(160,182,150,28, TAN)),
       //Terrain(160,182,150,28, TAN),
       terrain.push_back(Terrain(202,63,26,91, TAN)),
       //Terrain(202,63,26,91, TAN),
       terrain.push_back(Terrain(82,134,120,20,TAN)),
       //Terrain(82,134,120,20,TAN),
       //Misc
       terrain.push_back(Terrain(7,63,102,20, TAN)),
       //Terrain(7,63,102,20, TAN),
       terrain.push_back(Terrain(144,63,101,20,TAN)),
       //Terrain(144,63,101,20,TAN),
       terrain.push_back(Terrain(270,112,42,15, TAN)),
       //Terrain(270,112,42,15, TAN),

 
   drawTerrain(terrain);


  //testing if STATS button is touched
      LCD.Clear();


      //top w/ gap
      LCD.DrawRectangle(7,63,102,20);
      LCD.FillRectangle(7,63,102,20);

      LCD.DrawRectangle(144,63,101,20);
      LCD.FillRectangle(144,63,101,20);



      LCD.DrawLine(270,112,312,112);//final ledge
      LCD.DrawRectangle(270,112,42,15);
      LCD.FillRectangle(270,112,42,15);




      //time();
  
      LCD.Update();


 
  //ARROW KEYS CODE



  // Define button properties
  float upX = 250, upY = 175, btnSize = 20;
  float rightX = 270, rightY = 200;
  float leftX = 230, leftY = 200;
  float topRightX = 270, topRightY = 175; // Diagonal Top-Right
  float topLeftX = 230, topLeftY = 175;   // Diagonal Top-Left




   drawTerrain(terrain);


   EarthBoi earthboi(50,25); // Initialize EarthBoi
   Plant plantsprite;


   WaterDrop droplet(40, 140, 7);// Initialize water droplet
   droplet.draw();


   WaterDrop droplet2(175, 23, 7);
   droplet2.draw();


   FEHImage plant("deadplant.png");
   plant.Draw(275, 92);


   // Display Back Button
      LCD.SetFontColor(GOLD);
      LCD.WriteAt("||", 290, 20);

  // Draw buttons
  LCD.SetFontColor(GOLD);
  FEHImage up_arrow("up.png");
  up_arrow.Draw(upX,upY);
  FEHImage right_arrow("right.png");
  right_arrow.Draw(rightX,rightY);
  FEHImage left_arrow("left.png");
  left_arrow.Draw(leftX,leftY);
  FEHImage topright_arrow("topright.png");
  topright_arrow.Draw(topRightX,topRightY);
  FEHImage topleft_arrow("topleft.png");
  topleft_arrow.Draw(topLeftX,topLeftY);




  LCD.Update();

  float x, y, x_trash, y_trash;


  while (true) {
   gameTimer.displayTimer();
   LCD.Update();


   if (droplet.detectCollision(earthboi.x, earthboi.y, 10, 40, 140)) {
            LCD.SetFontColor(0x352100); //Replacement Color
            LCD.FillRectangle(40,140,25,26);
           dropletpts++;
   }
   if (droplet2.detectCollision(earthboi.x, earthboi.y, 10, 175, 23)) {
            LCD.SetFontColor(0x352100); //Replacement Color
            LCD.FillRectangle(175,23,25,26);
           dropletpts++;
   }
   touchPlant(earthboi, plantsprite);

   //?? the hard return??
    // If touchPlant triggered a win it sets currentState to 5; return so main() can show victoryScreen
    if (currentState == 5) {
        Sleep(2);
        return;
    }
    Sleep(50);


      // Wait for a touch
      earthboi.updatePosition(terrain);
      if (LCD.Touch(&x, &y)) {
          //return function
           if(x >= 290 && x <= 310 && y >= 20 && y <= 30){
              currentState = 0;
              return;
          }


          // Check which button was pressed
          if (isButtonPressed(x, y, upX, upY, btnSize, btnSize)) {
              earthboi.jump();
              earthboi.updatePosition(terrain);
          } else if (isButtonPressed(x, y, rightX, rightY, btnSize, btnSize)) {
              earthboi.moveRight(terrain);
              earthboi.updatePosition(terrain);
          } else if (isButtonPressed(x, y, leftX, leftY, btnSize, btnSize)) {
              earthboi.moveLeft(terrain);
              earthboi.updatePosition(terrain);
          } else if (isButtonPressed(x, y, topLeftX, topLeftY, btnSize, btnSize)) {
              earthboi.moveLeft(terrain);
              earthboi.jump();
              earthboi.updatePosition(terrain);
          } else if (isButtonPressed(x, y, topRightX, topRightY, btnSize, btnSize)) {
              earthboi.moveRight(terrain);
              earthboi.jump();
              earthboi.updatePosition(terrain);
          }
      }
     
  }
  if (backButton()) {
      currentState = 0; // Return to Main Menu
  }
}


// Back Button Function
bool backButton() {
  float x, y, x_trash, y_trash;


  // Display Back Button
  LCD.SetFontColor(GOLDENROD);
  LCD.WriteAt("<BACK", 30, 200);


  while (!LCD.Touch(&x, &y)) {}
  while (LCD.Touch(&x_trash, &y_trash)) {}

  // Check if Back Button was Pressed
  return (x >= 20 && x <= 100 && y >= 200 && y <= 220);
}
