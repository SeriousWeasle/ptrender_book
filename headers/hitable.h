#ifndef HITABLEH
#define HITABLEH

#include "ray.h"

class material;

/*void get_sphere_uv(const vec3& p, float& u, float& v) {
	float phi = atan2(p.z(), p.x());
	float theta = asin(p.y());
	u = 1 - (phi + 3.14159265358979323846) / (2 * 3.14159265358979323846);
	v = (theta + 3.14159265358979323846 / 2) / 3.14159265358979323846;
}*/

struct hit_record {
    float t;
	//float u;
	//float v;
    vec3 p;
    vec3 normal;
    material *mat_ptr;
};

class hitable {
    public:
        virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const = 0;
};

#endif