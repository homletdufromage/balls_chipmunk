#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <chipmunk/chipmunk.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <vector>
#include "SDL2_gfx/SDL2_gfxPrimitives.h"

// Constants ==================================================================

const unsigned SCREEN_WIDTH = 800;
const unsigned SCREEN_HEIGHT = 600;

// Structures =================================================================

typedef struct vect_t {
   int x, y;
} Vect;

typedef struct segment_t {
   cpVect a, b;
} Segment;

// Classes ====================================================================

class Ball {
   public:
      Ball(Vect pos, const int radius, const int mass, const SDL_Color c):
         _position(pos), _radius(radius), _mass(mass), _color(c) {};
      
      // Accessors
      void setPosition(int x, int y) { _position = {x, y}; }
      Vect getPosition() const { return _position; }

      int getRadius() const { return _radius; }
      SDL_Color getColor() const { return _color; }
      int getMass() const { return _mass; }

      cpBody* getBody() const { return _body; }
      void setBody(cpBody* body) { _body = body; }

      cpShape* getShape() const { return _shape; }
      void setShape(cpShape* shape) { _shape = shape; }
   private:
      Vect _position;
      const int _radius;
      const int _mass;
      const SDL_Color _color;
      cpBody* _body;
      cpShape* _shape;
};

// Functions declarations =====================================================

/**
 * @brief Initializes needed SDL features and creates the main window and
 * the renderer
 * 
 * @param window address of the pointer to the SDL_Window
 * @param renderer address of the pointer to the SDL_Renderer
 * @return true everything has been initialized
 * @return false an error occured
 */
bool init(SDL_Window** window, SDL_Renderer** renderer);

/**
 * @brief Closes SDL and frees allocated memory
 * 
 * @param window address of the pointer to the SDL_Window
 * @param renderer address of the pointer to the SDL_Renderer
 */
void close(SDL_Window** window, SDL_Renderer** renderer);

/**
 * @brief Calculates the norm of a vector
 * 
 * @param point1 first point of the vector
 * @param point2 second point
 * @return float norm of the vector
 */
float calculateNorm(cpVect point1, cpVect point2);

/**
 * @brief Adds a ball to the space and adds the ball to the balls list
 * 
 * @param balls_list balls vector
 * @param ball the ball to add
 * @param space existing and initialized cpSpace
 */
void addBall(std::vector<Ball>* balls_list, Ball& ball, cpSpace* space);

/**
 * @brief Removes every ball from the cpSpace and clears the balls vector
 * 
 * @param space existing cpSpace
 * @param balls balls vector
 */
void clearSpace(cpSpace* space, std::vector<Ball>* balls);

// Functions definitions ======================================================

bool init(SDL_Window** window, SDL_Renderer** renderer) {
   // SDL Initialization
   if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      printf("SDL could not initialize. Error : %s\n", SDL_GetError());
      return false;
   }
   // Texture filtering
   if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
      printf("Warning : linear texture filtering not enabled !\n");

   // Initlialization of the main window
   *window = SDL_CreateWindow("Rings and rings and rings", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH,
                              SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
   if (!*window) {
      printf("Window could not be created. Error : %s\n", SDL_GetError());
      return false;
   }

   // Initialization of the texture renderer
   *renderer = SDL_CreateRenderer(*window, -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

   if(!*renderer) {
      printf("Renderer could not be created. Error : %s\n", SDL_GetError());
      return false;
   }

   SDL_SetRenderDrawColor(*renderer, 0x00, 0x00, 0x00, 0xFF);
   SDL_SetRenderDrawBlendMode(*renderer, SDL_BLENDMODE_BLEND);

   // Initialization of the PNG pictures loader
   if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
      printf("SDL_Image could not initialize. Error : %s\n", IMG_GetError());
      return false;
   }

   return true;
}

void close(SDL_Window** window, SDL_Renderer** renderer) {
   assert(window && renderer);

   if (*window) {
      SDL_DestroyWindow(*window);
      *window = NULL;
   }
   if (*renderer) {
      SDL_DestroyRenderer(*renderer);
      *renderer = NULL;
   }

   IMG_Quit();
   SDL_Quit();
}

// TODO: use this function to rotate boxes rendered with segments
#ifdef _TODO_
void applyRotationAroundCenter(cpVect* point, cpVect center, float angle) {
   float tempX = point->x - center.x;
   float tempY = point->y - center.y;

   float rotatedX = tempX * cos(angle) - tempY * sin(angle);
   float rotatedY = tempX * sin(angle) + tempY * cos(angle);

   point->x = rotatedX + center.x;
   point->y = rotatedY + center.y;         
}
#endif

