#include <limits>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include "geometry.h"
using namespace std;

struct Light {
    Light(const Vec3f &p, const float &i) : position(p), intensity(i) {}
    Vec3f position;
    float intensity;
};

struct Material {
    Material(const float &r, const Vec4f &a, const Vec3f &color, const float &spec) : refractive_index(r), albedo(a), diffuse_color(color), specular_exponent(spec) {}
    Material() : refractive_index(1), albedo(1,0,0,0), diffuse_color(), specular_exponent() {}
    float refractive_index;
    Vec4f albedo;
    Vec3f diffuse_color;
    float specular_exponent;
};

struct Sphere {
    Vec3f center;
    float radius;
    Material material;

    Sphere(const Vec3f &c, const float &r, const Material &m) : center(c), radius(r), material(m) {}

    bool ray_intersect(const Vec3f &orig, const Vec3f &dir, float &t0) const {
        Vec3f L = center - orig;
        float tca = L*dir;
        float d2 = L*L - tca*tca;
        if (d2 > radius*radius) return false;
        float thc = sqrtf(radius*radius - d2);
        t0       = tca - thc;
        float t1 = tca + thc;
        if (t0 < 0) t0 = t1;
        if (t0 < 0) return false;
        return true;
    }
};

Vec3f reflect(const Vec3f &I, const Vec3f &N) {
    return I - N*2.f*(I*N);
}

Vec3f refract(const Vec3f &I, const Vec3f &N, const float &refractive_index) { // Snell's law
    float cosi = - std::max(-1.f, std::min(1.f, I*N));
    float etai = 1, etat = refractive_index;
    Vec3f n = N;
    if (cosi < 0) { // if the ray is inside the object, swap the indices and invert the normal to get the correct result
        cosi = -cosi;
        std::swap(etai, etat); n = -N;
    }
    float eta = etai / etat;
    float k = 1 - eta*eta*(1 - cosi*cosi);
    return k < 0 ? Vec3f(0,0,0) : I*eta + n*(eta * cosi - sqrtf(k));
}

bool scene_intersect(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres, Vec3f &hit, Vec3f &N, Material &material) {
    float spheres_dist = std::numeric_limits<float>::max();
    for (size_t i=0; i < spheres.size(); i++) {
        float dist_i;
        if (spheres[i].ray_intersect(orig, dir, dist_i) && dist_i < spheres_dist) {
            spheres_dist = dist_i;
            hit = orig + dir*dist_i;
            N = (hit - spheres[i].center).normalize();
            material = spheres[i].material;
        }
    }

    float checkerboard_dist = std::numeric_limits<float>::max();
    if (fabs(dir.y)>1e-3)  {
        float d = -(orig.y+4)/dir.y; // the checkerboard plane has equation y = -4
        Vec3f pt = orig + dir*d;
        if (d>0 && fabs(pt.x)<10 && pt.z<-10 && pt.z>-30 && d<spheres_dist) {
            checkerboard_dist = d;
            hit = pt;
            N = Vec3f(0,1,0);
            material.diffuse_color = (int(.5*hit.x+1000) + int(.5*hit.z)) & 1 ? Vec3f(1,1,1) : Vec3f(1, .7, .3);
            material.diffuse_color = material.diffuse_color*.3;
        }
    }
    return std::min(spheres_dist, checkerboard_dist)<1000;
}

Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres, const std::vector<Light> &lights) {
    Vec3f background = Vec3f (.2,.7,.8);

///////////////////////////////////////////// Node 0 /////////////////////////////////////////////

    Vec3f basePoint, baseN;
    Material baseMaterial;
    if (!scene_intersect(orig, dir, spheres, basePoint, baseN, baseMaterial)) {
        return background;
    }
    Vec3f reflectdir = reflect(dir, baseN).normalize();
    Vec3f reflectorig = reflectdir * baseN < 0 ? basePoint - baseN*1e-3 : basePoint + baseN*1e-3;
    Vec3f reflectColor;

///////////////////////////////////////////// Node 1 /////////////////////////////////////////////

    Vec3f reflectPoint, reflectN;
    Material reflectMaterial;
    if (!scene_intersect(reflectorig, reflectdir, spheres, reflectPoint, reflectN, reflectMaterial)) {
        reflectColor = background;
    }
    Vec3f reflectreflectdir = reflect(reflectdir, reflectN).normalize();
    Vec3f reflectreflectorig = reflectreflectdir * reflectN < 0 ? reflectPoint - reflectN*1e-3 : reflectPoint + reflectN*1e-3;
    Vec3f reflectreflectColor;

///////////////////////////////////////////// Node 3 /////////////////////////////////////////////

    Vec3f reflectreflectPoint, reflectreflectN;
    Material reflectreflectMaterial;
    if (!scene_intersect(reflectreflectorig, reflectreflectdir, spheres, reflectreflectPoint, reflectreflectN, reflectreflectMaterial)) {
        reflectreflectColor = background;
    }
    Vec3f reflectreflectreflectdir = reflect(reflectreflectdir, reflectreflectN).normalize();
    Vec3f reflectreflectreflectorig = reflectreflectreflectdir * reflectreflectN < 0 ? reflectreflectPoint - reflectreflectN*1e-3 : reflectreflectPoint + reflectreflectN*1e-3;
    Vec3f reflectreflectreflectColor;

///////////////////////////////////////////// Node 7 /////////////////////////////////////////////

    Vec3f reflectreflectreflectPoint, reflectreflectreflectN;
    Material reflectreflectreflectMaterial;
    if (!scene_intersect(reflectreflectreflectorig, reflectreflectreflectdir, spheres, reflectreflectreflectPoint, reflectreflectreflectN, reflectreflectreflectMaterial)) {
        reflectreflectreflectColor = background;
    }
    Vec3f reflectreflectreflectreflectColor;

///////////////////////////////////////////// Node 15 /////////////////////////////////////////////

    reflectreflectreflectreflectColor = background;

///////////////////////////////////////////// Node 7 /////////////////////////////////////////////

    Vec3f reflectreflectreflectrefractColor;

///////////////////////////////////////////// Node 16 /////////////////////////////////////////////

    reflectreflectreflectrefractColor = background;

///////////////////////////////////////////// Node 7 /////////////////////////////////////////////

    if(!(reflectreflectreflectColor[0] == background[0] && reflectreflectreflectColor[1] == background[1] && reflectreflectreflectColor[2] == background[2])) {
        float diffuse_light_intensity = 0, specular_light_intensity = 0;
        for (size_t i = 0; i < lights.size(); i++) {
            Vec3f light_dir = (lights[i].position - reflectreflectreflectPoint).normalize();
            float light_distance = (lights[i].position - reflectreflectreflectPoint).norm();

            Vec3f shadow_orig = light_dir * reflectreflectreflectN < 0 ? reflectreflectreflectPoint - reflectreflectreflectN * 1e-3 : reflectreflectreflectPoint + reflectreflectreflectN * 1e-3;
            Vec3f shadow_pt, shadow_N;
            Material tmpmaterial;
            if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt - shadow_orig).norm() < light_distance)
                continue;

            diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir * reflectreflectreflectN);
            specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, reflectreflectreflectN) * reflectreflectreflectdir), reflectreflectreflectMaterial.specular_exponent) * lights[i].intensity;
        }
        reflectreflectreflectColor = reflectreflectreflectMaterial.diffuse_color * diffuse_light_intensity * reflectreflectreflectMaterial.albedo[0] +
                                     Vec3f(1., 1., 1.) * specular_light_intensity * reflectreflectreflectMaterial.albedo[1] +
                                     reflectreflectreflectreflectColor * reflectreflectreflectMaterial.albedo[2] +
                                     reflectreflectreflectrefractColor * reflectreflectreflectMaterial.albedo[3];
    }

