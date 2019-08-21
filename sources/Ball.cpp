#include "Ball.h"
#include "SDL2_gfx/SDL2_gfxPrimitives.h"

/**
 * @brief Applies a rotation of <angle> radian degrees to the given point
 * around the given center
 * 
 * @param point address of the point on which apply the rotation
 * @param center center around which the point rotates
 * @param angle in radians
 */
void applyRotationAroundCenter(cpVect* point, cpVect center, float angle);


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