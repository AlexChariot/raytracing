#include <chrono>
#include <fstream>
#include <iostream>

#include "float.h"
#include "include/camera.h"
#include "include/hitablelist.h"
#include "include/material.h"
#include "include/sphere.h"

#define MONITOR_TIME

vec3 color(const ray& r, Hitable* world, int depth)
{
    Hit_record rec;
    if (world->hit(r, 0.001, FLT_MAX, rec)) {
        ray scattered;
        vec3 attenuation;
        if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
            return attenuation * color(scattered, world, depth + 1);
        } else {
            return vec3(0, 0, 0);
        }
    } else {
        vec3 unit_direction = unit_vector(r.direction());
        float t = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
    }
}

int main()
{
    std::ofstream file_pgm;
    file_pgm.open("test.pgm");

#if 1
    int nx = 400;
    int ny = 200;
    int ns = 100;
#else
    int nx = 1200;
    int ny = 600;
    int ns = 200;
#endif
    file_pgm << "P3\n" << nx << " " << ny << "\n255\n";
    Hitable* list[5];
    list[0] = new Sphere(vec3(0, 0, -1), 0.5, new Lambertian(vec3(0.1, 0.2, 0.5)));
    list[1] = new Sphere(vec3(0, -100.5, -1), 100, new Lambertian(vec3(0.8, 0.8, 0.0)));
    list[2] = new Sphere(vec3(1, 0, -1), 0.5, new Metal(vec3(0.8, 0.6, 0.2), 0.1));
    list[3] = new Sphere(vec3(-1, 0, -1), 0.5, new Dielectric(1.5));
    list[4] = new Sphere(vec3(-1, 0, -1), -0.45, new Dielectric(1.5));
    Hitable* world = new Hitable_list(list, 5);

    vec3 lookfrom(3, 3, 2);
    vec3 lookat(0, 0, -1);
    float dist_to_focus = (lookfrom - lookat).length();
    float aperture = 2.0;
    Camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, dist_to_focus);

#ifdef MONITOR_TIME
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    start = std::chrono::high_resolution_clock::now();
#endif

    for (int j = ny - 1; j >= 0; j--) {
        for (int i = 0; i < nx; i++) {
            vec3 col(0, 0, 0);
#pragma omp parallel for
            for (int s = 0; s < ns; s++) {
                float u = float(i + drand48()) / float(nx);
                float v = float(j + drand48()) / float(ny);
                ray r = cam.get_ray(u, v);
                vec3 p = r.point_at_parameter(2.0);
                col += color(r, world, 0);
            }
            col /= float(ns);
            col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
            int ir = int(255.99 * col[0]);
            int ig = int(255.99 * col[1]);
            int ib = int(255.99 * col[2]);
            file_pgm << ir << " " << ig << " " << ib << "\n";
        }
    }
    file_pgm.close();

#ifdef MONITOR_TIME
    end = std::chrono::high_resolution_clock::now();
    std::cout << "---TOTAL RENDERING TIME--- : " << std::chrono::duration<float>(end - start).count() * 1000 << "ms" << std::endl;
#endif

    return 0;
}