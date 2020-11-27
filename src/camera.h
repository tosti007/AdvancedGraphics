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
    vec3 TopLeft();

	bool MouseUp( int button ) { return false; /* implement if you want to detect mouse button presses */ }
	bool MouseDown( int button ) { return false; /* implement if you want to detect mouse button presses */ }
	bool MouseMove( int x, int y ) { return false; /* implement if you want to detect mouse movement */ }
	bool KeyUp( int key, byte repeat ) { return false; /* implement if you want to handle keys */ }
	bool KeyDown( int key, byte repeat );
	private:
	void RotateAround( vec3 axis, float angle );
};

}; // namespace AdvancedGraphics