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

Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres, const std::vector<Light> &lights, size_t depth=0) {
////////////////////////////////ORIGINAL/////////////////////////////////////////////////
/**
    Vec3f point, N;
    Material material;

    if (depth > 1 || !scene_intersect(orig, dir, spheres, point, N, material)) {
        return Vec3f(0.2, 0.7, 0.8); // background color
    }

    Vec3f reflect_dir = reflect(dir, N).normalize();
    Vec3f refract_dir = refract(dir, N, material.refractive_index).normalize();
    Vec3f reflect_orig = reflect_dir * N < 0 ? point - N * 1e-3 : point + N * 1e-3; // offset the original point to avoid occlusion by the object itself
    Vec3f refract_orig = refract_dir * N < 0 ? point - N * 1e-3 : point + N * 1e-3;
    Vec3f reflect_color = cast_ray(reflect_orig, reflect_dir, spheres, lights, depth + 1);
    Vec3f refract_color = cast_ray(refract_orig, refract_dir, spheres, lights, depth + 1);

    float diffuse_light_intensity = 0, specular_light_intensity = 0;
    for (size_t i = 0; i < lights.size(); i++) {
        Vec3f light_dir = (lights[i].position - point).normalize();
        float light_distance = (lights[i].position - point).norm();

        Vec3f shadow_orig = light_dir * N < 0 ? point - N * 1e-3 : point + N * 1e-3; // checking if the point lies in the shadow of the lights[i]
        Vec3f shadow_pt, shadow_N;
        Material tmpmaterial;
        if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) &&
            (shadow_pt - shadow_orig).norm() < light_distance)
            continue;

        diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir * N);
        specular_light_intensity +=
                powf(std::max(0.f, -reflect(-light_dir, N) * dir), material.specular_exponent) * lights[i].intensity;
    }
    return material.diffuse_color * diffuse_light_intensity * material.albedo[0] +
           Vec3f(1., 1., 1.) * specular_light_intensity * material.albedo[1] + reflect_color * material.albedo[2] +
           refract_color * material.albedo[3];
**/
////////////////////////////////ORIGINAL/////////////////////////////////////////////////
////////////////////////////////DEPTH 0//////////////////////////////////////////////////

/**
    if (!scene_intersect(orig, dir, spheres, point, N, material)) {
        return Vec3f(0.2, 0.7, 0.8); // background color
    }

    Vec3f background = Vec3f(0.2, 0.7, 0.8);
    Vec3f reflectColor = background;
    Vec3f refractColor = background;
    float diffuse_light_intensity = 0, specular_light_intensity = 0;
    for (size_t i=0; i<lights.size(); i++) {
        Vec3f light_dir      = (lights[i].position - point).normalize();
        float light_distance = (lights[i].position - point).norm();

        Vec3f shadow_orig = light_dir*N < 0 ? point - N*1e-3 : point + N*1e-3; // checking if the point lies in the shadow of the lights[i]
        Vec3f shadow_pt, shadow_N;
        Material tmpmaterial;
        if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt-shadow_orig).norm() < light_distance)
            continue;

        diffuse_light_intensity  += lights[i].intensity * std::max(0.f, light_dir*N);
        specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, N)*dir), material.specular_exponent)*lights[i].intensity;
    }
    return material.diffuse_color * diffuse_light_intensity * material.albedo[0] + Vec3f(1., 1., 1.)*specular_light_intensity * material.albedo[1] + reflectColor*material.albedo[2] + refractColor*material.albedo[3];
**/

////////////////////////////////DEPTH 0//////////////////////////////////////////////////
////////////////////////////////DEPTH 1//////////////////////////////////////////////////

//Black
    Vec3f baseLayerPoint, baseLayerN;
    Material baseLayerMaterial;
    Vec3f background = Vec3f (.2,.7,.8);
    if (!scene_intersect(orig, dir, spheres, baseLayerPoint, baseLayerN, baseLayerMaterial)) {
        return background;
    }
    Vec3f depthZeroReflectDir = reflect(dir, baseLayerN).normalize();
    Vec3f depthZeroReflectOrig = depthZeroReflectDir*baseLayerN < 0 ? baseLayerPoint - baseLayerN*1e-3 : baseLayerPoint + baseLayerN*1e-3;
    Vec3f depthZeroReflectColor;
//Blue
    Vec3f reflectLayerOnePoint, reflectLayerOneN;
    Material reflectLayerOneMaterial;
//    cout << "Reflect before albedo: " << reflectLayerOneMaterial.albedo << "\n";
//    cout << "Reflect point before: " << reflectLayerOnePoint << " Reflect N before: " << reflectLayerOneN << "\n";

    if (!scene_intersect(depthZeroReflectOrig, depthZeroReflectDir, spheres, reflectLayerOnePoint, reflectLayerOneN, reflectLayerOneMaterial)) {
        depthZeroReflectColor = background;
    }
//    cout << "Reflect after albedo: " << reflectLayerOneMaterial.albedo << "\n";
//    cout << "Reflect point after: " << reflectLayerOnePoint << " Reflect N after: " << reflectLayerOneN << "\n";


    Vec3f reflectLeftLayerOneDir = reflect(depthZeroReflectDir, reflectLayerOneN).normalize();
    Vec3f reflectLeftLayerOneOrig = reflectLeftLayerOneDir*reflectLayerOneN < 0 ? reflectLayerOnePoint - reflectLayerOneN*1e-3 : reflectLayerOnePoint + reflectLayerOneN*1e-3;
    Vec3f reflectLeftLayerOneColor;
//Dark Green
    //Syntax is reflect from parent then Reflect from grandparent
    Vec3f reflectReflectLayerTwoPoint, reflectReflectLayerTwoN;
    Material reflectReflectLayerTwoMaterial;
    if (!scene_intersect(reflectLeftLayerOneOrig, reflectLeftLayerOneDir, spheres, reflectReflectLayerTwoPoint, reflectReflectLayerTwoN, reflectReflectLayerTwoMaterial)) {
        reflectLeftLayerOneColor = background;
    }
    reflectLeftLayerOneColor = background;
//Blue
    Vec3f refractLeftLayerOneDir = refract(depthZeroReflectDir, reflectLayerOneN, reflectLayerOneMaterial.refractive_index).normalize();
    Vec3f refractLeftLayerOneOrig = refractLeftLayerOneDir*reflectLayerOneN < 0 ? reflectLayerOnePoint - reflectLayerOneN*1e-3 : reflectLayerOnePoint + reflectLayerOneN*1e-3;
    Vec3f refractLeftLayerOneColor;
//Orange
    //Syntax is refract from parent then Reflect from grandparent
    Vec3f refractReflectLayerTwoPoint, refractReflectLayerTwoN;
    Material refractReflectLayerTwoMaterial;
    if (!scene_intersect(refractLeftLayerOneOrig, refractLeftLayerOneDir, spheres, refractReflectLayerTwoPoint, refractReflectLayerTwoN, refractReflectLayerTwoMaterial)) {
        refractLeftLayerOneColor = background;
    }
    refractLeftLayerOneColor = background;
//Blue
    float diffuse_light_intensity = 0, specular_light_intensity = 0;
    if (!(depthZeroReflectColor[0] == .2 && depthZeroReflectColor[1] == .7 && depthZeroReflectColor[2] == .8)) {
        for (size_t i = 0; i < lights.size(); i++) {
            Vec3f light_dir = (lights[i].position - reflectLayerOnePoint).normalize();
            float light_distance = (lights[i].position - reflectLayerOnePoint).norm();

            Vec3f shadow_orig = light_dir * reflectLayerOneN < 0 ? reflectLayerOnePoint - reflectLayerOneN * 1e-3 : reflectLayerOnePoint + reflectLayerOneN * 1e-3; // checking if the point lies in the shadow of the lights[i]
            Vec3f shadow_pt, shadow_N;
            Material tmpmaterial;
            if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) &&
                (shadow_pt - shadow_orig).norm() < light_distance)
                continue;

            diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir * reflectLayerOneN);
            specular_light_intensity +=
                    powf(std::max(0.f, -reflect(-light_dir, reflectLayerOneN) * depthZeroReflectDir),
                         reflectLayerOneMaterial.specular_exponent) * lights[i].intensity;
        }
        depthZeroReflectColor =
                reflectLayerOneMaterial.diffuse_color * diffuse_light_intensity * reflectLayerOneMaterial.albedo[0] +
                Vec3f(1., 1., 1.) * specular_light_intensity * reflectLayerOneMaterial.albedo[1] +
                reflectLeftLayerOneColor * reflectLayerOneMaterial.albedo[2] +
                refractLeftLayerOneColor * reflectLayerOneMaterial.albedo[3];
