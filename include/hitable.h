#ifndef HITABLEH
#define HITABLEH

//#include "material.h"
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
    virtual bool hit(const ray& r, float t_min, float t_max, Hit_record& rec) const = 0;
};

#endif