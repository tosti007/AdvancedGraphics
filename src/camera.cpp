#include "precomp.h" // include (only) this in every .cpp file
#include "camera.h"

#define REALDOWN vec3(0, -1, 0)

Camera::Camera( vec3 p, vec3 d ) :
    position(p),
    direction(d.normalized()),
    right(cross(direction, REALDOWN).normalized()),
    down(cross(right, direction).normalized()),
    fov(1),
    topLeft()
{
    UpdateTopLeft();
}

void Camera::UpdateTopLeft()
{
    topLeft = fov * direction - 0.5 * (right + down);
}

bool Camera::KeyDown( int key, byte repeat )
{
    const float speed = 0.1;
    const float angle = 0.03;
    switch (key)
    {
        case SDLK_w:
            position += speed * direction;
            return true;
        case SDLK_a:
            position -= speed * right;
            return true;
        case SDLK_s:
            position -= speed * direction;
            return true;
        case SDLK_d:
            position += speed * right;
            return true;
        case SDLK_e:
            position -= speed * down;
            return true;
        case SDLK_r:
            fov += speed;
            return true;
        case SDLK_f:
            fov -= speed;
            return true;
        // These do not re-trigger once pressed.
        case SDLK_q:
            position += speed * down;
            return true;
        case SDLK_LEFT:
            RotateAround(down, angle);
            return true;
        case SDLK_RIGHT:
            RotateAround(down, -angle);
            return true;
        case SDLK_UP:
            RotateAround(right, -angle);
            return true;
        case SDLK_DOWN:
            RotateAround(right, angle);
            return true;
    }
    return false;
}

void Camera::RotateAround( vec3 axis, float angle )
{
    mat4 m = mat4::rotate(axis, angle);
    vec3 newdir = m * direction;

    // Check if we are not looking almost down or up
    if (std::abs(dot(newdir, REALDOWN)) < 0.9) {
        // We can choose here to do a cross or a mat4 multiplication
        // As we need to normalize anyhow doing twice a cross is cheaper
        direction = newdir;
        right = cross(direction, REALDOWN).normalized();
        down = cross(right, direction).normalized();
        UpdateTopLeft();
    }
}