///////////////////////////////////////////// Node 3 /////////////////////////////////////////////

    Vec3f reflectreflectrefractdir = refract(reflectreflectdir, reflectreflectN, reflectreflectMaterial.refractive_index).normalize();
    Vec3f reflectreflectrefractorig = reflectreflectrefractdir * reflectreflectN < 0 ? reflectreflectPoint - reflectreflectN*1e-3 : reflectreflectPoint + reflectreflectN*1e-3;
    Vec3f reflectreflectrefractColor;

///////////////////////////////////////////// Node 8 /////////////////////////////////////////////

    Vec3f reflectreflectrefractPoint, reflectreflectrefractN;
    Material reflectreflectrefractMaterial;
    if (!scene_intersect(reflectreflectrefractorig, reflectreflectrefractdir, spheres, reflectreflectrefractPoint, reflectreflectrefractN, reflectreflectrefractMaterial)) {
        reflectreflectrefractColor = background;
    }
    Vec3f reflectreflectrefractreflectColor;

///////////////////////////////////////////// Node 17 /////////////////////////////////////////////

    reflectreflectrefractreflectColor = background;

///////////////////////////////////////////// Node 8 /////////////////////////////////////////////

    Vec3f reflectreflectrefractrefractColor;

///////////////////////////////////////////// Node 18 /////////////////////////////////////////////

    reflectreflectrefractrefractColor = background;

///////////////////////////////////////////// Node 8 /////////////////////////////////////////////

    if(!(reflectreflectrefractColor[0] == background[0] && reflectreflectrefractColor[1] == background[1] && reflectreflectrefractColor[2] == background[2])) {
        float diffuse_light_intensity = 0, specular_light_intensity = 0;
        for (size_t i = 0; i < lights.size(); i++) {
            Vec3f light_dir = (lights[i].position - reflectreflectrefractPoint).normalize();
            float light_distance = (lights[i].position - reflectreflectrefractPoint).norm();

            Vec3f shadow_orig = light_dir * reflectreflectrefractN < 0 ? reflectreflectrefractPoint - reflectreflectrefractN * 1e-3 : reflectreflectrefractPoint + reflectreflectrefractN * 1e-3;
            Vec3f shadow_pt, shadow_N;
            Material tmpmaterial;
            if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt - shadow_orig).norm() < light_distance)
                continue;

            diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir * reflectreflectrefractN);
            specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, reflectreflectrefractN) * reflectreflectrefractdir), reflectreflectrefractMaterial.specular_exponent) * lights[i].intensity;
        }
        reflectreflectrefractColor = reflectreflectrefractMaterial.diffuse_color * diffuse_light_intensity * reflectreflectrefractMaterial.albedo[0] +
                                     Vec3f(1., 1., 1.) * specular_light_intensity * reflectreflectrefractMaterial.albedo[1] +
                                     reflectreflectrefractreflectColor * reflectreflectrefractMaterial.albedo[2] +
                                     reflectreflectrefractrefractColor * reflectreflectrefractMaterial.albedo[3];
    }

