#include <stdio.h>
#include <string.h>
#include <chrono>
#include <fstream>
#include <iostream>

// #include <omp.h>
#include "float.h"
#include "include/aarect.h"
#include "include/box.h"
#include "include/bvh_node.h"
#include "include/camera.h"
#include "include/constant_medium.h"
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
        vec3 emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
        float pdf;
        vec3 albedo;
        if (depth < 50 && rec.mat_ptr->scatter(r, rec, albedo, scattered, pdf)) {
            // return emitted + albedo * rec.mat_ptr->scattering_pdf(r, rec, scattered) * color(scattered, world, depth + 1) / pdf;
            vec3 on_light = vec3(213 + drand48() * (343 - 213), 554, 227 + drand48() * (332 - 227));
            vec3 to_light = on_light - rec.p;
            float distance_squared = to_light.squared_length();
            to_light.make_unit_vector();
            if (dot(to_light, rec.normal) < 0) return emitted;
            float light_area = (343 - 213) * (332 - 227);
            float light_cosine = fabs(to_light.y());
            if (light_cosine < 0.000001) return emitted;
            pdf = distance_squared / (light_cosine * light_area);
            scattered = Ray(rec.p, to_light, r.time());
            return emitted + albedo * rec.mat_ptr->scattering_pdf(r, rec, scattered) * color(scattered, world, depth + 1) / pdf;
        } else
            return emitted;
    } else
        return vec3(0, 0, 0);
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

Hitable* simple_light()
{
    Texture* pertext = new Noise_texture(4);
    Hitable** list = new Hitable*[4];
    list[0] = new Sphere(vec3(0, -1000, 0), 1000, new Lambertian(pertext));
    list[1] = new Sphere(vec3(0, 2, 0), 2, new Lambertian(pertext));
    list[2] = new Sphere(vec3(0, 6, 0), 1.5, new Diffuse_light(new Constant_texture(vec3(4, 4, 4))));
    list[3] = new XY_rect(3, 5, 1, 3, -2, new Diffuse_light(new Constant_texture(vec3(4, 4, 4))));
    return new Hitable_list(list, 4);
}

Hitable* cornell_box()
{
    Hitable** list = new Hitable*[8];
    int i = 0;
    Material* red = new Lambertian(new Constant_texture(vec3(0.65, 0.05, 0.05)));
    Material* white = new Lambertian(new Constant_texture(vec3(0.73, 0.73, 0.73)));
    Material* green = new Lambertian(new Constant_texture(vec3(0.12, 0.45, 0.15)));
    Material* light = new Diffuse_light(new Constant_texture(vec3(15, 15, 15)));
    // Add walls and ceil light
    list[i++] = new Flip_normals(new YZ_rect(0, 555, 0, 555, 555, green));
    list[i++] = new YZ_rect(0, 555, 0, 555, 0, red);
    list[i++] = new XZ_rect(213, 343, 227, 332, 554, light);
    list[i++] = new Flip_normals(new XZ_rect(0, 555, 0, 555, 555, white));
    list[i++] = new XZ_rect(0, 555, 0, 555, 0, white);
    list[i++] = new Flip_normals(new XY_rect(0, 555, 0, 555, 555, white));

    // // Add inside boxes
    // list[i++] = new Translate(new Rotate_y(new Box(vec3(0, 0, 0), vec3(165, 165, 165), white), -18), vec3(130, 0, 65));
    // list[i++] = new Translate(new Rotate_y(new Box(vec3(0, 0, 0), vec3(165, 330, 165), white), 15), vec3(265, 0, 295));

    // Add inside boxes
    Hitable* b1 = new Translate(new Rotate_y(new Box(vec3(0, 0, 0), vec3(165, 165, 165), white), -18), vec3(130, 0, 65));
    Hitable* b2 = new Translate(new Rotate_y(new Box(vec3(0, 0, 0), vec3(165, 330, 165), white), 15), vec3(265, 0, 295));
    list[i++] = new Constant_medium(b1, 0.01, new Constant_texture(vec3(1.0, 1.0, 1.0)));
    list[i++] = new Constant_medium(b2, 0.01, new Constant_texture(vec3(0.0, 0.0, 0.0)));

    return new Hitable_list(list, i);
}

