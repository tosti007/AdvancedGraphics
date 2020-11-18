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

void Camera::KeyDown( int key, byte repeat )
{
    float speed = 0.01;
    switch (key)
    {
        case SDLK_w:
            position += speed * direction;
            break;
        case SDLK_a:
            position -= speed * right;
            break;
        case SDLK_s:
            position -= speed * direction;
            break;
        case SDLK_d:
            position += speed * right;
            break;
        case SDLK_SPACE:
            position -= speed * down;
            break;
        // These do not re-trigger once pressed.
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            position += speed * down;
            break;
    }
}