///////////////////////////////////////////// Node 3 /////////////////////////////////////////////

    if(!(reflectreflectColor[0] == background[0] && reflectreflectColor[1] == background[1] && reflectreflectColor[2] == background[2])) {
        float diffuse_light_intensity = 0, specular_light_intensity = 0;
        for (size_t i = 0; i < lights.size(); i++) {
            Vec3f light_dir = (lights[i].position - reflectreflectPoint).normalize();
            float light_distance = (lights[i].position - reflectreflectPoint).norm();

            Vec3f shadow_orig = light_dir * reflectreflectN < 0 ? reflectreflectPoint - reflectreflectN * 1e-3 : reflectreflectPoint + reflectreflectN * 1e-3;
            Vec3f shadow_pt, shadow_N;
            Material tmpmaterial;
            if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt - shadow_orig).norm() < light_distance)
                continue;

            diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir * reflectreflectN);
            specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, reflectreflectN) * reflectreflectdir), reflectreflectMaterial.specular_exponent) * lights[i].intensity;
        }
        reflectreflectColor = reflectreflectMaterial.diffuse_color * diffuse_light_intensity * reflectreflectMaterial.albedo[0] +
                              Vec3f(1., 1., 1.) * specular_light_intensity * reflectreflectMaterial.albedo[1] +
                              reflectreflectreflectColor * reflectreflectMaterial.albedo[2] +
                              reflectreflectrefractColor * reflectreflectMaterial.albedo[3];
    }

///////////////////////////////////////////// Node 1 /////////////////////////////////////////////

    Vec3f reflectrefractdir = refract(reflectdir, reflectN, reflectMaterial.refractive_index).normalize();
    Vec3f reflectrefractorig = reflectrefractdir * reflectN < 0 ? reflectPoint - reflectN*1e-3 : reflectPoint + reflectN*1e-3;
    Vec3f reflectrefractColor;

///////////////////////////////////////////// Node 4 /////////////////////////////////////////////

    Vec3f reflectrefractPoint, reflectrefractN;
    Material reflectrefractMaterial;
    if (!scene_intersect(reflectrefractorig, reflectrefractdir, spheres, reflectrefractPoint, reflectrefractN, reflectrefractMaterial)) {
        reflectrefractColor = background;
    }
    Vec3f reflectrefractreflectdir = reflect(reflectrefractdir, reflectrefractN).normalize();
    Vec3f reflectrefractreflectorig = reflectrefractreflectdir * reflectrefractN < 0 ? reflectrefractPoint - reflectrefractN*1e-3 : reflectrefractPoint + reflectrefractN*1e-3;
    Vec3f reflectrefractreflectColor;

///////////////////////////////////////////// Node 9 /////////////////////////////////////////////

    Vec3f reflectrefractreflectPoint, reflectrefractreflectN;
    Material reflectrefractreflectMaterial;
    if (!scene_intersect(reflectrefractreflectorig, reflectrefractreflectdir, spheres, reflectrefractreflectPoint, reflectrefractreflectN, reflectrefractreflectMaterial)) {
        reflectrefractreflectColor = background;
    }
    Vec3f reflectrefractreflectreflectColor;

///////////////////////////////////////////// Node 19 /////////////////////////////////////////////

    reflectrefractreflectreflectColor = background;

///////////////////////////////////////////// Node 9 /////////////////////////////////////////////

    Vec3f reflectrefractreflectrefractColor;

///////////////////////////////////////////// Node 20 /////////////////////////////////////////////

    reflectrefractreflectrefractColor = background;

///////////////////////////////////////////// Node 9 /////////////////////////////////////////////

    if(!(reflectrefractreflectColor[0] == background[0] && reflectrefractreflectColor[1] == background[1] && reflectrefractreflectColor[2] == background[2])) {
        float diffuse_light_intensity = 0, specular_light_intensity = 0;
        for (size_t i = 0; i < lights.size(); i++) {
            Vec3f light_dir = (lights[i].position - reflectrefractreflectPoint).normalize();
            float light_distance = (lights[i].position - reflectrefractreflectPoint).norm();

            Vec3f shadow_orig = light_dir * reflectrefractreflectN < 0 ? reflectrefractreflectPoint - reflectrefractreflectN * 1e-3 : reflectrefractreflectPoint + reflectrefractreflectN * 1e-3;
            Vec3f shadow_pt, shadow_N;
            Material tmpmaterial;
            if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt - shadow_orig).norm() < light_distance)
                continue;

            diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir * reflectrefractreflectN);
            specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, reflectrefractreflectN) * reflectrefractreflectdir), reflectrefractreflectMaterial.specular_exponent) * lights[i].intensity;
        }
        reflectrefractreflectColor = reflectrefractreflectMaterial.diffuse_color * diffuse_light_intensity * reflectrefractreflectMaterial.albedo[0] +
                                     Vec3f(1., 1., 1.) * specular_light_intensity * reflectrefractreflectMaterial.albedo[1] +
                                     reflectrefractreflectreflectColor * reflectrefractreflectMaterial.albedo[2] +
                                     reflectrefractreflectrefractColor * reflectrefractreflectMaterial.albedo[3];
    }

