#ifndef MATERIALH
#define MATERIALH

struct hit_record;

#include "ray.h"
#include "hitable.h"

vec3 random_in_unit_sphere() {
    vec3 p;
    do {
        p = 2.0 * vec3(drand48(),drand48(),drand48()) - vec3(1,1,1);
    } while (p.squared_length() >= 1.0);
    return p;
}

vec3 reflect (const vec3& v, const vec3& n) {
    return v - 2*dot(v, n)*n;
}

bool refract(const vec3& v, const vec3& n, float ni_over_nt, vec3& refracted) {
    vec3 uv = unit_vector(v);
    float dt = dot(uv, n);
    float discriminant = 1.0 - ni_over_nt*ni_over_nt*(1-dt*dt);
    if (discriminant > 0) {
        refracted = ni_over_nt * (uv - n*dt) - n*sqrt(discriminant);
        return true;
    }
    else
        return false;
}

float schlick(float cosine, float ref_idx) {
    float r0 = (1-ref_idx) / (1+ref_idx);
    r0 = r0*r0;
    return r0 + (1-r0)*pow((1-cosine),5);
}

class texture {
public:
	virtual vec3 value(float u, float v, const vec3& p) const = 0;
};

class checker_texture : public texture {
public:
	checker_texture() {}
	checker_texture(texture *t0, texture *t1) : even(t0), odd(t1) {}
	virtual vec3 value(float u, float v, const vec3& p) const {
		float sines = sin(10 * p.x())*sin(10 * p.y())*sin(10 * p.z());
		if (sines < 0)
			return odd->value(u, v, p);
		else
			return even->value(u, v, p);
	}
	texture *odd;
	texture *even;
};

class material {
public:
	virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const = 0;
};

class diffuse_light : public material {
	public:
		diffuse_light(texture *a) : emit(a) {}
		virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const { return false; }
		virtual vec3 emitted(float u, float v, const vec3& p) const { return emit->value(u, v, p); }
		texture *emit;
};

class constant_texture : public texture {
public:
	constant_texture() {}
	constant_texture(vec3 c) : color(c) {}
	virtual vec3 value(float u, float v, const vec3& p) const {
		return color;
	}
	vec3 color;
};

class lambertian : public material {
    public:
        lambertian(texture *a) : albedo(a) {}
        virtual bool scatter(const ray& r_in, const hit_record&  rec, vec3& attenuation, ray& scattered) const {
            vec3 target = rec.p + rec.normal + random_in_unit_sphere();
            scattered = ray(rec.p, target-rec.p);
            attenuation = albedo->value(0,0,rec.p);
            return true;
        }
        texture *albedo;
};

class metal : public material {
    public:
        metal(const vec3& a, float f) : albedo(a){if(f < 1) fuzz = f; else fuzz = 1;}
        virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const {
            vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
            scattered = ray(rec.p, reflected + fuzz*random_in_unit_sphere());
            attenuation = albedo;
            return (dot(scattered.direction(), rec.normal) > 0);
        }
        vec3 albedo;
        float fuzz;
};

class dielectric : public material {
    public:
        dielectric(float ri) : ref_idx(ri) {}
        virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const {
            vec3 outward_normal;
            vec3 reflected = reflect(r_in.direction(), rec.normal);
            float ni_over_nt;
            attenuation = vec3(1.0, 1.0, 1.0);
            vec3 refracted;
            float reflect_prob;
            float cosine;
            if (dot(r_in.direction(), rec.normal) > 0) {
                outward_normal = -rec.normal;
                ni_over_nt = ref_idx;
                cosine = ref_idx * dot(r_in.direction(), rec.normal) / r_in.direction().length();
            }
            else {
                outward_normal = rec.normal;
                ni_over_nt = 1.0 / ref_idx;
                cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
            }
            if (refract (r_in.direction(), outward_normal, ni_over_nt, refracted)) {
                reflect_prob = schlick(cosine, ref_idx);
            }
            else {
                scattered = ray(rec.p, reflected);
                reflect_prob = 1.0;
            }
            if (drand48() < reflect_prob) {
                scattered = ray(rec.p, reflected);
            }
            else {
                scattered = ray(rec.p, refracted);
            }
            return true;
        }
    float ref_idx;
};

/*class image_texture : public texture {
public:
	image_texture() {}
	image_texture(unsigned char *pixels, int A, int B) : data(pixels), nx(A), ny(B) {}
	virtual vec3 value(float u, float v, const vec3& p) const;
	unsigned char *data;
	int nx, ny;
};

vec3 image_texture::value(float u, float v, const vec3& p) const {
	int i = (u)*nx;
	int j = (1 - v)*ny - 0.001;
	if (i < 0) i = 0;
	if (j < 0) j = 0;
	if (i > nx - 1) i = nx - 1;
	if (j > ny - 1) j = ny - 1;
	float rx = int(data[3 * i + 3 * nx*j]) / 255.0;
	float gx = int(data[3 * i + 3 * nx*j + 1]) / 255.0;
	float bx = int(data[3 * i + 3 * nx*j + 2]) / 255.0;
	return vec3(rx, gx, bx);
}*/

#endif