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
    const float speed = 0.01;
    const float angle = 0.03;
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
        case SDLK_LEFT:
            RotateAround(down, angle);
            break;
        case SDLK_RIGHT:
            RotateAround(down, -angle);
            break;
        case SDLK_UP:
            RotateAround(right, -angle);
            break;
        case SDLK_DOWN:
            RotateAround(right, angle);
            break;
    }
}

void Camera::RotateAround( vec3 axis, float angle )
{
    mat4 m = mat4::rotate(axis, angle);
    direction = m * direction;
    right = m * right;
    down = m * down;
    direction.normalize();
    right.normalize();
    down.normalize();
}