///////////////////////////////////////////// Node 4 /////////////////////////////////////////////

    Vec3f reflectrefractrefractdir = refract(reflectrefractdir, reflectrefractN, reflectrefractMaterial.refractive_index).normalize();
    Vec3f reflectrefractrefractorig = reflectrefractrefractdir * reflectrefractN < 0 ? reflectrefractPoint - reflectrefractN*1e-3 : reflectrefractPoint + reflectrefractN*1e-3;
    Vec3f reflectrefractrefractColor;

///////////////////////////////////////////// Node 10 /////////////////////////////////////////////

    Vec3f reflectrefractrefractPoint, reflectrefractrefractN;
    Material reflectrefractrefractMaterial;
    if (!scene_intersect(reflectrefractrefractorig, reflectrefractrefractdir, spheres, reflectrefractrefractPoint, reflectrefractrefractN, reflectrefractrefractMaterial)) {
        reflectrefractrefractColor = background;
    }
    Vec3f reflectrefractrefractreflectColor;

///////////////////////////////////////////// Node 21 /////////////////////////////////////////////

    reflectrefractrefractreflectColor = background;

///////////////////////////////////////////// Node 10 /////////////////////////////////////////////

    Vec3f reflectrefractrefractrefractColor;

///////////////////////////////////////////// Node 22 /////////////////////////////////////////////

    reflectrefractrefractrefractColor = background;

///////////////////////////////////////////// Node 10 /////////////////////////////////////////////

    if(!(reflectrefractrefractColor[0] == background[0] && reflectrefractrefractColor[1] == background[1] && reflectrefractrefractColor[2] == background[2])) {
        float diffuse_light_intensity = 0, specular_light_intensity = 0;
        for (size_t i = 0; i < lights.size(); i++) {
            Vec3f light_dir = (lights[i].position - reflectrefractrefractPoint).normalize();
            float light_distance = (lights[i].position - reflectrefractrefractPoint).norm();

            Vec3f shadow_orig = light_dir * reflectrefractrefractN < 0 ? reflectrefractrefractPoint - reflectrefractrefractN * 1e-3 : reflectrefractrefractPoint + reflectrefractrefractN * 1e-3;
            Vec3f shadow_pt, shadow_N;
            Material tmpmaterial;
            if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt - shadow_orig).norm() < light_distance)
                continue;

            diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir * reflectrefractrefractN);
            specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, reflectrefractrefractN) * reflectrefractrefractdir), reflectrefractrefractMaterial.specular_exponent) * lights[i].intensity;
        }
        reflectrefractrefractColor = reflectrefractrefractMaterial.diffuse_color * diffuse_light_intensity * reflectrefractrefractMaterial.albedo[0] +
                                     Vec3f(1., 1., 1.) * specular_light_intensity * reflectrefractrefractMaterial.albedo[1] +
                                     reflectrefractrefractreflectColor * reflectrefractrefractMaterial.albedo[2] +
                                     reflectrefractrefractrefractColor * reflectrefractrefractMaterial.albedo[3];
    }

///////////////////////////////////////////// Node 4 /////////////////////////////////////////////

    if(!(reflectrefractColor[0] == background[0] && reflectrefractColor[1] == background[1] && reflectrefractColor[2] == background[2])) {
        float diffuse_light_intensity = 0, specular_light_intensity = 0;
        for (size_t i = 0; i < lights.size(); i++) {
            Vec3f light_dir = (lights[i].position - reflectrefractPoint).normalize();
            float light_distance = (lights[i].position - reflectrefractPoint).norm();

            Vec3f shadow_orig = light_dir * reflectrefractN < 0 ? reflectrefractPoint - reflectrefractN * 1e-3 : reflectrefractPoint + reflectrefractN * 1e-3;
            Vec3f shadow_pt, shadow_N;
            Material tmpmaterial;
            if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt - shadow_orig).norm() < light_distance)
                continue;

            diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir * reflectrefractN);
            specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, reflectrefractN) * reflectrefractdir), reflectrefractMaterial.specular_exponent) * lights[i].intensity;
        }
        reflectrefractColor = reflectrefractMaterial.diffuse_color * diffuse_light_intensity * reflectrefractMaterial.albedo[0] +
                              Vec3f(1., 1., 1.) * specular_light_intensity * reflectrefractMaterial.albedo[1] +
                              reflectrefractreflectColor * reflectrefractMaterial.albedo[2] +
                              reflectrefractrefractColor * reflectrefractMaterial.albedo[3];
    }

///////////////////////////////////////////// Node 1 /////////////////////////////////////////////

    if(!(reflectColor[0] == background[0] && reflectColor[1] == background[1] && reflectColor[2] == background[2])) {
        float diffuse_light_intensity = 0, specular_light_intensity = 0;
        for (size_t i = 0; i < lights.size(); i++) {
            Vec3f light_dir = (lights[i].position - reflectPoint).normalize();
            float light_distance = (lights[i].position - reflectPoint).norm();

            Vec3f shadow_orig = light_dir * reflectN < 0 ? reflectPoint - reflectN * 1e-3 : reflectPoint + reflectN * 1e-3;
            Vec3f shadow_pt, shadow_N;
            Material tmpmaterial;
            if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt - shadow_orig).norm() < light_distance)
                continue;

            diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir * reflectN);
            specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, reflectN) * reflectdir), reflectMaterial.specular_exponent) * lights[i].intensity;
        }
        reflectColor = reflectMaterial.diffuse_color * diffuse_light_intensity * reflectMaterial.albedo[0] +
                       Vec3f(1., 1., 1.) * specular_light_intensity * reflectMaterial.albedo[1] +
                       reflectreflectColor * reflectMaterial.albedo[2] +
                       reflectrefractColor * reflectMaterial.albedo[3];
    }

