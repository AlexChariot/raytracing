#include <stdio.h>
#include <string.h>
#include <chrono>
#include <fstream>
#include <iostream>

// #include <omp.h>

#include "float.h"
#include "include/bvh_node.h"
#include "include/camera.h"
#include "include/hitablelist.h"
#include "include/material.h"
#include "include/moving_sphere.h"
#include "include/perlin.h"
#include "include/sphere.h"

#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/stb_image_write.h"

#define MONITOR_TIME

/*
 * Compute final color of a pixel
 */
vec3 color(const Ray& r, Hitable* world, int depth)
{
    Hit_record rec;
    if (world->hit(r, 0.001, FLT_MAX, rec)) {
        Ray scattered;
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
    Texture* checker = new Checker_texture(new Constant_texture(vec3(0.2, 0.3, 0.1)), new Constant_texture(vec3(0.9, 0.9, 0.9)));

    list[0] = new Sphere(vec3(0, -1000, 0), 1000, new Lambertian(checker));
    int i = 1;
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = drand48();
            vec3 center(a + 0.9 * drand48(), 0.2, b + 0.9 * drand48());
            if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
                if (choose_mat < 0.8) {  // diffuse
                    list[i++] = new Moving_sphere(
                        center, center + vec3(0, 0.5 * drand48(), 0), 0.0, 1.0, 0.2,
                        new Lambertian(new Constant_texture(vec3(drand48() * drand48(), drand48() * drand48(), drand48() * drand48()))));
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
    list[i++] = new Sphere(vec3(-4, 1, 0), 1.0, new Lambertian(new Constant_texture(vec3(0.4, 0.2, 0.1))));
    list[i++] = new Sphere(vec3(4, 1, 0), 1.0, new Metal(vec3(0.7, 0.6, 0.5), 0.0));

    return new Hitable_list(list, i);
}

Hitable* two_spheres()
{
    Texture* checker = new Checker_texture(new Constant_texture(vec3(0.2, 0.3, 0.1)), new Constant_texture(vec3(0.9, 0.9, 0.9)));
    int n = 50;
    Hitable** list = new Hitable*[n + 1];
    list[0] = new Sphere(vec3(0, -10, 0), 10, new Lambertian(checker));
    list[1] = new Sphere(vec3(0, 10, 0), 10, new Lambertian(checker));

    return new Hitable_list(list, 2);
}

Hitable* two_perlin_spheres()
{
    Texture* pertext = new Noise_texture(4);
    Hitable** list = new Hitable*[2];
    list[0] = new Sphere(vec3(0, -1000, 0), 1000, new Lambertian(pertext));
    list[1] = new Sphere(vec3(0, 2, 0), 2, new Lambertian(pertext));
    return new Hitable_list(list, 2);
}

Hitable* two_earths()
{
    int nx, ny, nn;

    unsigned char* pixels = stbi_load("assets/earth.jpg", &nx, &ny, &nn, 0);

    Texture* earth_tex = new Image_texture(pixels, nx, ny);

    Hitable** list = new Hitable*[2];
    list[0] = new Sphere(vec3(0, -15, 0), 15, new Lambertian(earth_tex));
    list[1] = new Sphere(vec3(0, 1, 0), 1, new Lambertian(earth_tex));
    return new Hitable_list(list, 2);
}

int main()
{
#if 1
    int nx = 400;
    int ny = 200;
    int ns = 100;
#else
    int nx = 1920;  // 1200;
    int ny = 1080;  // 600;
    int ns = 200;   // 200;

#endif

#if 0
    Hitable* list[5];
    float t0 = 0.0;
    float t1 = 1.0;
    vec3 center(0, 0, -1);
    list[0] = new Moving_sphere(center, center + vec3(0, 0.2, 0), t0, t1, 0.5, new Lambertian(new Constant_texture(vec3(0.1, 0.2, 0.5))));
    // list[0] = new Sphere(vec3(0, 0, -1), 0.5, new Lambertian(new Constant_texture( vec3(0.1, 0.2, 0.5))));
    list[1] = new Sphere(vec3(0, -100.5, -1), 100, new Lambertian(new Constant_texture(vec3(0.8, 0.8, 0.0))));
    list[2] = new Sphere(vec3(-1, 0, -1), 0.5, new Metal(vec3(0.8, 0.6, 0.2), 0.1));
    list[3] = new Sphere(vec3(1, 0, -1), 0.5, new Dielectric(1.5));
    list[4] = new Sphere(vec3(1, 0, -1), -0.45, new Dielectric(1.5));
    Hitable* world = new Hitable_list(list, 5);

    vec3 lookfrom(3, 3, 2);
    vec3 lookat(0, 0, -1);
    float dist_to_focus = (lookfrom - lookat).length();
    float aperture = 0.1;
    Camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 0.1);
#else
#if 0
    Hitable* world = random_scene();

    vec3 lookfrom(13, 2, 3);
    vec3 lookat(0, 0, 0);
    float dist_to_focus = 10.0;
    float aperture = 0.1;
    Camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 1.0);
#else
#if 0
    Hitable* world = two_spheres();

    vec3 lookfrom(13, 2, 3);
    vec3 lookat(0, 0, 0);
    float dist_to_focus = 10.0;
    float aperture = 0.0;
    Camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 1.0);