Hitable* cornell_box(Hitable** scene, Camera** cam, float aspect)
{
    int i = 0;
    Hitable** list = new Hitable*[8];
    Material* red = new Lambertian(new Constant_texture(vec3(0.65, 0.05, 0.05)));
    Material* white = new Lambertian(new Constant_texture(vec3(0.73, 0.73, 0.73)));
    Material* green = new Lambertian(new Constant_texture(vec3(0.12, 0.45, 0.15)));
    Material* light = new Diffuse_light(new Constant_texture(vec3(15, 15, 15)));
    // Add walls and ceil light
    list[i++] = new Flip_normals(new YZ_rect(0, 555, 0, 555, 555, green));
    list[i++] = new YZ_rect(0, 555, 0, 555, 0, red);
    list[i++] = new XZ_rect(213, 343, 227, 332, 554, light);
    list[i++] = new Flip_normals(new XZ_rect(0, 555, 0, 555, 555, white));
    list[i++] = new XZ_rect(0, 555, 0, 555, 0, white);
    list[i++] = new Flip_normals(new XY_rect(0, 555, 0, 555, 555, white));

    // Add inside boxes
    list[i++] = new Translate(new Rotate_y(new Box(vec3(0, 0, 0), vec3(165, 165, 165), white), -18), vec3(130, 0, 65));
    list[i++] = new Translate(new Rotate_y(new Box(vec3(0, 0, 0), vec3(165, 330, 165), white), 15), vec3(265, 0, 295));

    *scene = new Hitable_list(list, i);
    vec3 lookfrom(278, 278, -800);
    vec3 lookat(278, 278, 0);
    float dist_to_focus = 10.0;
    float aperture = 0.0;
    float vfov = 40;
    *cam = new Camera(lookfrom, lookat, vec3(0, 1, 0), vfov, aspect, aperture, dist_to_focus, 0.0, 1.0);
}

Hitable* final_scene()
{
    int nb = 20;
    Hitable** list = new Hitable*[30];
    Hitable** boxlist = new Hitable*[10000];
    Hitable** boxlist2 = new Hitable*[10000];

    Material* white = new Lambertian(new Constant_texture(vec3(0.73, 0.73, 0.73)));
    Material* ground = new Lambertian(new Constant_texture(vec3(0.48, 0.83, 0.53)));

    // Ground
    int b = 0;
    for (int i = 0; i < nb; i++) {
        for (int j = 0; j < nb; j++) {
            float w = 100;
            float x0 = -1000 + i * w;
            float y0 = 0;
            float z0 = -1000 + j * w;
            float x1 = x0 + w;
            float y1 = 100 * (drand48() + 0.01);
            float z1 = z0 + w;
            boxlist[b++] = new Box(vec3(x0, y0, z0), vec3(x1, y1, z1), ground);
        }
    }
    int l = 0;
    list[l++] = new Bvh_node(boxlist, b, 0, 1);

    // The rest
    Material* light = new Diffuse_light(new Constant_texture(vec3(7, 7, 7)));
    list[l++] = new XZ_rect(123, 423, 147, 412, 554, light);
    vec3 center(400, 400, 200);
    list[l++] = new Moving_sphere(center, center + vec3(30, 0, 0), 0, 1, 50, new Lambertian(new Constant_texture(vec3(0.7, 0.3, 0.1))));
    list[l++] = new Sphere(vec3(260, 150, 45), 50, new Dielectric(1.5));
    list[l++] = new Sphere(vec3(0, 150, 145), 50, new Metal(vec3(0.8, 0.8, 0.8), 10.0));
    Hitable* boundary = new Sphere(vec3(360, 150, 145), 70, new Dielectric(1.5));
    list[l++] = boundary;
    list[l++] = new Constant_medium(boundary, 0.2, new Constant_texture(vec3(0.2, 0.4, 0.9)));

    boundary = new Sphere(vec3(0, 0, 0), 5000, new Dielectric(1.5));
    list[l++] = new Constant_medium(boundary, 0.0001, new Constant_texture(vec3(1.0, 1.0, 1.0)));

    int nx, ny, nn;
    unsigned char* tex_data = stbi_load("assets/earth.jpg", &nx, &ny, &nn, 0);
    Material* emat = new Lambertian(new Image_texture(tex_data, nx, ny));
    list[l++] = new Sphere(vec3(400, 200, 400), 100, emat);
    Texture* pertext = new Noise_texture(0.1);
    list[l++] = new Sphere(vec3(220, 280, 300), 80, new Lambertian(pertext));

    int ns = 1000;
    for (int j = 0; j < ns; j++) {
        boxlist2[j] = new Sphere(vec3(165 * drand48(), 165 * drand48(), 165 * drand48()), 10, white);
    }
    list[l++] = new Translate(new Rotate_y(new Bvh_node(boxlist2, ns, 0.0, 1.0), 15), vec3(-100, 270, 395));
    return new Hitable_list(list, l);
}
int main()
{
#if 1
    int nx = 500;
    int ny = 500;
    int ns = 10;
#else
    int nx = 1920;  // 1200;
    int ny = 1080;  // 600;
    int ns = 600;   // 200;

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
#elif 0
    Hitable* world = random_scene();

    vec3 lookfrom(13, 2, 3);
    vec3 lookat(0, 0, 0);
    float dist_to_focus = 10.0;
    float aperture = 0.1;
    Camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 1.0);
