#ifndef MOVINGSPHEREH
#define MOVINGSPHEREH

#include "hitable.h"
#include "ray.h"

class Moving_sphere : public Hitable
{
  public:
    Moving_sphere();
    Moving_sphere(vec3 cen0, vec3 cen1, float t0, float t1, float r, Material* m)
      : center0(cen0), center1(cen1), time0(t0), time1(t1), radius(r), mat_ptr(m){};
    virtual bool hit(const Ray& r, float tmin, float tmax, Hit_record& rec) const;
    bool bounding_box(float t0, float t1, Aabb& box) const;
    vec3 center(float time) const;
    vec3 center0, center1;
    float time0, time1;
    float radius;
    Material* mat_ptr;
};

vec3 Moving_sphere::center(const float time) const { return center0 + ((time - time0) / (time1 - time0)) * (center1 - center0); }
bool Moving_sphere::hit(const Ray& r, float t_min, float t_max, Hit_record& rec) const
{
    vec3 oc = r.origin() - center(r.time());
    float a = dot(r.direction(), r.direction());
    float b = dot(oc, r.direction());
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - a * c;
    if (discriminant > 0) {
        float temp = (-b - sqrt(discriminant)) / a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.p = r.point_at_parameter(rec.t);
            rec.normal = (rec.p - center(r.time())) / radius;
            rec.mat_ptr = mat_ptr;
            return true;
        }
        temp = (-b + sqrt(discriminant)) / a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.p = r.point_at_parameter(rec.t);
            rec.normal = (rec.p - center(r.time())) / radius;
            rec.mat_ptr = mat_ptr;
            return true;
        }
    }
    return false;
}

bool Moving_sphere::bounding_box(float t0, float t1, Aabb& box) const
{
    Aabb box0(center(t0) - vec3(radius, radius, radius), center(t0) + vec3(radius, radius, radius));
    Aabb box1(center(t1) - vec3(radius, radius, radius), center(t1) + vec3(radius, radius, radius));
    box = surrounding_box(box0, box1);
    return true;
}
#endif