//        if (depthZeroReflectColor[0] == 0 && depthZeroReflectColor[1] == 0 && depthZeroReflectColor[2] == 0) {
//        cout << "Reflect color: " << depthZeroReflectColor << " albedo: " << reflectLayerOneMaterial.albedo << "\n";
//        cout << "Reflect reflect color: " << reflectLeftLayerOneColor << "Refract reflect color: " << refractLeftLayerOneColor << "\n";
//        cout << "Diffuse: " << reflectLayerOneMaterial.diffuse_color << " diffuse intensity: " << diffuse_light_intensity << " albedo[0]: " << reflectLayerOneMaterial.albedo[0] << "\n";
//        cout << reflectLayerOneMaterial.diffuse_color << " * " << diffuse_light_intensity << " * " << reflectLayerOneMaterial.albedo[0] << "\n";
//            cout << "Point: " << reflectLayerOnePoint << " N: " << reflectLayerOneN << "\n";
//            depthZeroReflectColor = background;
//        }
    }
    //Black
    Vec3f depthZeroRefractDir = refract(dir, baseLayerN, baseLayerMaterial.refractive_index).normalize();
    Vec3f depthZeroRefractOrig = depthZeroRefractDir*baseLayerN < 0 ? baseLayerPoint - baseLayerN*1e-3 : baseLayerPoint + baseLayerN*1e-3;
    Vec3f depthZeroRefractColor;
