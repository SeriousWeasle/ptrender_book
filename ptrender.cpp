#include <iostream>
#include <fstream>
#include "float.h"
#include "./headers/drand.h"
#include "./headers/sphere.h"
#include "./headers/hitable_list.h"
#include "./headers/material.h"
#include "./headers/camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include "./headers/stb_image.h"

using namespace std;

vec3 color(const ray& r, hitable *world, int depth) {
    hit_record rec;

    if (world->hit(r, 0.0, FLT_MAX, rec)) {
        ray scattered;
        vec3 attenuation;
        if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
            return attenuation*color(scattered,world,depth+1);
        }
        else {
            return vec3(0,0,0);
        }
    }
    else {
        vec3 unit_direction = unit_vector(r.direction());
        float t = 0.5*(unit_direction.y() + 1.0);
        return (1.0-t)*vec3(1.0,1.0,1.0) + t*vec3(0.5,0.7,1.0);
    }
}

hitable *random_scene() {
	texture *checker = new checker_texture(new constant_texture(vec3(1.0,0.0,0.0)), new constant_texture(vec3(0.0,0.0,1.0)));
    int n = 500;
    hitable **list = new hitable*[n+1];
    list[0] = new sphere(vec3(0,-1000,0), 1000, new lambertian(checker));
    int i = 1;
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = drand48();
            vec3 center(a+0.9*drand48(),0.2, b+0.9*drand48());
            if((center-vec3(4,0.2,0)).length() > 0.9) {
                if (choose_mat < 0.8) {
					texture *rand_ctexture = new constant_texture(vec3(drand48()*drand48(), drand48()*drand48(), drand48()*drand48()));
                    list[i++] = new sphere(center, 0.2, new lambertian(rand_ctexture));
                }
                else if (choose_mat <0.95) {
                    list[i++] = new sphere(center, 0.2, new metal(vec3(0.5*(1+drand48()), 0.5*(1+drand48()), 0.5*(1+drand48())), 0.5*drand48()));
                }
                else {
                    list[i++] = new sphere(center, 0.2, new dielectric(1.5));
                }
            }
        }
    }
    list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
	texture *tex3 = new constant_texture(vec3(0.4, 0.2, 0.1));
	/*int nx, ny, nn;
	unsigned char *tex_data = stbi_load("Kunst.png", &nx, &ny, &nn, 0);
	material *mat = new lambertian(new image_texture(tex_data, nx, ny));*/
    list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(tex3));
    list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));

    return new hitable_list(list, i);
}

hitable *spheres() {
    hitable **list = new hitable*[5];
	texture *tex1 = new constant_texture(vec3(0.8, 0.3, 0.3));
	texture *tex2 = new constant_texture(vec3(0.4, 0, 0.8));
    list[0] = new sphere(vec3(0,0,-1), 0.5, new lambertian(tex1));
    list[1] = new sphere(vec3(0,-100.5,-1),100, new lambertian(tex2));
    list[2] = new sphere(vec3(1,0,-1), 0.5, new metal(vec3(0.8,0.6,0.2), 0.0));
    list[3] = new sphere(vec3(-1,0,-1), 0.5, new dielectric(1.5));
    list[4] = new sphere(vec3(-1,0,-1), -0.45, new dielectric(1.5));
    return new hitable_list(list,5);
}

int main()
{
    ofstream outfile;
    outfile.open ("output.ppm");
    int nx;
    int ny;
    int ns;
    cout << "Define image width: ";
    cin >> nx;
    cout << "Define image height: ";
    cin >> ny;
    cout << "Define image spp: ";
    cin >> ns;
    outfile << "P3\n" << nx << " " << ny << "\n255\n";
    hitable *world;
    world = random_scene();
    vec3 lookfrom(20,3,15);
    vec3 lookat (0,0,-1);
    float dist_to_focus = (lookfrom-lookat).length();
    float aperture = 0.0;
    camera cam(lookfrom, lookat, vec3(0,1,0), 20, float(nx)/float(ny), aperture, dist_to_focus);
    for (int j = ny - 1; j >= 0; j--){
        for (int i = 0; i < nx; i++) {
            vec3 col(0, 0, 0);
            for (int s = 0; s < ns; s++) {
                float u = float(i + drand48()) / float(nx);
                float v = float(j + drand48()) / float(ny);
                ray r = cam.get_ray(u, v);
                vec3 p = r.point_at_parameter(2.0);
                col += color(r, world, 0);
            }
            col/= float(ns);
            col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
            int ir = int(255.99*col[0]);
            int ig = int(255.99*col[1]);
            int ib = int(255.99*col[2]);
            outfile << ir << " " << ig << " " << ib << "\n";
            cout << "Rendering pixel " << i+1 <<", " << ny-j <<endl;
        }
    }
    outfile.close();
}