///////////////////////////////////////////// Node 0 /////////////////////////////////////////////

    Vec3f refractdir = refract(dir, baseN, baseMaterial.refractive_index).normalize();
    Vec3f refractorig = refractdir * baseN < 0 ? basePoint - baseN*1e-3 : basePoint + baseN*1e-3;
    Vec3f refractColor;

///////////////////////////////////////////// Node 2 /////////////////////////////////////////////

    Vec3f refractPoint, refractN;
    Material refractMaterial;
    if (!scene_intersect(refractorig, refractdir, spheres, refractPoint, refractN, refractMaterial)) {
        refractColor = background;
    }
    Vec3f refractreflectdir = reflect(refractdir, refractN).normalize();
    Vec3f refractreflectorig = refractreflectdir * refractN < 0 ? refractPoint - refractN*1e-3 : refractPoint + refractN*1e-3;
    Vec3f refractreflectColor;

///////////////////////////////////////////// Node 5 /////////////////////////////////////////////

    Vec3f refractreflectPoint, refractreflectN;
    Material refractreflectMaterial;
    if (!scene_intersect(refractreflectorig, refractreflectdir, spheres, refractreflectPoint, refractreflectN, refractreflectMaterial)) {
        refractreflectColor = background;
    }
    Vec3f refractreflectreflectdir = reflect(refractreflectdir, refractreflectN).normalize();
    Vec3f refractreflectreflectorig = refractreflectreflectdir * refractreflectN < 0 ? refractreflectPoint - refractreflectN*1e-3 : refractreflectPoint + refractreflectN*1e-3;
    Vec3f refractreflectreflectColor;

///////////////////////////////////////////// Node 11 /////////////////////////////////////////////

    Vec3f refractreflectreflectPoint, refractreflectreflectN;
    Material refractreflectreflectMaterial;
    if (!scene_intersect(refractreflectreflectorig, refractreflectreflectdir, spheres, refractreflectreflectPoint, refractreflectreflectN, refractreflectreflectMaterial)) {
        refractreflectreflectColor = background;
    }
    Vec3f refractreflectreflectreflectColor;

///////////////////////////////////////////// Node 23 /////////////////////////////////////////////

    refractreflectreflectreflectColor = background;

///////////////////////////////////////////// Node 11 /////////////////////////////////////////////

    Vec3f refractreflectreflectrefractColor;

///////////////////////////////////////////// Node 24 /////////////////////////////////////////////

    refractreflectreflectrefractColor = background;

///////////////////////////////////////////// Node 11 /////////////////////////////////////////////

    if(!(refractreflectreflectColor[0] == background[0] && refractreflectreflectColor[1] == background[1] && refractreflectreflectColor[2] == background[2])) {
        float diffuse_light_intensity = 0, specular_light_intensity = 0;
        for (size_t i = 0; i < lights.size(); i++) {
            Vec3f light_dir = (lights[i].position - refractreflectreflectPoint).normalize();
            float light_distance = (lights[i].position - refractreflectreflectPoint).norm();

            Vec3f shadow_orig = light_dir * refractreflectreflectN < 0 ? refractreflectreflectPoint - refractreflectreflectN * 1e-3 : refractreflectreflectPoint + refractreflectreflectN * 1e-3;
            Vec3f shadow_pt, shadow_N;
            Material tmpmaterial;
            if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt - shadow_orig).norm() < light_distance)
                continue;

            diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir * refractreflectreflectN);
            specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, refractreflectreflectN) * refractreflectreflectdir), refractreflectreflectMaterial.specular_exponent) * lights[i].intensity;
        }
        refractreflectreflectColor = refractreflectreflectMaterial.diffuse_color * diffuse_light_intensity * refractreflectreflectMaterial.albedo[0] +
                                     Vec3f(1., 1., 1.) * specular_light_intensity * refractreflectreflectMaterial.albedo[1] +
                                     refractreflectreflectreflectColor * refractreflectreflectMaterial.albedo[2] +
                                     refractreflectreflectrefractColor * refractreflectreflectMaterial.albedo[3];
    }

///////////////////////////////////////////// Node 5 /////////////////////////////////////////////

    Vec3f refractreflectrefractdir = refract(refractreflectdir, refractreflectN, refractreflectMaterial.refractive_index).normalize();
    Vec3f refractreflectrefractorig = refractreflectrefractdir * refractreflectN < 0 ? refractreflectPoint - refractreflectN*1e-3 : refractreflectPoint + refractreflectN*1e-3;
    Vec3f refractreflectrefractColor;

///////////////////////////////////////////// Node 12 /////////////////////////////////////////////

    Vec3f refractreflectrefractPoint, refractreflectrefractN;
    Material refractreflectrefractMaterial;
    if (!scene_intersect(refractreflectrefractorig, refractreflectrefractdir, spheres, refractreflectrefractPoint, refractreflectrefractN, refractreflectrefractMaterial)) {
        refractreflectrefractColor = background;
    }
    Vec3f refractreflectrefractreflectColor;

