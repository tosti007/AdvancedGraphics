#pragma once

#include "vectors.h"

namespace AdvancedGraphics {

struct Camera
{
public:
    Camera( float px, float py, float pz );
	vec3 Center();
    vec3 TopLeft();
    vec3 TopRight();
    vec3 BottomLeft();
	void MouseUp( int button ) { /* implement if you want to detect mouse button presses */ }
	void MouseDown( int button ) { /* implement if you want to detect mouse button presses */ }
	void MouseMove( int x, int y ) { /* implement if you want to detect mouse movement */ }
	void KeyUp( int key, byte repeat ) { /* implement if you want to handle keys */ }
	void KeyDown( int key, byte repeat ) { /* implement if you want to handle keys */ }
private:
	vec3 position, direction;
    vec3 left, up;
    float fov;
};

}; // namespace AdvancedGraphics