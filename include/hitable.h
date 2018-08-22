#ifndef HITABLEH
#define HITABLEH

//#include "material.h"
#include "aabb.h"
#include "ray.h"

class Material;

void get_sphere_uv(const vec3& p, float& u, float& v)
{
    float phi = atan2(p.z(), p.x());
    float theta = asin(p.y());
    u = 1 - (phi + M_PI) / (2 * M_PI);
    v = (theta + M_PI / 2) / M_PI;
}

struct Hit_record {
    float t;
    float u;
    float v;
    vec3 p;
    vec3 normal;
    Material* mat_ptr;
};

/**************************************************************************************************************/
/*
 * Class Hitable
 */
class Hitable
{
  public:
    virtual bool hit(const Ray& r, float t_min, float t_max, Hit_record& rec) const = 0;
    virtual bool bounding_box(float t0, float t1, Aabb& box) const = 0;
};

/**************************************************************************************************************/
/*
 * Class Flip_normals
 */
class Flip_normals : public Hitable
{
  public:
    Flip_normals(Hitable* p) : ptr(p) {}
    virtual bool hit(const Ray& r, float t_min, float t_max, Hit_record& rec) const
    {
        if (ptr->hit(r, t_min, t_max, rec)) {
            rec.normal = -rec.normal;
            return true;
        } else
            return false;
    }
    virtual bool bounding_box(float t0, float t1, Aabb& box) const { return ptr->bounding_box(t0, t1, box); }
    Hitable* ptr;
};

/**************************************************************************************************************/
/*
 * Class Translate
 */
class Translate : public Hitable
{
  public:
    Translate(Hitable* p, const vec3& displacement) : ptr(p), offset(displacement) {}
    virtual bool hit(const Ray& r, float t_min, float t_max, Hit_record& rec) const;
    virtual bool bounding_box(float t0, float t1, Aabb& box) const;
    vec3 offset;
    Hitable* ptr;
};

bool Translate::hit(const Ray& r, float t_min, float t_max, Hit_record& rec) const
{
    Ray moved_r(r.origin() - offset, r.direction(), r.time());
    if (ptr->hit(moved_r, t_min, t_max, rec)) {
        rec.p += offset;
        return true;
    } else
        return false;
}

bool Translate::bounding_box(float t0, float t1, Aabb& box) const
{
    if (ptr->bounding_box(t0, t1, box)) {
        box = Aabb(box.min() + offset, box.max() + offset);
        return true;
    } else
        return false;
}
/**************************************************************************************************************/
/*
 * Class Rotate_y
 */
class Rotate_y : public Hitable
{
  public:
    Rotate_y(Hitable* p, float angle);
    virtual bool hit(const Ray& r, float t_min, float t_max, Hit_record& rec) const;
    virtual bool bounding_box(float t0, float t1, Aabb& box) const
    {
        box = bbox;
        return hasbox;
    }
    Hitable* ptr;
    float sin_theta;
    float cos_theta;
    bool hasbox;
    Aabb bbox;
};
Rotate_y::Rotate_y(Hitable* p, float angle) : ptr(p)
{
    float radians = (M_PI / 180.) * angle;
    sin_theta = sin(radians);
    cos_theta = cos(radians);
    hasbox = ptr->bounding_box(0, 1, bbox);
    vec3 min(FLT_MAX, FLT_MAX, FLT_MAX);
    vec3 max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                float x = i * bbox.max().x() + (1 - i) * bbox.min().x();
                float y = j * bbox.max().y() + (1 - j) * bbox.min().y();
                float z = k * bbox.max().z() + (1 - k) * bbox.min().z();
                float newx = cos_theta * x + sin_theta * z;
                float newz = -sin_theta * x + cos_theta * z;
                vec3 tester(newx, y, newz);
                for (int c = 0; c < 3; c++) {
                    if (tester[c] > max[c]) max[c] = tester[c];
                    if (tester[c] < min[c]) min[c] = tester[c];
                }
            }
        }
    }

    bbox = Aabb(min, max);
}
bool Rotate_y::hit(const Ray& r, float t_min, float t_max, Hit_record& rec) const
{
    vec3 origin = r.origin();
    vec3 direction = r.direction();
    origin[0] = cos_theta * r.origin()[0] - sin_theta * r.origin()[2];
    origin[2] = sin_theta * r.origin()[0] + cos_theta * r.origin()[2];
    direction[0] = cos_theta * r.direction()[0] - sin_theta * r.direction()[2];
    direction[2] = sin_theta * r.direction()[0] + cos_theta * r.direction()[2];
    Ray rotated_r(origin, direction, r.time());
    if (ptr->hit(rotated_r, t_min, t_max, rec)) {
        vec3 p = rec.p;
        vec3 normal = rec.normal;
        p[0] = cos_theta * rec.p[0] + sin_theta * rec.p[2];
        p[2] = -sin_theta * rec.p[0] + cos_theta * rec.p[2];
        normal[0] = cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
        normal[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];
        rec.p = p;
        rec.normal = normal;
        return true;
    } else
        return false;
}
#endif