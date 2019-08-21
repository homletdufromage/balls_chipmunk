#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "chipmunk/chipmunk.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <vector>
#include <sstream>
#include "SDL2_gfx/SDL2_gfxPrimitives.h"

// Texture ==================================================================== 
class Texture {
	public:
		// Initialise les variables
		Texture();

		// Désalloue la mémoire
		~Texture();

      bool initFont(std::string fontFile);

		//Creates image from font string
      bool loadFromRenderedText(std::string textureText, SDL_Color textColor, SDL_Renderer* renderer);

		// Désalloue la mémoire de la texture
		void free();

		// Change le modificateur de couleur
		void setColor(Uint8 red, Uint8 green, Uint8 blue);

		//Set blending
      void setBlendMode(SDL_BlendMode blending);

		// Affiche la texture à certaines coordonnées donné
		void render(SDL_Renderer* renderer);

		// Accesseurs de la texture
		int getWidth();
		int getHeight();

      void setPosition(int x, int y) { _x = x; _y = y; }

	private:
      TTF_Font* _font;
		SDL_Texture* _texture;

		// Dimensions de l'image
		int _width, _height;
      int _x, _y;
};


Texture::Texture() {
	_texture = NULL;
   _font = NULL;
	_width = 0;
	_height = 0;
   _x = _y = 0;
}

Texture::~Texture() {
	free();
}

bool Texture::initFont(std::string fontFile) {
   _font = TTF_OpenFont("Minecraft.ttf", 20);
   if (!_font) {
      printf("Can't load font. Error : %s\n", TTF_GetError());
      return false;
   }
   return true;
}

bool Texture::loadFromRenderedText(std::string textureText, SDL_Color textColor, SDL_Renderer* renderer) {
	// Get rid of another loaded text if there is one
	free();

	SDL_Surface* textSurface = TTF_RenderText_Blended(_font, textureText.c_str(), textColor);
	if (!textSurface) {
		printf("Could not render text surface. SDL_ttf error : %s\n", TTF_GetError());
	} else {
		_texture = SDL_CreateTextureFromSurface(renderer, textSurface);
		if (_texture == NULL) {
			printf("Unable to create texture from rendered text. Error : %s\n", SDL_GetError());
		} else {
			_width = textSurface->w;
			_height = textSurface->h;
		}

		SDL_FreeSurface(textSurface);
	}

	return _texture != NULL;
}

void Texture::free() {
	if(_texture) {
		SDL_DestroyTexture(_texture);
		_texture = NULL;
		_width = 0;
		_height = 0;
	}
}

void Texture::setColor(Uint8 red, Uint8 green, Uint8 blue) {
	SDL_SetTextureColorMod(_texture, red, green, blue);
}

void Texture::setBlendMode(SDL_BlendMode blending) {
	SDL_SetTextureBlendMode(_texture, blending);
}

void Texture::render(SDL_Renderer* renderer) {
	//Set rendering space and render to the screen
	SDL_Rect renderQuad = { _x, _y, _width, _height };

	// C'est ici qu'on choisit "clip" comme "Rect" de zone
	SDL_RenderCopyEx(renderer, _texture, NULL, &renderQuad, 0, NULL, SDL_FLIP_NONE);
}

int Texture::getWidth() {
	return _width;
}

int Texture::getHeight() {
	return _height;
}

// Timer ======================================================================
class Timer {
	public:
		Timer();

		void start();
        void stop();
        void pause();
        void unpause();

        //Gets the timer's time
        Uint32 getTicks();

        //Checks the status of the timer
        bool isStarted();
        bool isPaused();

    private:
        //The clock time when the timer started
        Uint32 mStartTicks;

        //The ticks stored when the timer was paused
        Uint32 mPausedTicks;

        //The timer status
        bool mPaused;
        bool mStarted;
};

Timer::Timer()
{
    //Initialize the variables
    mStartTicks = 0;
    mPausedTicks = 0;

    mPaused = false;
    mStarted = false;
}

void Timer::start() {
	mStarted = true;

	mPaused = false;

	mStartTicks = SDL_GetTicks();
	mPausedTicks = 0;
}

void Timer::stop()
{
    //Stop the timer
    mStarted = false;

    //Unpause the timer
    mPaused = false;

    //Clear tick variables
    mStartTicks = 0;
    mPausedTicks = 0;
}

