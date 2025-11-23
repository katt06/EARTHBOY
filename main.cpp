

#include "FEHLCD.h"
#include "FEHUtility.h"
#include "FEHImages.h"
#include <vector>
#include <string>
#include <cmath>
#include <iostream>




//319 by 238




// Function Prototypes
void menuButtons();
void stats();
void play();
void instructions();
void credits();
bool backButton();


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
           FEHImage water("drop.png");
           water.Draw(x,y);
       }
   }
  
   // Method to detect collision with EarthBoi
   bool detectCollision(int earthBoiX, int earthBoiY, int earthBoiRadius) {
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




class EarthBoi {
public:
   float x, y, width = 10, height = 10;  // Position and size
   float velocityX, velocityY;  // Velocity in x and y directions
   float gravity;               // Gravity strength
   bool isJumping;// Flag to check if the player is jumping




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
           LCD.FillRectangle(x,y,10,10) ;
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
       LCD.DrawRectangle(oldx,oldy,10,10);




       LCD.SetFontColor(GREEN);
       LCD.DrawRectangle(x,y,10,10);
   }
};








class Plant {








  private:
      FEHImage deadPlant;
      FEHImage livePlant;


      //position
      int plantx = 275, planty = 92;








  public:
      Plant(){
          deadPlant.Open("deadplant.png");
          livePlant.Open("liveplant.png");
      }


      // Method to start the animation
  void animate(EarthBoi &earthboi) {
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
      deadPlant.Draw(plantx, planty);
      LCD.Update();
  }
};


// Function to check collision between earthboy and plant
void touchPlant(EarthBoi &earthBoi, Plant &psprite) {
   // Get positions and dimensions
   float earthX = earthBoi.x;
   float earthY = earthBoi.y;
   float plantX = 275;
   float plantY = 92;


   // Check for collision (bounding box check)
   if (earthX + 10 >= plantX && earthX - 10 <= plantX + 30 &&
       earthY + 10 >= plantY && earthY - 10 <= plantY + 26) {
       psprite.animate(earthBoi);
       LCD.Write("You Win!");
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
void handleButtonUp(EarthBoi &earthBoi) {
 // earthBoi.moveUp(10);
}








void handleButtonRight(EarthBoi &earthBoi) {
  //earthBoi.moveRIGHT(10);
}








void handleButtonLeft(EarthBoi &earthBoi){
//  earthBoi.moveLEFT(10);
}








void handleButtonUpRight(EarthBoi &earthBoi, float land) {
 // earthBoi.moveUPRIGHT(10, 20, land);
}








void handleButtonUpLeft(EarthBoi &earthBoi, float land){
 // earthBoi.moveUPLEFT(10, 20, land);
}




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
              credits();
              break;
      }
  }








  return 0;
}








// Draws main menu
void menuButtons() {
  LCD.Clear();
  float x, y, x_trash, y_trash;








  // Background
  FEHImage background("level1background.png");
  background.Draw(0, 0);








  // EARTHBOY Title
  FEHImage title("logo.png");
  title.Draw(5, 5);








  // PLAY Button
  FEHImage playImg("play.png");
  playImg.Draw(halfWidth - 30, halfHeight - 15);








  // CREDITS Button
  FEHImage creditsImg("credits.png");
  creditsImg.Draw((halfWidth / 2) - 30, halfHeight + 40);








  // STATS Button
  FEHImage statsImg("stats.png");
  statsImg.Draw((halfWidth + (halfWidth / 2)) - 40, halfHeight + 40);








  // INSTRUCTIONS Button
  FEHImage instructionsImg("gear.png");
  instructionsImg.Draw(halfWidth + 110, halfHeight + 85);








  // Wait for Touch
  while (!LCD.Touch(&x, &y)) {}
  while (LCD.Touch(&x_trash, &y_trash)) {}








  // Determine Button Press
  if (x > (halfWidth - 30) && x < (halfWidth + 30) && y > (halfHeight - 15) && y < (halfHeight + 15)) {
      currentState = 2; // Play
  } else if (x > ((halfWidth / 2) - 30) && x < ((halfWidth / 2) + 50) && y > (halfHeight + 40) && y < (halfHeight + 55)) {
      currentState = 4; // Credits
  } else if (x > ((halfWidth + (halfWidth / 2)) - 40) && x < ((halfWidth + (halfWidth / 2)) + 40) && y > (halfHeight + 40) && y < (halfHeight + 55)) {
      currentState = 1; // Stats
  } else if (x > (halfWidth + 110) && x < (halfWidth + 132) && y > (halfHeight + 85) && y < (halfHeight + 105)) {
      currentState = 3; // Instructions
  }
}








// Stats Page
void stats() {
  LCD.Clear();
  LCD.WriteAt("STATS: ", halfWidth, (halfHeight / 2));
  LCD.WriteAt("LEVEL: 1", halfWidth - 70, halfHeight-20);
  LCD.WriteAt("DROPS: ", halfWidth - 70, halfHeight);
  LCD.WriteAt(dropletpts, halfWidth+20, halfHeight);




  if (backButton()) {
      currentState = 0; // Return to Main Menu
  }
}








// Instructions Page
void instructions() {
  LCD.Clear();
 
   FEHImage instruc("instruction.png");
   instruc.Draw(0,0);






  if (backButton()) {
      currentState = 0; // Return to Main Menu
  }
}








// Credits Page
void credits() {
  LCD.Clear();
  LCD.WriteAt("Creators: ", 50, 50);
  LCD.WriteAt("Kate Tang", 50, 100);
  LCD.WriteAt("Gus Khalid", 50, 150);








  if (backButton()) {
      currentState = 0; // Return to Main Menu
  }
}




// Function to display the victory screen
void victoryScreen() {
   LCD.Clear();


   FEHImage victory("level1background.png");
   victory.Draw(0,0);


   // Display Victory Message
   LCD.SetFontColor(GOLD);
   LCD.WriteAt("YOU WIN!", halfWidth - 50, halfHeight - 30);


   // Display Back Button
   LCD.SetFontColor(WHITE);
   LCD.WriteAt("BACK", halfWidth - 20, halfHeight + 35);


   LCD.Update();


   float x, y;
   float x_trash, y_trash;


   // Wait for touch and check if it's on the back button
   while (!LCD.Touch(&x, &y)) {}
   while (LCD.Touch(&x_trash, &y_trash)) {}


   if (x > (halfWidth - 50) && x < (halfWidth + 50) &&
       y > (halfHeight + 30) && y < (halfHeight + 60)) {
       currentState = 0; // Return to main menu
   }
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


   EarthBoi earthboi(50,50); // Initialize EarthBoi
   Plant plantsprite;


   WaterDrop droplet(40, 120, 7);// Initialize water droplet
   droplet.draw();


   WaterDrop droplet2(175, 45, 7);
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


   if (droplet.detectCollision(earthboi.x, earthboi.y, 10)) {
           dropletpts++;
   }
   if (droplet2.detectCollision(earthboi.x, earthboi.y, 10)) {
           dropletpts++;
   }
   touchPlant(earthboi, plantsprite);
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
  LCD.WriteAt("<BACK", 30, 200);


  while (!LCD.Touch(&x, &y)) {}
  while (LCD.Touch(&x_trash, &y_trash)) {}

  // Check if Back Button was Pressed
  return (x >= 20 && x <= 100 && y >= 200 && y <= 220);
}
