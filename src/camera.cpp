#include "precomp.h" // include (only) this in every .cpp file
#include "camera.h"

#define REALDOWN vec3(0, -1, 0)

Camera::Camera( vec3 p, vec3 d ) :
    position(p),
    direction(d),
    right(cross(d, REALDOWN)),
    down(cross(right, d)),
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
        case SDLK_e:
            position -= speed * down;
            break;
        // These do not re-trigger once pressed.
        case SDLK_q:
            position += speed * down;
            break;
    }
}