void Timer::pause()
{
    //If the timer is running and isn't already paused
    if( mStarted && !mPaused )
    {
        //Pause the timer
        mPaused = true;

        //Calculate the paused ticks
        mPausedTicks = SDL_GetTicks() - mStartTicks;
        mStartTicks = 0;
    }
}

void Timer::unpause()
{
    //If the timer is running and paused
    if( mStarted && mPaused )
    {
        //Unpause the timer
        mPaused = false;

        //Reset the starting ticks
        mStartTicks = SDL_GetTicks() - mPausedTicks;

        //Reset the paused ticks
        mPausedTicks = 0;
    }
}

Uint32 Timer::getTicks() {
	Uint32 time = 0;

	if (mStarted) {
		if (mPaused)
			time = mPausedTicks;
		else
			time = SDL_GetTicks() - mStartTicks;
	}

	return time;
}

bool Timer::isStarted()
{
    //Timer is running and paused or unpaused
    return mStarted;
}

bool Timer::isPaused()
{
    //Timer is running and paused
    return mPaused && mStarted;
}

// Constants ==================================================================

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;
const bool BALLS_AS_POINTS = false;

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

      void render(SDL_Renderer* renderer);
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

/**
 * @brief Applies a rotation of <angle> radian degrees to the given point
 * around the given center
 * 
 * @param point address of the point on which apply the rotation
 * @param center center around which the point rotates
 * @param angle in radians
 */
void applyRotationAroundCenter(cpVect* point, cpVect center, float angle);

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
                              SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);
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

   SDL_SetRenderDrawBlendMode(*renderer, SDL_BLENDMODE_BLEND);

   // Initialization of the PNG pictures loader
   if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
      printf("SDL_Image could not initialize. Error : %s\n", IMG_GetError());
      return false;
   }

   // Initialization of the TTF text manager
   if (TTF_Init() == -1) {
      printf("SDL_TTF could not initialize. Error : %s\n", TTF_GetError());
      return false;
   }

   return true;
}