//Purple
    Vec3f refractLayerOnePoint, refractLayerOneN;
    Material refractLayerOneMaterial;
//    cout << "Refract before albedo: " << refractLayerOneMaterial.albedo << "\n";
//    cout << "Refract point before: " << refractLayerOnePoint << " Refract N before: " << refractLayerOneN << "\n";

    if (!scene_intersect(depthZeroRefractOrig, depthZeroRefractDir, spheres, refractLayerOnePoint, refractLayerOneN, refractLayerOneMaterial)) {
        depthZeroRefractColor = background;
    }

//    cout << "Refract after albedo: " << refractLayerOneMaterial.albedo << "\n";
//    cout << "Refract point after: " << refractLayerOnePoint << " Refract N after: " << refractLayerOneN << "\n";


    Vec3f reflectRightLayerOneDir = reflect(depthZeroRefractDir, refractLayerOneN).normalize();
    Vec3f reflectRightLayerOneOrig = reflectRightLayerOneDir*refractLayerOneN < 0 ? refractLayerOnePoint - refractLayerOneN*1e-3 : refractLayerOnePoint + refractLayerOneN*1e-3;
    Vec3f reflectRightLayerOneColor;
//Brown
    //Syntax is reflect from parent then Refract from grandparent
    Vec3f reflectRefractLayerTwoPoint, reflectRefractLayerTwoN;
    Material reflectRefractLayerTwoMaterial;
    if (!scene_intersect(reflectRightLayerOneOrig, reflectRightLayerOneDir, spheres, reflectRefractLayerTwoPoint, reflectRefractLayerTwoN, reflectRefractLayerTwoMaterial)) {
        reflectRightLayerOneColor = background;
    }
    reflectRightLayerOneColor = background;
//Purple
    Vec3f refractRightLayerOneDir = refract(depthZeroRefractDir, refractLayerOneN, refractLayerOneMaterial.refractive_index).normalize();
    Vec3f refractRightLayerOneOrig = refractRightLayerOneDir*refractLayerOneN < 0 ? refractLayerOnePoint - refractLayerOneN*1e-3 : refractLayerOnePoint + refractLayerOneN*1e-3;
    Vec3f refractRightLayerOneColor;
//Light Green
    //Syntax is refract from parent then Refract from grandparent
    Vec3f refractRefractLayerTwoPoint, refractRefractLayerTwoN;
    Material refractRefractLayerTwoMaterial;
    if (!scene_intersect(refractRightLayerOneOrig, refractRightLayerOneDir, spheres, refractRefractLayerTwoPoint, refractRefractLayerTwoN, refractRefractLayerTwoMaterial)) {
        refractRightLayerOneColor = background;
    }
    refractRightLayerOneColor = background;