float calculateNorm(cpVect point1, cpVect point2) {
   cpFloat diff_x = pow(point2.x - point1.x, 2);
   cpFloat diff_y = pow(point2.y - point1.y, 2);
   cpFloat sum = diff_x + diff_y;

   return sqrt(sum);
}

void addBall(std::vector<Ball>* balls_list, Ball& ball,cpSpace* space) {
   // Moment of inertia
   cpFloat moment = cpMomentForCircle(ball.getMass(), 0, ball.getRadius(), cpvzero);

   // Ball body
   cpBody* ballBody = cpSpaceAddBody(space, cpBodyNew(ball.getMass(), moment));
   cpBodySetPosition(ballBody, cpv(ball.getPosition().x, ball.getPosition().y));
   ball.setBody(ballBody);

   // Collision shape of the ball
   cpShape* ballShape = cpSpaceAddShape(space, cpCircleShapeNew(ballBody, ball.getRadius(), cpvzero));
   cpShapeSetFriction(ballShape, 0.7);
   cpShapeSetElasticity(ballShape, 1);
   ball.setShape(ballShape);

   balls_list->push_back(ball);
}

void clearSpace(cpSpace* space, std::vector<Ball>* balls) {
   for (unsigned i = 0; i < balls->size(); i++) {
      cpSpaceRemoveShape(space, (*balls)[i].getShape());
      cpShapeFree((*balls)[i].getShape());
      (*balls)[i].setShape(NULL);
      cpSpaceRemoveBody(space, (*balls)[i].getBody());
      cpBodyFree((*balls)[i].getBody());
      (*balls)[i].setBody(NULL);
   }
   balls->clear();
}

