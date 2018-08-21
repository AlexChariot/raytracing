#ifndef AARECTH
#define AARECTH

#include "hitable.h"

class XY_rect : public Hitable
{
  public:
    XY_rect() {}
    XY_rect(float _x0, float _x1, float _y0, float _y1, float _k, Material* mat) : x0(_x0), x1(_x1), y0(_y0), y1(_y1), k(_k), mp(mat){};
    virtual bool hit(const Ray& r, float t_min, float t_max, Hit_record& rec) const;
    virtual bool bounding_box(float t0, float t1, Aabb& box) const
    {
        box = Aabb(vec3(x0, y0, k - 0.0001), vec3(x1, y1, k + 0.0001));
        return true;
    }
    Material* mp;
    float x0, x1, y0, y1, k;
};

class XZ_rect : public Hitable
{
  public:
    XZ_rect() {}
    XZ_rect(float _x0, float _x1, float _z0, float _z1, float _k, Material* mat) : x0(_x0), x1(_x1), z0(_z0), z1(_z1), k(_k), mp(mat){};
    virtual bool hit(const Ray& r, float t_min, float t_max, Hit_record& rec) const;
    virtual bool bounding_box(float t0, float t1, Aabb& box) const
    {
        box = Aabb(vec3(x0, k - 0.0001, z0), vec3(x1, k + 0.0001, z1));
        return true;
    }
    Material* mp;
    float x0, x1, z0, z1, k;
};

class YZ_rect : public Hitable
{
  public:
    YZ_rect() {}
    YZ_rect(float _y0, float _y1, float _z0, float _z1, float _k, Material* mat) : y0(_y0), y1(_y1), z0(_z0), z1(_z1), k(_k), mp(mat){};
    virtual bool hit(const Ray& r, float t_min, float t_max, Hit_record& rec) const;
    virtual bool bounding_box(float t0, float t1, Aabb& box) const
    {
        box = Aabb(vec3(k - 0.0001, y0, z0), vec3(k + 0.0001, y1, z1));
        return true;
    }
    Material* mp;
    float y0, y1, z0, z1, k;
};

bool XY_rect::hit(const Ray& r, float t_min, float t_max, Hit_record& rec) const
{
    float t = (k - r.origin().z()) / r.direction().z();
    if (t < t_min || t > t_max) return false;
    float x = r.origin().x() + t * r.direction().x();
    float y = r.origin().y() + t * r.direction().y();
    if (x < x0 || x > x1 || y < y0 || y > y1) return false;
    rec.u = (x - x0) / (x1 - x0);
    rec.v = (y - y0) / (y1 - y0);
    rec.t = t;
    rec.mat_ptr = mp;
    rec.p = r.point_at_parameter(t);
    rec.normal = vec3(0, 0, 1);
    return true;
}

bool XZ_rect::hit(const Ray& r, float t_min, float t_max, Hit_record& rec) const
{
    float t = (k - r.origin().y()) / r.direction().y();
    if (t < t_min || t > t_max) return false;
    float x = r.origin().x() + t * r.direction().x();
    float z = r.origin().z() + t * r.direction().z();
    if (x < x0 || x > x1 || z < z0 || z > z1) return false;
    rec.u = (x - x0) / (x1 - x0);
    rec.v = (z - z0) / (z1 - z0);
    rec.t = t;
    rec.mat_ptr = mp;
    rec.p = r.point_at_parameter(t);
    rec.normal = vec3(0, 1, 0);
    return true;
}
bool YZ_rect::hit(const Ray& r, float t_min, float t_max, Hit_record& rec) const
{
    float t = (k - r.origin().x()) / r.direction().x();
    if (t < t_min || t > t_max) return false;
    float y = r.origin().y() + t * r.direction().y();
    float z = r.origin().z() + t * r.direction().z();
    if (y < y0 || y > y1 || z < z0 || z > z1) return false;
    rec.u = (y - y0) / (y1 - y0);
    rec.v = (z - z0) / (z1 - z0);
    rec.t = t;
    rec.mat_ptr = mp;
    rec.p = r.point_at_parameter(t);
    rec.normal = vec3(1, 0, 0);
    return true;
}
#endif  // AARECTH