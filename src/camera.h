#pragma once

#include "vectors.h"

namespace AdvancedGraphics {

struct Camera
{
	public:
	vec3 position, direction;
	// right and down define the whole "border" of the screen in that direction.
    vec3 right, down;
    float fov;

    Camera( vec3 p, vec3 d );
	vec3 Center();
    vec3 TopLeft();

	void MouseUp( int button ) { /* implement if you want to detect mouse button presses */ }
	void MouseDown( int button ) { /* implement if you want to detect mouse button presses */ }
	void MouseMove( int x, int y ) { /* implement if you want to detect mouse movement */ }
	void KeyUp( int key, byte repeat ) { /* implement if you want to handle keys */ }
	void KeyDown( int key, byte repeat );
	private:
	void RotateAround( vec3 axis, float angle );
};

}; // namespace AdvancedGraphics