int main(int argc, char const *argv[])  
{  
   // SDL related stuff
   SDL_Window* window = NULL;
   SDL_Renderer* renderer = NULL;
   SDL_Event e;
   bool quit = false;

   // FPS management
   const float SCREEN_FPS = 120.0;
   const float SCREEN_TICKS_PER_FRAME = 1000.0 / SCREEN_FPS;
   cpFloat timeStep = 1.0/SCREEN_FPS;
   Uint32 startTicks, ticksDifference;

   // Initialization of SDL
   if (!init(&window, &renderer))
      quit = true;

   // Chipmunk stuff
   cpVect gravity = cpv(0, 500);

   // Creation of the new space
   cpSpace* space = cpSpaceNew();
   cpSpaceSetGravity(space, gravity);
   
   // Creation of the walls
   const int NB_WALLS = 4;

   Segment floor = {{0, SCREEN_HEIGHT-10}, {SCREEN_WIDTH, SCREEN_HEIGHT-10}};
   Segment roof = {{0, 10}, {SCREEN_WIDTH, 10}};
   Segment leftWall = { { 10, 0 },{ 10, SCREEN_HEIGHT } };
   Segment rightWall = { { SCREEN_WIDTH-10, 0 },{ SCREEN_WIDTH-10, SCREEN_HEIGHT } };

   cpShape* ground[4] = {cpSegmentShapeNew(cpSpaceGetStaticBody(space), floor.a, floor.b, 0), 
                         cpSegmentShapeNew(cpSpaceGetStaticBody(space), roof.a, roof.b, 0), 
                         cpSegmentShapeNew(cpSpaceGetStaticBody(space), leftWall.a, leftWall.b, 0), 
                         cpSegmentShapeNew(cpSpaceGetStaticBody(space), rightWall.a, rightWall.b, 0) };
                         
   for(int i = 0; i < NB_WALLS; i++) {
      cpShapeSetFriction(ground[i], 0.5);
      cpShapeSetElasticity(ground[i], 1);
      cpSpaceAddShape(space, ground[i]);
   }

   // Balls
   std::vector<Ball> balls;

   // Constraint management
   int NB_BALLS_TO_ADD = 1;
   cpConstraint* mouseConstraint = NULL;
   int linkedBallId = -1;

   // Main loop
   while (!quit) {
      startTicks = SDL_GetTicks();
      
      // Events manager
      while (SDL_PollEvent(&e) != 0) {
         if (e.type == SDL_QUIT)
            quit = true;
         else if (e.type == SDL_KEYDOWN) {
            switch(e.key.keysym.sym) {
               case SDLK_r:
                  if (mouseConstraint) {
                     cpSpaceRemoveConstraint(space, mouseConstraint);
                     cpConstraintFree(mouseConstraint);
                     mouseConstraint = NULL;
                     linkedBallId = -1;
                  }
                  clearSpace(space, &balls);
                  break;
               case SDLK_p:
                  NB_BALLS_TO_ADD += 10;
                  printf("Nb balls to add set to %d\n", NB_BALLS_TO_ADD);
                  break;
               case SDLK_m:
                  NB_BALLS_TO_ADD -= 10;
                  if (NB_BALLS_TO_ADD < 0)
                     NB_BALLS_TO_ADD = 0;
                  printf("Nb balls to add set to %d\n", NB_BALLS_TO_ADD);
                  break;
            }
         } 
         else if (e.type == SDL_MOUSEBUTTONDOWN && !mouseConstraint) {
            int x, y;
            Uint32 button = SDL_GetMouseState(&x, &y);
            if (button == 1) {
               for (int i = 0; i < NB_BALLS_TO_ADD; i++) {
                  int radius = 25, mass = 5;
                  SDL_Color color;
                  color.r = (Uint32) (rand() % 0xFF);
                  color.g = (Uint32) (rand() % 0xFF);
                  color.b = (Uint32) (rand() % 0xFF);
                  color.a = 0xFF;
                  Ball b({x, y}, radius, mass, color);
                  addBall(&balls, b, space);
               }
               printf("%d %s added at (%d, %d)\n", NB_BALLS_TO_ADD,
                      (NB_BALLS_TO_ADD==1)?"ball":"balls", x, y);
            }
            else if (button == 4) {
               for (Uint32 i = 0; i < balls.size(); i++) {
                  if (calculateNorm(cpv(x, y), cpv(balls[i].getPosition().x, balls[i].getPosition().y)) <= balls[i].getRadius()) {
                     linkedBallId = i;
                     mouseConstraint = cpPivotJointNew(balls[i].getBody(), cpSpaceGetStaticBody(space), cpv(x, y));
                     cpSpaceAddConstraint(space, mouseConstraint);
                     break;
                  }
               }
            }
         }
         else if (e.type == SDL_MOUSEBUTTONUP && mouseConstraint) {
            cpSpaceRemoveConstraint(space, mouseConstraint);
            cpConstraintFree(mouseConstraint);
            mouseConstraint = NULL;
            linkedBallId = -1;
         }
         else if (e.type == SDL_MOUSEMOTION) {
            if (mouseConstraint) {
               int x, y;
               SDL_GetMouseState(&x, &y);
               cpPivotJointSetAnchorB(mouseConstraint, cpv(x, y));
            }
         }
      }

      // Clear screen
      SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
      SDL_RenderClear(renderer);

      // Render walls
      SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0xFF, 0xFF );  
      SDL_RenderDrawLine(renderer, floor.a.x, floor.a.y, floor.b.x, floor.b.y);
      SDL_RenderDrawLine(renderer, roof.a.x, roof.a.y, roof.b.x, roof.b.y);
      SDL_RenderDrawLine(renderer, leftWall.a.x, leftWall.a.y, leftWall.b.x, leftWall.b.y);
      SDL_RenderDrawLine(renderer, rightWall.a.x, rightWall.a.y, rightWall.b.x, rightWall.b.y);

      // Render constraints
      if (mouseConstraint) {
         int x, y;
         SDL_GetMouseState(&x, &y);
         SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF );  
         SDL_RenderDrawLine(renderer, x, y, balls[linkedBallId].getPosition().x, balls[linkedBallId].getPosition().y);
      }
      
      // Render balls
      for(unsigned i = 0; i < balls.size(); i++) {
         balls[i].setPosition(cpBodyGetPosition(balls[i].getBody() ).x, cpBodyGetPosition(balls[i].getBody()).y);

         SDL_SetRenderDrawColor(renderer, balls[i].getColor().r, balls[i].getColor().g, balls[i].getColor().b, balls[i].getColor().a);

         filledCircleRGBA(renderer, balls[i].getPosition().x, balls[i].getPosition().y, balls[i].getRadius(), balls[i].getColor().r,
                        balls[i].getColor().g, balls[i].getColor().b, 255 * 0.6 );

         aacircleColor(renderer, balls[i].getPosition().x, balls[i].getPosition().y, balls[i].getRadius(), 0xFFFFFFFF);
      }
      // Update screen
      SDL_RenderPresent(renderer);

      // Step
      cpSpaceStep(space, timeStep);

      // Keeping <SCREEN_FPS> FPS
      ticksDifference = SDL_GetTicks() - startTicks;
      if (ticksDifference < SCREEN_TICKS_PER_FRAME)
         SDL_Delay(SCREEN_TICKS_PER_FRAME - ticksDifference);
   }

   clearSpace(space, &balls);
   
   for(unsigned i = 0; i < NB_WALLS; i++)
      cpShapeFree(ground[i]);

   if (mouseConstraint) {
      cpSpaceRemoveConstraint(space, mouseConstraint);
      cpConstraintFree(mouseConstraint);
      mouseConstraint = NULL;
      linkedBallId = -1;
   }
   cpSpaceFree(space);
   space = NULL;

   close(&window, &renderer);
   return 0;
}