#else
#if 0
    Hitable* world = two_perlin_spheres();

    vec3 lookfrom(13, 2, 3);
    vec3 lookat(0, 0, 0);
    float dist_to_focus = 10.0;
    float aperture = 0.0;
    Camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 1.0);
#else
    Hitable* world = two_earths();

    vec3 lookfrom(13, 2, 3);
    vec3 lookat(0, 0, 0);
    float dist_to_focus = 10.0;
    float aperture = 0.0;
    Camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 1.0);

#endif

#endif

#endif
#endif

    int size_img_tab = 3 * nx * ny;

    int* img_tab = new int[size_img_tab];
    memset(img_tab, 0, size_img_tab);

#ifdef MONITOR_TIME
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    start = std::chrono::high_resolution_clock::now();
#endif

#pragma omp parallel for collapse(2) num_threads(4)
    for (int j = ny - 1; j >= 0; j--) {
        for (int i = 0; i < nx; i++) {
            vec3 col(0, 0, 0);
            for (int s = 0; s < ns; s++) {
                float u = float(i + drand48()) / float(nx);
                float v = float(j + drand48()) / float(ny);
                Ray r = cam.get_ray(u, v);
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
            // file_pgm << ir << " " << ig << " " << ib << "\n";

            int jj = -j + ny - 1;
            int ii = i;
            int index = 3 * (jj * nx + ii);
            img_tab[index] = ir;
            img_tab[index + 1] = ig;
            img_tab[index + 2] = ib;

        }  // i
    }      // j

#ifdef MONITOR_TIME
    end = std::chrono::high_resolution_clock::now();
    std::cout << "---TOTAL RENDERING TIME--- : " << std::chrono::duration<float>(end - start).count() << "s" << std::endl;
    start = std::chrono::high_resolution_clock::now();
#endif

    // File writing
    std::ofstream file_pgm;
    file_pgm.open("test.pgm");
    file_pgm << "P3\n" << nx << " " << ny << "\n255\n";
    for (int i = 0; i < nx * ny; i++) {
        // std::cout << img_tab[3 * i] << " " << img_tab[3 * i + 1] << " " << img_tab[3 * i + 2] << "\n";
        file_pgm << img_tab[3 * i] << " " << img_tab[3 * i + 1] << " " << img_tab[3 * i + 2] << "\n";
    }
    file_pgm.close();

#ifdef MONITOR_TIME
    end = std::chrono::high_resolution_clock::now();
    std::cout << "---TOTAL WRITING TIME--- : " << std::chrono::duration<float>(end - start).count() << "s" << std::endl;
#endif

    delete[] img_tab;
    return 0;
}