///////////////////////////////////////////// Node 25 /////////////////////////////////////////////

    refractreflectrefractreflectColor = background;

///////////////////////////////////////////// Node 12 /////////////////////////////////////////////

    Vec3f refractreflectrefractrefractColor;

///////////////////////////////////////////// Node 26 /////////////////////////////////////////////

    refractreflectrefractrefractColor = background;

///////////////////////////////////////////// Node 12 /////////////////////////////////////////////

    if(!(refractreflectrefractColor[0] == background[0] && refractreflectrefractColor[1] == background[1] && refractreflectrefractColor[2] == background[2])) {
        float diffuse_light_intensity = 0, specular_light_intensity = 0;
        for (size_t i = 0; i < lights.size(); i++) {
            Vec3f light_dir = (lights[i].position - refractreflectrefractPoint).normalize();
            float light_distance = (lights[i].position - refractreflectrefractPoint).norm();

            Vec3f shadow_orig = light_dir * refractreflectrefractN < 0 ? refractreflectrefractPoint - refractreflectrefractN * 1e-3 : refractreflectrefractPoint + refractreflectrefractN * 1e-3;
            Vec3f shadow_pt, shadow_N;
            Material tmpmaterial;
            if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt - shadow_orig).norm() < light_distance)
                continue;

            diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir * refractreflectrefractN);
            specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, refractreflectrefractN) * refractreflectrefractdir), refractreflectrefractMaterial.specular_exponent) * lights[i].intensity;
        }
        refractreflectrefractColor = refractreflectrefractMaterial.diffuse_color * diffuse_light_intensity * refractreflectrefractMaterial.albedo[0] +
                                     Vec3f(1., 1., 1.) * specular_light_intensity * refractreflectrefractMaterial.albedo[1] +
                                     refractreflectrefractreflectColor * refractreflectrefractMaterial.albedo[2] +
                                     refractreflectrefractrefractColor * refractreflectrefractMaterial.albedo[3];
    }

///////////////////////////////////////////// Node 5 /////////////////////////////////////////////

    if(!(refractreflectColor[0] == background[0] && refractreflectColor[1] == background[1] && refractreflectColor[2] == background[2])) {
        float diffuse_light_intensity = 0, specular_light_intensity = 0;
        for (size_t i = 0; i < lights.size(); i++) {
            Vec3f light_dir = (lights[i].position - refractreflectPoint).normalize();
            float light_distance = (lights[i].position - refractreflectPoint).norm();

            Vec3f shadow_orig = light_dir * refractreflectN < 0 ? refractreflectPoint - refractreflectN * 1e-3 : refractreflectPoint + refractreflectN * 1e-3;
            Vec3f shadow_pt, shadow_N;
            Material tmpmaterial;
            if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt - shadow_orig).norm() < light_distance)
                continue;

            diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir * refractreflectN);
            specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, refractreflectN) * refractreflectdir), refractreflectMaterial.specular_exponent) * lights[i].intensity;
        }
        refractreflectColor = refractreflectMaterial.diffuse_color * diffuse_light_intensity * refractreflectMaterial.albedo[0] +
                              Vec3f(1., 1., 1.) * specular_light_intensity * refractreflectMaterial.albedo[1] +
                              refractreflectreflectColor * refractreflectMaterial.albedo[2] +
                              refractreflectrefractColor * refractreflectMaterial.albedo[3];
    }

///////////////////////////////////////////// Node 2 /////////////////////////////////////////////

    Vec3f refractrefractdir = refract(refractdir, refractN, refractMaterial.refractive_index).normalize();
    Vec3f refractrefractorig = refractrefractdir * refractN < 0 ? refractPoint - refractN*1e-3 : refractPoint + refractN*1e-3;
    Vec3f refractrefractColor;

///////////////////////////////////////////// Node 6 /////////////////////////////////////////////

    Vec3f refractrefractPoint, refractrefractN;
    Material refractrefractMaterial;
    if (!scene_intersect(refractrefractorig, refractrefractdir, spheres, refractrefractPoint, refractrefractN, refractrefractMaterial)) {
        refractrefractColor = background;
    }
    Vec3f refractrefractreflectdir = reflect(refractrefractdir, refractrefractN).normalize();
    Vec3f refractrefractreflectorig = refractrefractreflectdir * refractrefractN < 0 ? refractrefractPoint - refractrefractN*1e-3 : refractrefractPoint + refractrefractN*1e-3;
    Vec3f refractrefractreflectColor;

///////////////////////////////////////////// Node 13 /////////////////////////////////////////////

    Vec3f refractrefractreflectPoint, refractrefractreflectN;
    Material refractrefractreflectMaterial;
    if (!scene_intersect(refractrefractreflectorig, refractrefractreflectdir, spheres, refractrefractreflectPoint, refractrefractreflectN, refractrefractreflectMaterial)) {
        refractrefractreflectColor = background;
    }
    Vec3f refractrefractreflectreflectColor;

///////////////////////////////////////////// Node 27 /////////////////////////////////////////////

    refractrefractreflectreflectColor = background;

///////////////////////////////////////////// Node 13 /////////////////////////////////////////////

    Vec3f refractrefractreflectrefractColor;

///////////////////////////////////////////// Node 28 /////////////////////////////////////////////

    refractrefractreflectrefractColor = background;

