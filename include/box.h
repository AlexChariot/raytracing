#ifndef BOXH
#define BOXH

#include "aarect.h"
#include "hitable.h"
#include "hitablelist.h"

class Box : public Hitable
{
  public:
    Box() {}
    Box(const vec3& p0, const vec3& p1, Material* ptr);
    virtual bool hit(const Ray& r, float t_min, float t_max, Hit_record& rec) const;
    virtual bool bounding_box(float t0, float t1, Aabb& box) const
    {
        box = Aabb(pmin, pmax);
        return true;
    }
    vec3 pmin, pmax;
    Hitable* list_ptr;
};

Box::Box(const vec3& p0, const vec3& p1, Material* ptr)
{
    pmin = p0;
    pmax = p1;
    Hitable** list = new Hitable*[6];
    unsigned int i = 0;
    list[i++] = new XY_rect(p0.x(), p1.x(), p0.y(), p1.y(), p1.z(), ptr);
    list[i++] = new Flip_normals(new XY_rect(p0.x(), p1.x(), p0.y(), p1.y(), p0.z(), ptr));
    list[i++] = new XZ_rect(p0.x(), p1.x(), p0.z(), p1.z(), p1.y(), ptr);
    list[i++] = new Flip_normals(new XZ_rect(p0.x(), p1.x(), p0.z(), p1.z(), p0.y(), ptr));
    list[i++] = new YZ_rect(p0.y(), p1.y(), p0.z(), p1.z(), p1.x(), ptr);
    list[i++] = new Flip_normals(new YZ_rect(p0.y(), p1.y(), p0.z(), p1.z(), p0.x(), ptr));
    list_ptr = new Hitable_list(list, i);
}
bool Box::hit(const Ray& r, float t_min, float t_max, Hit_record& rec) const { return list_ptr->hit(r, t_min, t_max, rec); }
#endif  // !BOXH