#elif 0
    Hitable* world = two_spheres();

    vec3 lookfrom(13, 2, 3);
    vec3 lookat(0, 0, 0);
    float dist_to_focus = 10.0;
    float aperture = 0.0;
    Camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 1.0);

#elif 0
    Hitable* world = two_perlin_spheres();

    vec3 lookfrom(13, 2, 3);
    vec3 lookat(0, 0, 0);
    float dist_to_focus = 10.0;
    float aperture = 0.0;
    Camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 1.0);
#elif 0
    Hitable* world = two_earths();

    vec3 lookfrom(13, 2, 3);
    vec3 lookat(0, 0, 0);
    float dist_to_focus = 10.0;
    float aperture = 0.0;
    Camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 1.0);
#elif 0
    Hitable* world = simple_light();

    vec3 lookfrom(13, 2, 3);
    vec3 lookat(0, 1, 0);
    float dist_to_focus = 10.0;
    float aperture = 0.0;
    Camera cam(lookfrom, lookat, vec3(0, 1.5, 0), 40, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 1.0);
#elif 0
    Hitable* world = cornell_box();
    vec3 lookfrom(278, 278, -800);
    vec3 lookat(278, 278, 0);
    float dist_to_focus = 10.0;
    float aperture = 0.0;
    float vfov = 35;

    Camera cam(lookfrom, lookat, vec3(0, 1, 0), vfov, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 1.0);
#elif 0
    Hitable* world = final_scene();
    vec3 lookfrom(278, 278, -800);
    vec3 lookat(278, 278, 0);
    float dist_to_focus = 10.0;
    float aperture = 0.0;
    float vfov = 35;

    Camera cam(lookfrom, lookat, vec3(0, 1, 0), vfov, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 1.0);

#else

    Hitable* world;
    Camera* cam;
    cornell_box(&world, &cam, float(nx) / float(ny));

#endif

    int size_img_tab = 3 * nx * ny;

    int* img_tab = new int[size_img_tab];
    memset(img_tab, 0, size_img_tab);

#ifdef MONITOR_TIME
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    start = std::chrono::high_resolution_clock::now();
#endif
    int jref = 0;

#pragma omp parallel for collapse(2) num_threads(4)
    for (int j = ny - 1; j >= 0; j--) {
        for (int i = 0; i < nx; i++) {
            vec3 col(0, 0, 0);
            for (int s = 0; s < ns; s++) {
                float u = float(i + drand48()) / float(nx);
                float v = float(j + drand48()) / float(ny);
                Ray r = cam->get_ray(u, v);
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