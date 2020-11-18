#include "precomp.h" // include (only) this in every .cpp file
#include "camera.h"

Camera::Camera( float px, float py, float pz ) :
    position(px, py, pz), 
    direction(0, 0, 1),
    right(1, 0, 0),
    down(0, -1, 0),
    fov(1)
{
}

vec3 Camera::Center()
{
    return position + fov * direction;
}

vec3 Camera::TopLeft()
{
    return Center() - 0.5 * (right + down);
}
