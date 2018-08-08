#ifndef SPHEREH
#define SPHEREH

#include "hitable.h"

class Sphere : public Hitable
{
  public:
    Sphere() {}
    Sphere(vec3 cen, float r, Material* m) : center(cen), radius(r), mat_ptr(m){};
    virtual bool hit(const ray& r, float tmin, float tmax, Hit_record& rec) const;
    vec3 center;
    float radius;
    Material* mat_ptr;
};

bool Sphere::hit(const ray& r, float t_min, float t_max, Hit_record& rec) const
{
    vec3 oc = r.origin() - center;
    float a = dot(r.direction(), r.direction());
    float b = dot(oc, r.direction());
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - a * c;
    if (discriminant > 0) {
        float temp = (-b - sqrt(b * b - a * c)) / a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.p = r.point_at_parameter(rec.t);
            rec.normal = (rec.p - center) / radius;
            rec.mat_ptr = mat_ptr;
            return true;
        }
        temp = (-b + sqrt(b * b - a * c)) / a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.p = r.point_at_parameter(rec.t);
            rec.normal = (rec.p - center) / radius;
            rec.mat_ptr = mat_ptr;
            return true;
        }
    }
    return false;
}

#endif