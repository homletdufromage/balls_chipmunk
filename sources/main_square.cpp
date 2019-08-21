#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <chipmunk/chipmunk.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>

// Constants ==================================================================

const unsigned SCREEN_WIDTH = 800;
const unsigned SCREEN_HEIGHT = 600;

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
   *window = SDL_CreateWindow("Hello Chipmunk !", SDL_WINDOWPOS_CENTERED,
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
typedef struct vect_t {
   int x, y;
} Vect;

class Box {
   public:
      Box(Vect pos, const int width, const int height, const SDL_Color c):
         _position(pos), _width(width), _height(height), _color(c) {};
      
      // Accessors
      void setPosition(int x, int y) { _position = {x, y}; }
      Vect getPosition() const { return _position; }

      int getWidth() const { return _width; }
      int getHeight() const { return _height; }

      SDL_Color getColor() const { return _color; }
   private:
      Vect _position;
      const int _width, _height;
      const SDL_Color _color;
};

void applyRotationAroundCenter(cpVect* point, cpVect center, float angle) {
   float tempX = point->x - center.x;
   float tempY = point->y - center.y;

   float rotatedX = tempX * cos(angle) - tempY * sin(angle);
   float rotatedY = tempX * sin(angle) + tempY * cos(angle);

   point->x = rotatedX + center.x;
   point->y = rotatedY + center.y;         
}

typedef struct segment_t {
   cpVect a, b;
} Segment;

int main(int argc, char const *argv[])  
{
   SDL_Window* window = NULL;
   SDL_Renderer* renderer = NULL;
   SDL_Event e;
   bool quit = false;

   const float SCREEN_FPS = 120.0;
   const float SCREEN_TICKS_PER_FRAME = 1000.0 / SCREEN_FPS;
   Uint32 startTicks, ticksDifference;

   if (!init(&window, &renderer))
      quit = true;

   Box box({SCREEN_WIDTH/2, SCREEN_HEIGHT/4}, 50, 100, {0xFF, 0x00, 0xFF, 0xFF});

   // CHIPMUNK =========

   cpVect gravity = cpv(0, 500);

   // Création d'un nouvel espace
   cpSpace* space = cpSpaceNew();
   cpSpaceSetGravity(space, gravity);

   Segment groundSeg = {{0, SCREEN_HEIGHT/2}, {SCREEN_WIDTH, SCREEN_HEIGHT/2}};
   Segment groundSeg2 = {{SCREEN_WIDTH/4, 0}, {SCREEN_WIDTH/4, SCREEN_HEIGHT}};

   // Création d'un segment représentant le sol
   cpShape* ground = cpSegmentShapeNew(cpSpaceGetStaticBody(space), groundSeg.a, groundSeg.b, 0);
   cpShapeSetFriction(ground, 0.5);
   cpSpaceAddShape(space, ground);
   // Création d'un segment représentant le sol
   cpShape* ground2 = cpSegmentShapeNew(cpSpaceGetStaticBody(space), groundSeg2.a, groundSeg2.b, 0);
   cpShapeSetFriction(ground2, 0.5);
   cpSpaceAddShape(space, ground2);

   /**
    * Maintenant, créons la boite qui tombe au sol.
    * D'abord, créons un cpBody qui détiendra les propriétés physiques de l'objet.
    * Càd la masse, position, vitesse, angle, etc. de l'objet.
    * Ensuite, on attache la forme de collision au corps et on lui donne une taille et une forme.
    **/

   cpFloat mass = 1;

   // Fixons le moment d'inertie ; la masse de la rotation
   cpFloat moment = cpMomentForBox(mass, box.getWidth(), box.getHeight());

   // cpSpaceAdd() renvoie la chose que l'on est en train d'ajouter
   // C'est pratique pour créer et ajouter un objet en une ligne
   cpBody* boxBody = cpSpaceAddBody(space, cpBodyNew(mass, moment));
   cpBodySetPosition(boxBody, cpv(box.getPosition().x, box.getPosition().y));
   cpBodySetAngle(boxBody, M_PI_2);

   // Mmaintenant on peut créer la collision-shapes
   // On peut créer plusieurs collision-shapes qui pointent vers le même corps
   // Elles seront attachées au corps et bougeront pour le suivre
   cpShape* boxShape = cpSpaceAddShape(space, cpBoxShapeNew(boxBody, box.getWidth(), box.getHeight(), box.getWidth()/2));
   //cpShapeSetElasticity(boxShape, 1);
   cpShapeSetFriction(boxShape, 0.5);
   
   // Maintenant que nous avons configuré la boite, on simule tous les objets dans l'espace
   // Avec de petits incréments appelés steps
   cpFloat timeStep = 1.0/SCREEN_FPS;
   while (!quit) {
      startTicks = SDL_GetTicks();

      while (SDL_PollEvent(&e) != 0) {
         if (e.type == SDL_QUIT)
            quit = true;
         else if (e.type == SDL_KEYDOWN) {
            switch(e.key.keysym.sym) {
               case SDLK_LEFT:
                  cpBodySetVelocity(boxBody, {-5, cpBodyGetVelocity(boxBody).y});
                  break;
               case SDLK_RIGHT:
                  cpBodySetVelocity(boxBody, {-5, cpBodyGetVelocity(boxBody).y});
                  break;
            }
         }
         else if (e.type == SDL_MOUSEBUTTONDOWN) {
            int x, y;
            SDL_GetMouseState(&x, &y);
            cpBodySetPosition(boxBody, cpv((cpFloat)x - box.getWidth()/2, (cpFloat)y - box.getHeight()/2));
            cpBodySetVelocity(boxBody, cpv(0, 0));
         }
         else if (e.type == SDL_MOUSEWHEEL) {
            cpBodySetAngle(boxBody, cpBodyGetAngle(boxBody) + M_PI_2);
         }
      }

      // Clear screen
      SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
      SDL_RenderClear(renderer);

      box.setPosition(cpBodyGetPosition(boxBody).x, cpBodyGetPosition(boxBody).y);

      // Render ground
      SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0xFF, 0xFF );  
      SDL_RenderDrawLine(renderer, groundSeg.a.x, groundSeg.a.y, groundSeg.b.x, groundSeg.b.y);
      SDL_RenderDrawLine(renderer, groundSeg2.a.x, groundSeg2.a.y, groundSeg2.b.x, groundSeg2.b.y);
      
      // Render body

      SDL_SetRenderDrawColor(renderer, box.getColor().r, box.getColor().g, box.getColor().b, 0xFF);

      //SDL_RenderDrawPoints(renderer, corners, 2);

      cpFloat angle = cpBodyGetAngle(boxBody);
      
      cpVect center = { (cpFloat)box.getPosition().x, (cpFloat)box.getPosition().y };

      cpVect topLeft = cpv( center.x - box.getWidth()/2 , center.y - box.getHeight()/2 );
      cpVect topRight = cpv( center.x + box.getWidth()/2 , center.y - box.getHeight()/2 );
      cpVect bottomLeft = cpv( center.x - box.getWidth()/2 , center.y + box.getHeight()/2 );
      cpVect bottomRight = cpv( center.x + box.getWidth()/2 , center.y + box.getHeight()/2 );

      applyRotationAroundCenter(&topLeft, center, angle);
      applyRotationAroundCenter(&topRight, center, angle);
      applyRotationAroundCenter(&bottomLeft, center, angle);
      applyRotationAroundCenter(&bottomRight, center, angle);

      SDL_RenderDrawLine(renderer, topLeft.x, topLeft.y, topRight.x, topRight.y);
      SDL_RenderDrawLine(renderer, topLeft.x, topLeft.y, bottomLeft.x, bottomLeft.y);
      SDL_RenderDrawLine(renderer, topRight.x, topRight.y, bottomRight.x, bottomRight.y);
      SDL_RenderDrawLine(renderer, bottomLeft.x, bottomLeft.y, bottomRight.x, bottomRight.y);

      // Update screen
      SDL_RenderPresent(renderer);

      cpSpaceStep(space, timeStep);
      // Keeping <SCREEN_FPS> FPS
      ticksDifference = SDL_GetTicks() - startTicks;
      if (ticksDifference < SCREEN_TICKS_PER_FRAME)
         SDL_Delay(SCREEN_TICKS_PER_FRAME - ticksDifference);
   }

   cpShapeFree(boxShape);
   cpBodyFree(boxBody);
   cpShapeFree(ground);
   cpSpaceFree(space);

   close(&window, &renderer);
   return 0;
}