bool loadMedia(SDL_Renderer* renderer, Texture& textTexture) {
   SDL_Color textColor = {0xFF, 0xFF, 0xFF, 0xFF};
   if (!textTexture.loadFromRenderedText("MODO Seal3 : Coucou les blablas ^^", textColor, renderer)) {
      printf("Could not render text texture\n");
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

void applyRotationAroundCenter(cpVect* point, cpVect center, float angle) {
   float tempX = point->x - center.x;
   float tempY = point->y - center.y;

   float rotatedX = tempX * cos(angle) - tempY * sin(angle);
   float rotatedY = tempX * sin(angle) + tempY * cos(angle);

   point->x = rotatedX + center.x;
   point->y = rotatedY + center.y;         
}


void Ball::render(SDL_Renderer* renderer) {
   _position.x = cpBodyGetPosition(_body).x;
   _position.y = cpBodyGetPosition(_body).y;
   if (BALLS_AS_POINTS) {
      SDL_SetRenderDrawColor(renderer, _color.r, _color.g, _color.b, _color.a);
      SDL_RenderDrawPoint(renderer, _position.x, _position.y);
   } else {
      SDL_SetRenderDrawColor(renderer, _color.r, _color.g, _color.b, _color.a);

      filledCircleRGBA(renderer, _position.x, _position.y, _radius, _color.r,
                     _color.g, _color.b, 255 * 0.5 );
      aacircleColor(renderer, _position.x, _position.y, _radius, 0xFFFFFFFF);


      // X:Y axis on the ball 
      cpVect center, point1, point2;
      center = { (cpFloat) _position.x, (cpFloat) _position.y };
      point1 = { (cpFloat) _position.x, (cpFloat) _position.y - (cpFloat) _radius * 0.7 };
      point2 = { (cpFloat) _position.x + _radius * 0.7, (cpFloat) _position.y };
      applyRotationAroundCenter(&point1, center, cpBodyGetAngle(_body));
      applyRotationAroundCenter(&point2, center, cpBodyGetAngle(_body));

      aalineRGBA(renderer, _position.x, _position.y, point1.x, point1.y, 0x00, 0x00, 0xFF, 0xFF * 0.8);

      aalineRGBA(renderer, _position.x, _position.y, point2.x, point2.y, 0xFF, 0x00, 0x00, 0xFF * 0.8);

      SDL_SetRenderDrawColor(renderer, 0X00, 0xFF, 0x00, 0xFF);
      SDL_RenderDrawPoint(renderer, _position.x, _position.y);
   }
}

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

   // SDL_Font related stuff
   Texture textTexture;
   textTexture.setPosition(50, 50);

   // Initialization of SDL
   if (!init(&window, &renderer))
      quit = true;
   
   if (!textTexture.initFont("Minecraft.ttf"))
      return false;

   // Initialization of the text
   if (!loadMedia(renderer, textTexture))
      quit = true;

   // Chipmunk stuff
   cpVect gravity = cpv(0, 1000);

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

   unsigned NB_BALLS = 0;

   // FPS management
   const float SCREEN_FPS = 60.0;
   const float SCREEN_TICKS_PER_FRAME = 1000.0 / SCREEN_FPS;
   cpFloat timeStep = 1.0/SCREEN_FPS;
   Uint32 startTicks, ticksDifference;
   Timer fpsTimer;
   std::stringstream fpsText;
   int countedFrames = 0;
   fpsTimer.start();

   // Main loop
   while (!quit) {
      startTicks = SDL_GetTicks();
      
      // Events manager
      while (SDL_PollEvent(&e) != 0) {
         if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE))
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
                  NB_BALLS = 0;
                  clearSpace(space, &balls);
                  break;
               case SDLK_p:
                  NB_BALLS_TO_ADD += 10;
                  printf("Nb balls to add set to %d\n", NB_BALLS_TO_ADD);
                  break;
               case SDLK_m:
                  NB_BALLS_TO_ADD -= 10;
                  if (NB_BALLS_TO_ADD < 0)
                     NB_BALLS_TO_ADD = 1;
                  printf("Nb balls to add set to %d\n", NB_BALLS_TO_ADD);
                  break;
            }
         } 
         else if (e.type == SDL_MOUSEBUTTONDOWN && !mouseConstraint) {
            int x, y;
            Uint32 button = SDL_GetMouseState(&x, &y);
            if (button == 1) {
               for (int i = 0; i < NB_BALLS_TO_ADD; i++) {
                  int radius = 30, mass = 5;
                  SDL_Color color;
                  color.r = (Uint32) (rand() % 0xFF);
                  color.g = (Uint32) (rand() % 0xFF);
                  color.b = (Uint32) (rand() % 0xFF);
                  color.a = 0xFF;
                  Ball b({rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT}, radius, mass, color);
                  if (NB_BALLS_TO_ADD == 1)
                     b.setPosition(x, y);
                        
                  addBall(&balls, b, space);
               }
               NB_BALLS += NB_BALLS_TO_ADD;
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
      
      // Render balls
      for(unsigned i = 0; i < balls.size(); i++)
         balls[i].render(renderer);

      // Render constraints
      if (mouseConstraint) {
         int x, y;
         SDL_GetMouseState(&x, &y);

         SDL_RenderDrawLine(renderer, x, y, balls[linkedBallId].getPosition().x, balls[linkedBallId].getPosition().y);


         aalineRGBA(renderer, x, y, balls[linkedBallId].getPosition().x, balls[linkedBallId].getPosition().y, 0x00, 0xFF, 0x00, 0xFF * 0.5);


         filledCircleRGBA(renderer, balls[linkedBallId].getPosition().x, balls[linkedBallId].getPosition().y, 2, 0x00,
                          0xFF, 0x00, 255);
         filledCircleRGBA(renderer, x, y, 2, 0x00, 0xFF, 0x00, 255);
      }

      // Render text
      float avgFPS = countedFrames / (fpsTimer.getTicks() / 1000.0);
      if (avgFPS > 2000000)
         avgFPS = 0;
      
      fpsText.str("");
      fpsText << "Balls count : " << NB_BALLS << " - FPS : " << round(avgFPS);
      if (!textTexture.loadFromRenderedText(fpsText.str().c_str(), {0xFF, 0xFF, 0xFF, 0xFF}, renderer))
         printf("Could not render text\n");
      textTexture.render(renderer);
      
      // Update screen
      SDL_RenderPresent(renderer);

      // Step
      cpSpaceStep(space, timeStep);

      // Keeping <SCREEN_FPS> FPS
      ++ countedFrames;
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
