#include <math.h>       /* sqrt */

#include "precomp.h" // include (only) this in every .cpp file
#include "primitive.h"

Plane::Plane( vec3 n, float d, Pixel c ) :
    Primitive(c),
    normal(n.normalized()),
    dist(d)
{
}

bool Plane::Intersect(Ray* r) 
{
    float t = -(dot(r->origin, normal) + dist) / dot(r->direction, normal);

    if (t >= r->t || t <= 0) return false;
    r->t = t;
    r->obj = this;
    return true;
}

vec3 Plane::NormalAt( vec3 point )
{
	return normal;
}

vec3 Sphere::NormalAt( vec3 point )
{
	return point - position;
}

	Sphere::Sphere( vec3 p, float r, Pixel c ) :
    Primitive(c),
    position(p),
    radius(r)
{
}

bool Sphere::Intersect(Ray* r)
{
    vec3 C = position - r->origin;
    float t = dot(C, r->direction);
    vec3 Q = C - t * r->direction;
    float p2 = dot(Q, Q);
    float r2 = radius * radius;
    if (p2 > r2) return false;
    t -= sqrt(r2 - p2);
    
    if (t >= r->t || t <= 0) return false;
    r->t = t;
    r->obj = this;
    return true;
}

Triangle::Triangle( vec3 v0, vec3 v1, vec3 v2, Pixel c) :
    Primitive(c),
    p0(v0),
    p1(v1),
    p2(v2),
    normal(cross(v1 - v0, v2 - v0))
{
}

Triangle::Triangle( vec3 v0, vec3 v1, vec3 v2, vec3 n, Color c) :
    Primitive(c),
    p0(v0),
    p1(v1),
    p2(v2),
    normal(n)
{
}

bool Triangle::Intersect(Ray* r)
{
    // Implementation from:
    // https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
    vec3 p0p1 = p1 - p0;
    vec3 p0p2 = p2 - p0;
    vec3 pvec = r->direction.cross(p0p2);
    float det = p0p1.dot(pvec);

    // ray and triangle are parallel if det is close to 0
    // This should probaby be something smaller, but for now it will do
	if ( fabs( det ) < 0.0000001 ) return false;

    float invDet = 1 / det;

    vec3 tvec = r->origin - p0;
    float u = tvec.dot(pvec) * invDet;
    if (u < 0 || u > 1) return false;

    vec3 qvec = tvec.cross(p0p1);
    float v = r->direction.dot(qvec) * invDet;
    if (v < 0 || u + v > 1) return false;

    float t = p0p2.dot(qvec) * invDet;

    if (t >= r->t || t <= 0) return false;
    r->t = t;
    r->obj = this;
    return true;
}

vec3 Triangle::NormalAt( vec3 point )
{
	return normal;
}

vec3 Triangle::ComputeNormal( vec3 v0, vec3 v1, vec3 v2 )
{
    return cross(v1 - v0, v2 - v0);
}

vec3 TinyObjGetVector(int idx, std::vector<tinyobj::real_t>* values) {
    assert(idx >= 0);
    // I think there are better ways than to use "at", but im lazy for now.
	tinyobj::real_t vx = values->at(3 * idx + 0);
	tinyobj::real_t vy = values->at(3 * idx + 1);
	tinyobj::real_t vz = values->at(3 * idx + 2);
    return vec3(vx, vy, vz);
}

Triangle Triangle::FromTinyObj( tinyobj::attrib_t* attrib, tinyobj::mesh_t* mesh, size_t f )
{
    tinyobj::index_t idx0 = mesh->indices[3 * f + 0];
    tinyobj::index_t idx1 = mesh->indices[3 * f + 1];
    tinyobj::index_t idx2 = mesh->indices[3 * f + 2];

    /* Just an example of how to retrieve material id and values.
    vec3 diffuse(0);
    int current_material_id = mesh->material_ids[f];
    for (size_t i = 0; i < 3; i++) {
        diffuse[i] = materials[current_material_id].diffuse[i];
    }
    */

    vec3 v0 = TinyObjGetVector(idx0.vertex_index, &attrib->vertices);
    vec3 v1 = TinyObjGetVector(idx1.vertex_index, &attrib->vertices);
    vec3 v2 = TinyObjGetVector(idx2.vertex_index, &attrib->vertices);

    // I think colors are defined on a per-vertex base.
    // Since we need only the full face normal let's just compute it ourselves.
    vec3 n = ComputeNormal(v0, v1, v2);

    // I think colors are defined on a per-vertex base, hence I don't currently know how to handle this. 
    // Color c = (Color)TinyObjGetVector(idx.vertex_index, &attrib.colors);
    // Let's just use white.
    Color c(0.0f, 1.0f, 0.0f);

    return Triangle(v0, v1, v2, n, c);
}

TriangleSoup::TriangleSoup(Triangle* fs, uint nfs, Pixel c) :
    Primitive(c),
    nr_faces(nfs),
    faces(fs)
{
}

bool TriangleSoup::Intersect(Ray* r)
{
    // TODO: do some AABB intersection here first.

    bool found = false;
    for (uint i = 0; i < nr_faces; i++)
    {
        found |= faces[i].Intersect(r);
    }
    return found;
}

vec3 TriangleSoup::NormalAt( vec3 point )
{
	return NULL;
}

void TriangleSoup::FromTinyObj( TriangleSoup* soup, tinyobj::attrib_t* attrib, tinyobj::mesh_t* mesh)
{
    soup->nr_faces = mesh->indices.size() / 3;
    soup->faces = new Triangle[soup->nr_faces];
    for (size_t f = 0; f < soup->nr_faces; f++)
    {
        // This MUST hold for our custom Triangle implementaion, if it isnt, then this face is no triangle, but e.g. a quad.
        assert(mesh->num_face_vertices[f] == 3);
        // I think this will work and not result in NULL pointers later on.
        soup->faces[f] = Triangle::FromTinyObj(attrib, mesh, f);
    }
}