///////////////////////////////////////////// Node 13 /////////////////////////////////////////////

    if(!(refractrefractreflectColor[0] == background[0] && refractrefractreflectColor[1] == background[1] && refractrefractreflectColor[2] == background[2])) {
        float diffuse_light_intensity = 0, specular_light_intensity = 0;
        for (size_t i = 0; i < lights.size(); i++) {
            Vec3f light_dir = (lights[i].position - refractrefractreflectPoint).normalize();
            float light_distance = (lights[i].position - refractrefractreflectPoint).norm();

            Vec3f shadow_orig = light_dir * refractrefractreflectN < 0 ? refractrefractreflectPoint - refractrefractreflectN * 1e-3 : refractrefractreflectPoint + refractrefractreflectN * 1e-3;
            Vec3f shadow_pt, shadow_N;
            Material tmpmaterial;
            if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt - shadow_orig).norm() < light_distance)
                continue;

            diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir * refractrefractreflectN);
            specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, refractrefractreflectN) * refractrefractreflectdir), refractrefractreflectMaterial.specular_exponent) * lights[i].intensity;
        }
        refractrefractreflectColor = refractrefractreflectMaterial.diffuse_color * diffuse_light_intensity * refractrefractreflectMaterial.albedo[0] +
                                     Vec3f(1., 1., 1.) * specular_light_intensity * refractrefractreflectMaterial.albedo[1] +
                                     refractrefractreflectreflectColor * refractrefractreflectMaterial.albedo[2] +
                                     refractrefractreflectrefractColor * refractrefractreflectMaterial.albedo[3];
    }

///////////////////////////////////////////// Node 6 /////////////////////////////////////////////

    Vec3f refractrefractrefractdir = refract(refractrefractdir, refractrefractN, refractrefractMaterial.refractive_index).normalize();
    Vec3f refractrefractrefractorig = refractrefractrefractdir * refractrefractN < 0 ? refractrefractPoint - refractrefractN*1e-3 : refractrefractPoint + refractrefractN*1e-3;
    Vec3f refractrefractrefractColor;

///////////////////////////////////////////// Node 14 /////////////////////////////////////////////

    Vec3f refractrefractrefractPoint, refractrefractrefractN;
    Material refractrefractrefractMaterial;
    if (!scene_intersect(refractrefractrefractorig, refractrefractrefractdir, spheres, refractrefractrefractPoint, refractrefractrefractN, refractrefractrefractMaterial)) {
        refractrefractrefractColor = background;
    }
    Vec3f refractrefractrefractreflectColor;

///////////////////////////////////////////// Node 29 /////////////////////////////////////////////

    refractrefractrefractreflectColor = background;

///////////////////////////////////////////// Node 14 /////////////////////////////////////////////

    Vec3f refractrefractrefractrefractColor;

///////////////////////////////////////////// Node 30 /////////////////////////////////////////////

    refractrefractrefractrefractColor = background;

///////////////////////////////////////////// Node 14 /////////////////////////////////////////////

    if(!(refractrefractrefractColor[0] == background[0] && refractrefractrefractColor[1] == background[1] && refractrefractrefractColor[2] == background[2])) {
        float diffuse_light_intensity = 0, specular_light_intensity = 0;
        for (size_t i = 0; i < lights.size(); i++) {
            Vec3f light_dir = (lights[i].position - refractrefractrefractPoint).normalize();
            float light_distance = (lights[i].position - refractrefractrefractPoint).norm();

            Vec3f shadow_orig = light_dir * refractrefractrefractN < 0 ? refractrefractrefractPoint - refractrefractrefractN * 1e-3 : refractrefractrefractPoint + refractrefractrefractN * 1e-3;
            Vec3f shadow_pt, shadow_N;
            Material tmpmaterial;
            if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt - shadow_orig).norm() < light_distance)
                continue;

            diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir * refractrefractrefractN);
            specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, refractrefractrefractN) * refractrefractrefractdir), refractrefractrefractMaterial.specular_exponent) * lights[i].intensity;
        }
        refractrefractrefractColor = refractrefractrefractMaterial.diffuse_color * diffuse_light_intensity * refractrefractrefractMaterial.albedo[0] +
                                     Vec3f(1., 1., 1.) * specular_light_intensity * refractrefractrefractMaterial.albedo[1] +
                                     refractrefractrefractreflectColor * refractrefractrefractMaterial.albedo[2] +
                                     refractrefractrefractrefractColor * refractrefractrefractMaterial.albedo[3];
    }

///////////////////////////////////////////// Node 6 /////////////////////////////////////////////

    if(!(refractrefractColor[0] == background[0] && refractrefractColor[1] == background[1] && refractrefractColor[2] == background[2])) {
        float diffuse_light_intensity = 0, specular_light_intensity = 0;
        for (size_t i = 0; i < lights.size(); i++) {
            Vec3f light_dir = (lights[i].position - refractrefractPoint).normalize();
            float light_distance = (lights[i].position - refractrefractPoint).norm();

            Vec3f shadow_orig = light_dir * refractrefractN < 0 ? refractrefractPoint - refractrefractN * 1e-3 : refractrefractPoint + refractrefractN * 1e-3;
            Vec3f shadow_pt, shadow_N;
            Material tmpmaterial;
            if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt - shadow_orig).norm() < light_distance)
                continue;

            diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir * refractrefractN);
            specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, refractrefractN) * refractrefractdir), refractrefractMaterial.specular_exponent) * lights[i].intensity;
        }
        refractrefractColor = refractrefractMaterial.diffuse_color * diffuse_light_intensity * refractrefractMaterial.albedo[0] +
                              Vec3f(1., 1., 1.) * specular_light_intensity * refractrefractMaterial.albedo[1] +
                              refractrefractreflectColor * refractrefractMaterial.albedo[2] +
                              refractrefractrefractColor * refractrefractMaterial.albedo[3];
    }

