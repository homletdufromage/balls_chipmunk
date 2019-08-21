#pragma once
#include <SDL2/SDL_image.h>
#include "chipmunk/chipmunk.h"

typedef struct vect_t {
   int x, y;
} Vect;

typedef struct segment_t {
   cpVect a, b;
} Segment;

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