#ifndef CUBEH
#define SPHEREH

#include "hitable.h"

class cube: public hitable {
    public:
        cube() {}
        cube(vec3 cen, vec3 size, material *m): center(cen), size(size), mat_ptr(m){};
        virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec) const;
        vec3 center;
        vec3 size;
        material *mat_ptr;
};

bool cube::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
    
}

#endif