///////////////////////////////////////////// Node 2 /////////////////////////////////////////////

    if(!(refractColor[0] == background[0] && refractColor[1] == background[1] && refractColor[2] == background[2])) {
        float diffuse_light_intensity = 0, specular_light_intensity = 0;
        for (size_t i = 0; i < lights.size(); i++) {
            Vec3f light_dir = (lights[i].position - refractPoint).normalize();
            float light_distance = (lights[i].position - refractPoint).norm();

            Vec3f shadow_orig = light_dir * refractN < 0 ? refractPoint - refractN * 1e-3 : refractPoint + refractN * 1e-3;
            Vec3f shadow_pt, shadow_N;
            Material tmpmaterial;
            if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt - shadow_orig).norm() < light_distance)
                continue;

            diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir * refractN);
            specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, refractN) * refractdir), refractMaterial.specular_exponent) * lights[i].intensity;
        }
        refractColor = refractMaterial.diffuse_color * diffuse_light_intensity * refractMaterial.albedo[0] +
                       Vec3f(1., 1., 1.) * specular_light_intensity * refractMaterial.albedo[1] +
                       refractreflectColor * refractMaterial.albedo[2] +
                       refractrefractColor * refractMaterial.albedo[3];
    }

///////////////////////////////////////////// Node 0 /////////////////////////////////////////////

    float diffuse_light_intensity = 0, specular_light_intensity = 0;
    for (size_t i = 0; i < lights.size(); i++) {
        Vec3f light_dir = (lights[i].position - basePoint).normalize();
        float light_distance = (lights[i].position - basePoint).norm();

        Vec3f shadow_orig = light_dir * baseN < 0 ? basePoint - baseN * 1e-3 : basePoint + baseN * 1e-3;
        Vec3f shadow_pt, shadow_N;
        Material tmpmaterial;
        if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt - shadow_orig).norm() < light_distance)
            continue;

        diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir * baseN);
        specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, baseN) * dir), baseMaterial.specular_exponent) * lights[i].intensity;
    }
    return baseMaterial.diffuse_color * diffuse_light_intensity * baseMaterial.albedo[0] +
           Vec3f(1., 1., 1.) * specular_light_intensity * baseMaterial.albedo[1] +
           reflectColor * baseMaterial.albedo[2] +
           refractColor * baseMaterial.albedo[3];

}



void render(const std::vector<Sphere> &spheres, const std::vector<Light> &lights) {
    const int width = 1024;
    const int height = 768;
    const int fov = 1;
    std::vector<Vec3f> framebuffer(width * height);

//    #pragma omp parallel for
    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
            float x = (2 * (i + 0.5) / (float) width - 1) * tan(fov / 2.) * width / (float) height;
            float y = -(2 * (j + 0.5) / (float) height - 1) * tan(fov / 2.);
            Vec3f dir = Vec3f(x, y, -1).normalize();
            framebuffer[i + j * width] = cast_ray(Vec3f(0, 0, 0), dir, spheres, lights);
        }
    }
    //myfile.close();

    std::ofstream ofs;
    ofs.open("./attemptDepth3.ppm", std::ofstream::out | std::ofstream::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";

    for (size_t i = 0; i < height * width; ++i) {
        Vec3f &c = framebuffer[i];
        float max = std::max(c[0], std::max(c[1], c[2]));
        if (max > 1) c = c * (1. / max);
        for (size_t j = 0; j < 3; j++) {
            ofs << (char) (255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
        }
    }
    ofs.close();
}

    int main() {
        Material      ivory(1.0, Vec4f(0.6,  0.3, 0.1, 0.0), Vec3f(0.4, 0.4, 0.3),   50.);
        Material      glass(1.5, Vec4f(0.0,  0.5, 0.1, 0.8), Vec3f(0.6, 0.7, 0.8),  125.);
        Material red_rubber(1.0, Vec4f(0.9,  0.1, 0.0, 0.0), Vec3f(0.3, 0.1, 0.1),   10.);
        Material     mirror(1.0, Vec4f(0.0, 10.0, 0.8, 0.0), Vec3f(1.0, 1.0, 1.0), 1425.);

        std::vector<Sphere> spheres;
        spheres.push_back(Sphere(Vec3f(-3,    0,   -16), 2,      ivory));
        spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2,      glass));
        spheres.push_back(Sphere(Vec3f( 1.5, -0.5, -18), 3, red_rubber));
        spheres.push_back(Sphere(Vec3f( 7,    5,   -18), 4,     mirror));

        std::vector<Light>  lights;
        lights.push_back(Light(Vec3f(-20, 20,  20), 1.5));
        lights.push_back(Light(Vec3f( 30, 50, -25), 1.8));
        lights.push_back(Light(Vec3f( 30, 20,  30), 1.7));

        render(spheres, lights);

        return 0;
    }
    