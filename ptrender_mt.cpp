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

#include <string>
#include <Windows.h>
#include <process.h>

using namespace std;

int totalpx;
int currpx;

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
	texture *worldtex = new constant_texture(vec3(0.4, 0, 0.8));
    int n = 500;
    hitable **list = new hitable*[n+1];
    list[0] = new sphere(vec3(0,-1000,0), 1000, new lambertian(worldtex));
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

//Array of strings for the output of all the threads
string threadOutput[12];

struct renderThread_args {
    renderThread_args(int tid, int w, int h, int s, float camx, float camy, float camz) {thread_id = tid; width = w; height = h; spp = s; camera_x = camx; camera_y = camy; camera_z = camz;}

    int getWidth() {
        return width;
    }

    int getHeight() {
        return height;
    }

    int getTID() {
        return thread_id;
    }

    int getSPP() {
        return spp;
    }

    float getCamX() {
        return camera_x;
    }

    float getCamY() {
        return camera_y;
    }

    float getCamZ() {
        return camera_z;
    }

    int thread_id;
    int width;
    int height;
    int spp;
    float camera_x;
    float camera_y;
    float camera_z;
};

hitable *world;

unsigned int __stdcall renderThread(void* rtargs) {
    using namespace std;
    renderThread_args* targs = (renderThread_args*)rtargs;

    int tnx = targs->getWidth();
    int tny = targs->getHeight();
    int tns = targs->getSPP();
    int tid = targs->getTID();
    float camerax = targs->getCamX();
    float cameraz = targs->getCamZ();
    
    //printf("Starting thread %i width %i height %i spp %i\n", tid, tnx, tny, tns);

    int iny = tny*12;

    int yoff = tny + tny*tid;

    //Code for rendering
    vec3 lookfrom(camerax,3,cameraz);
    vec3 lookat (0,0,-1);
    float dist_to_focus = (lookfrom-lookat).length();
    float aperture = 0.0;
    camera cam(lookfrom, lookat, vec3(0,1,0), 20, float(tnx)/float(iny), aperture, dist_to_focus);
    for (int j = iny - 1; j >= 0; j--){
        for (int i = 0; i < tnx; i++) {
            vec3 col(0, 0, 0);
            for (int s = 0; s < tns; s++) {
                float u = float(i + drand48()) / float(tnx);
                float v = float(j + tid*iny + drand48()) / float(iny);
                ray r = cam.get_ray(u, v);
                vec3 p = r.point_at_parameter(2.0);
                col += color(r, world, 0);
            }
            col/= float(tns);
            col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
            int ir = int(255.99*col[0]);
            int ig = int(255.99*col[1]);
            int ib = int(255.99*col[2]);
            threadOutput[tid] = threadOutput[tid] + to_string(ir) + " " + to_string(ig) + " " + to_string(ib) + "\n";
            currpx ++;
            float cpx = currpx;
            float tpx = totalpx;
            float percentage = (cpx*100)/tpx;
            printf("Render at %.4f percent\n", percentage);
        }
    }

    return 0;
}

int main() {
    int nx;
    int ny;
    int ns;
    int nf;
    cout << "Define image width: ";
    cin >> nx;
    cout << "Define image height (multiple of 12): ";
    cin >> ny;
    cout << "Define image spp: ";
    cin >> ns;
    cout << "Define amount of frames: ";
    cin >> nf;
    totalpx = nx * ny * 12 * nf;

    world = random_scene();
    for (int f = 0; f < nf; f++) {

        for (int d = 0; d < 12; d++) {
            threadOutput[d] = "";
        }

        std::ofstream file("./frames/output" + to_string(f) + ".ppm");
        file << "P3\n" << nx << " " << ny << "\n255\n";
        float startx = 20;
        float maxz = 20;

        float cframe = f;
        float maxframes = nf;

        float xrot = cos((cframe/maxframes)*2*3.14159265358979323846);
        float zrot = sin((cframe/maxframes)*2*3.14159265358979323846);

        float posx = startx * xrot;
        float posz = maxz * zrot;

        //printf("Frame %i %f %f", f, posx, posz);

        HANDLE renderThreadHandler[12];

            renderThread_args targs0(0, nx, ny/12, ns, posx, 3, posz);
            renderThread_args targs1(1, nx, ny/12, ns, posx, 3, posz);
            renderThread_args targs2(2, nx, ny/12, ns, posx, 3, posz);
            renderThread_args targs3(3, nx, ny/12, ns, posx, 3, posz);
            renderThread_args targs4(4, nx, ny/12, ns, posx, 3, posz);
            renderThread_args targs5(5, nx, ny/12, ns, posx, 3, posz);
            renderThread_args targs6(6, nx, ny/12, ns, posx, 3, posz);
            renderThread_args targs7(7, nx, ny/12, ns, posx, 3, posz);
            renderThread_args targs8(8, nx, ny/12, ns, posx, 3, posz);
            renderThread_args targs9(9, nx, ny/12, ns, posx, 3, posz);
            renderThread_args targs10(10, nx, ny/12, ns, posx, 3, posz);
            renderThread_args targs11(11, nx, ny/12, ns, posx, 3, posz);

            renderThreadHandler[0] = (HANDLE)_beginthreadex(NULL, 0, &renderThread, &targs0, 0, NULL);
            renderThreadHandler[1] = (HANDLE)_beginthreadex(NULL, 0, &renderThread, &targs1, 0, NULL);
            renderThreadHandler[2] = (HANDLE)_beginthreadex(NULL, 0, &renderThread, &targs2, 0, NULL);
            renderThreadHandler[3] = (HANDLE)_beginthreadex(NULL, 0, &renderThread, &targs3, 0, NULL);
            renderThreadHandler[4] = (HANDLE)_beginthreadex(NULL, 0, &renderThread, &targs4, 0, NULL);
            renderThreadHandler[5] = (HANDLE)_beginthreadex(NULL, 0, &renderThread, &targs5, 0, NULL);
            renderThreadHandler[6] = (HANDLE)_beginthreadex(NULL, 0, &renderThread, &targs6, 0, NULL);
            renderThreadHandler[7] = (HANDLE)_beginthreadex(NULL, 0, &renderThread, &targs7, 0, NULL);
            renderThreadHandler[8] = (HANDLE)_beginthreadex(NULL, 0, &renderThread, &targs8, 0, NULL);
            renderThreadHandler[9] = (HANDLE)_beginthreadex(NULL, 0, &renderThread, &targs9, 0, NULL);
            renderThreadHandler[10] = (HANDLE)_beginthreadex(NULL, 0, &renderThread, &targs10, 0, NULL);
            renderThreadHandler[11] = (HANDLE)_beginthreadex(NULL, 0, &renderThread, &targs11, 0, NULL);

            WaitForMultipleObjects(12, renderThreadHandler, true, INFINITE);

            for (int n = 0; n < 12; n++) {
                CloseHandle(renderThreadHandler[n]);
            }

            for (int i = 0; i < 12; i++) {
                file << threadOutput[i];
            }

            file.close();
    }
    return 0;
}

/* old int main():
int main()
{
    ofstream outfile;
    outfile.open ("output.ppm");
    int nx;
    int ny;
    int ns;
    cout << "Define image width: ";
    cin >> nx;
    cout << "Define image height (multiple of 12): ";
    cin >> ny;
    cout << "Define image spp: ";
    cin >> ns;
    outfile << "P3\n" << nx << " " << ny << "\n255\n";
    int pcount = 0;
    int area = nx * ny;

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
            pcount ++;
            cout << "Rendered pixel " <<pcount << " / " << area <<endl;
        }
    }
    outfile.close();
}
*/