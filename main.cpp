#include <chrono>
#include <fstream>
#include <iostream>

#include "float.h"
#include "include/camera.h"
#include "include/hitablelist.h"
#include "include/material.h"
#include "include/moving_sphere.h"
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

Hitable* random_scene()
{
    int n = 500;
    Hitable** list = new Hitable*[n + 1];
    list[0] = new Sphere(vec3(0, -1000, 0), 1000, new Lambertian(vec3(0.5, 0.5, 0.5)));
    int i = 1;
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = drand48();
            vec3 center(a + 0.9 * drand48(), 0.2, b + 0.9 * drand48());
            if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
                if (choose_mat < 0.8) {  // diffuse
                    list[i++] = new Moving_sphere(center, center + vec3(0, 0.5 * drand48(), 0), 0.0, 1.0, 0.2,
                                                  new Lambertian(vec3(drand48() * drand48(), drand48() * drand48(), drand48() * drand48())));
                } else if (choose_mat < 0.95) {  // metal
                    list[i++] = new Sphere(center, 0.2,
                                           new Metal(vec3(0.5 * (1 + drand48()), 0.5 * (1 + drand48()), 0.5 * (1 + drand48())), 0.5 * drand48()));
                } else {  // glass
                    list[i++] = new Sphere(center, 0.2, new Dielectric(1.5));
                }
            }
        }
    }

    list[i++] = new Sphere(vec3(0, 1, 0), 1.0, new Dielectric(1.5));
    list[i++] = new Sphere(vec3(-4, 1, 0), 1.0, new Lambertian(vec3(0.4, 0.2, 0.1)));
    list[i++] = new Sphere(vec3(4, 1, 0), 1.0, new Metal(vec3(0.7, 0.6, 0.5), 0.0));

    return new Hitable_list(list, i);
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
    int nx = 1920;  // 1200;
    int ny = 1080;  // 600;
    int ns = 400;   // 200;

#endif
    file_pgm << "P3\n" << nx << " " << ny << "\n255\n";

#if 1
    Hitable* list[5];
    float t0 = 0.0;
    float t1 = 1.0;
    vec3 center(0, 0, -1);
    list[0] = new Moving_sphere(center, center + vec3(0, 0.2, 0), t0, t1, 0.5, new Lambertian(vec3(0.1, 0.2, 0.5)));
    // list[0] = new Sphere(vec3(0, 0, -1), 0.5, new Lambertian(vec3(0.1, 0.2, 0.5)));
    list[1] = new Sphere(vec3(0, -100.5, -1), 100, new Lambertian(vec3(0.8, 0.8, 0.0)));
    list[2] = new Sphere(vec3(-1, 0, -1), 0.5, new Metal(vec3(0.8, 0.6, 0.2), 0.1));
    list[3] = new Sphere(vec3(1, 0, -1), 0.5, new Dielectric(1.5));
    list[4] = new Sphere(vec3(1, 0, -1), -0.45, new Dielectric(1.5));
    Hitable* world = new Hitable_list(list, 5);

    vec3 lookfrom(3, 3, 2);
    vec3 lookat(0, 0, -1);
    float dist_to_focus = (lookfrom - lookat).length();
    float aperture = 0.1;
    Camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 1.0);
#else
    Hitable* world = world = random_scene();

    vec3 lookfrom(13, 2, 3);
    vec3 lookat(0, 0, 0);
    float dist_to_focus = 10.0;
    float aperture = 0.1;
    Camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 1.0);
#endif

#ifdef MONITOR_TIME
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    start = std::chrono::high_resolution_clock::now();
#endif

    for (int j = ny - 1; j >= 0; j--) {
        for (int i = 0; i < nx; i++) {
            vec3 col(0, 0, 0);
            // #pragma omp parallel for
            for (int s = 0; s < ns; s++) {
                float u = float(i + drand48()) / float(nx);
                float v = float(j + drand48()) / float(ny);
                ray r = cam.get_ray(u, v);
                vec3 p = r.point_at_parameter(2.0);
                // #pragma omp critical
                {
                    col += color(r, world, 0);
                }
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
    std::cout << "---TOTAL RENDERING TIME--- : " << std::chrono::duration<float>(end - start).count() << "s" << std::endl;
#endif

    return 0;
}