//Purple
    if (!(depthZeroRefractColor[0] == .2 && depthZeroRefractColor[1] == .7 && depthZeroRefractColor[2] == .8)) {
        diffuse_light_intensity = 0, specular_light_intensity = 0;
        for (size_t i = 0; i < lights.size(); i++) {
            Vec3f light_dir = (lights[i].position - refractLayerOnePoint).normalize();
            float light_distance = (lights[i].position - refractLayerOnePoint).norm();

            Vec3f shadow_orig = light_dir * refractLayerOneN < 0 ? refractLayerOnePoint - refractLayerOneN * 1e-3 :
                                refractLayerOnePoint +
                                refractLayerOneN * 1e-3; // checking if the point lies in the shadow of the lights[i]
            Vec3f shadow_pt, shadow_N;
            Material tmpmaterial;
            if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) &&
                (shadow_pt - shadow_orig).norm() < light_distance)
                continue;

            diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir * refractLayerOneN);
            specular_light_intensity +=
                    powf(std::max(0.f, -reflect(-light_dir, refractLayerOneN) * depthZeroRefractDir),
                         refractLayerOneMaterial.specular_exponent) * lights[i].intensity;
        }
        depthZeroRefractColor =
                refractLayerOneMaterial.diffuse_color * diffuse_light_intensity * refractLayerOneMaterial.albedo[0] +
                Vec3f(1., 1., 1.) * specular_light_intensity * refractLayerOneMaterial.albedo[1] +
                reflectRightLayerOneColor * refractLayerOneMaterial.albedo[2] +
                refractRightLayerOneColor * refractLayerOneMaterial.albedo[3];
//        cout << "Refract color: " << depthZeroRefractColor << " albedo: " << refractLayerOneMaterial.albedo << "\n";
//        cout << "Reflect refract color: " << reflectRightLayerOneColor << "Refract refract color: " << refractRightLayerOneColor << "\n";

    }
    //Black
    diffuse_light_intensity = 0, specular_light_intensity = 0;
    for (size_t i=0; i<lights.size(); i++) {
        Vec3f light_dir      = (lights[i].position - baseLayerPoint).normalize();
        float light_distance = (lights[i].position - baseLayerPoint).norm();

        Vec3f shadow_orig = light_dir*baseLayerN < 0 ? baseLayerPoint - baseLayerN*1e-3 : baseLayerPoint + baseLayerN*1e-3; // checking if the point lies in the shadow of the lights[i]
        Vec3f shadow_pt, shadow_N;
        Material tmpmaterial;
        if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt-shadow_orig).norm() < light_distance)
            continue;

        diffuse_light_intensity  += lights[i].intensity * std::max(0.f, light_dir*baseLayerN);
        specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, baseLayerN)*dir), baseLayerMaterial.specular_exponent)*lights[i].intensity;
    }
//    cout << "---------------------------------\n";
    return baseLayerMaterial.diffuse_color * diffuse_light_intensity * baseLayerMaterial.albedo[0] + Vec3f(1., 1., 1.)*specular_light_intensity * baseLayerMaterial.albedo[1] + depthZeroReflectColor*baseLayerMaterial.albedo[2] + depthZeroRefractColor*baseLayerMaterial.albedo[3];
}
////////////////////////////////DEPTH 1//////////////////////////////////////////////////

void render(const std::vector<Sphere> &spheres, const std::vector<Light> &lights) {
    const int width    = 1024;
    const int height   = 768;
    const int fov      = 1.05;
    std::vector<Vec3f> framebuffer(width*height);

//    #pragma omp parallel for
    for (size_t j = 0; j<height; j++) {
        for (size_t i = 0; i<width; i++) {
            float x =  (2*(i + 0.5)/(float)width  - 1)*tan(fov/2.)*width/(float)height;
            float y = -(2*(j + 0.5)/(float)height - 1)*tan(fov/2.);
            Vec3f dir = Vec3f(x, y, -1).normalize();
            framebuffer[i + j * width] = cast_ray(Vec3f(0, 0, 0), dir, spheres, lights, 0);
//            if (framebuffer[i + j * width][0] == 0 && framebuffer[i + j * width][1] == 0 && framebuffer[i + j * width][2] == 0) {
//                cout << j << " " << i << "\n";
//            }
        }
    }

    std::ofstream ofs;
    ofs.open("./attemptDepth1.ppm", std::ofstream::out | std::ofstream::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";

    for (size_t i = 0; i < height*width; ++i) {
        Vec3f &c = framebuffer[i];
        float max = std::max(c[0], std::max(c[1], c[2]));
        if (max>1) c = c*(1./max);
        for (size_t j = 0; j<3; j++) {
            ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
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
