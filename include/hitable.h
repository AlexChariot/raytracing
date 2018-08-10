#ifndef HITABLEH
#define HITABLEH

//#include "material.h"
#include "aabb.h"
#include "ray.h"

class Material;

struct Hit_record {
    float t;
    vec3 p;
    vec3 normal;
    Material* mat_ptr;
};

class Hitable
{
  public:
    virtual bool hit(const Ray& r, float t_min, float t_max, Hit_record& rec) const = 0;
    virtual bool bounding_box(float t0, float t1, Aabb& box) const = 0;
};

#endif