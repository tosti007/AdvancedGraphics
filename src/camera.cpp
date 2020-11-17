#include "precomp.h" // include (only) this in every .cpp file
#include "game.h"

Camera::Camera( float px, float py, float pz ) :
    position(px, py, pz), 
    direction(0, 0, 1),
    left(-1, 0, 0), 
    up(0, 1, 0),
    fov(1)
{
}

vec3 Camera::Center()
{
    return position + fov * direction;
}

vec3 Camera::TopLeft()
{
    return Center() + left + up;
}

vec3 Camera::TopRight()
{
    return Center() - left + up;
}

vec3 Camera::BottomLeft()
{
    return Center() + left - up;
}