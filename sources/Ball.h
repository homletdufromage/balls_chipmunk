#pragma once
#include <stdio.h>
#include <SDL2/SDL_image.h>
#include "chipmunk/chipmunk.h"

typedef struct vect_t {
   int x, y;
} Vect;

typedef struct segment_t {
   cpVect a, b;
} Segment;

class GameObject {
   public:
      GameObject(Vect pos, const int mass, const SDL_Color c):
         _position(pos), _mass(mass), _color(c) {};
      ~GameObject() {
         cpShapeFree(_shape);
         _shape = NULL;
         cpBodyFree(_body);
         _body = NULL;
      };
      
      // Accessors
      void setPosition(int x, int y) { _position = {x, y}; }
      Vect getPosition() const { return _position; }

      SDL_Color getColor() const { return _color; }
      int getMass() const { return _mass; }

      cpBody* getBody() const { return _body; }
      void setBody(cpBody* body) { _body = body; }

      cpShape* getShape() const { return _shape; }
      void setShape(cpShape* shape) { _shape = shape; }

   protected:
      Vect _position;
      const int _mass;
      const SDL_Color _color;
      cpBody* _body;
      cpShape* _shape;
};

class Ball : public GameObject{
   public:
      Ball(Vect pos, const int mass, int radius, const SDL_Color c):
         GameObject(pos, mass, c),
         _radius(radius) {}
      
      void createBody(cpSpace* space);

      void render(SDL_Renderer* renderer);

      int getRadius() const { return _radius; }
   private:
      int _radius;
};