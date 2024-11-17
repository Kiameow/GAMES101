//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

// pos is the position of the intersection point on light source
void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    // this method sum the area of all light sources, then uniformly sample one point
    // this method is not very accurate, because the bigger light source has more chance to be sampled
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            // only do one sampling
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of Path Tracing (efficient version -- has light view)
// TO DO Implement Path Tracing Algorithm here
    /*shade(p, wo)
    sampleLight(inter , pdf_light)
    Get x, ws, NN, emit from inter
    Shoot a ray from p to x
    If the ray is not blocked in the middle
    L_dir = emit * eval(wo, ws, N) * dot(ws, N) * dot(ws,NN) / |x-p|^2 / pdf_light

    L_indir = 0.0
    Test Russian Roulette with probability RussianRoulette
    wi = sample(wo, N)
    Trace a ray r(p, wi)
    If ray r hit a non-emitting object at q
    L_indir = shade(q, wi) * eval(wo, wi, N) * dot(wi, N)/ pdf(wo, wi, N) / RussianRoulette
    Return L_dir + L_indir*/
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // 1. cast a ray from camera to scene, judge if it hits object
    Intersection inter_shading_point = intersect(ray);
    // 2. if it hits normal object, calculate radiance from the point of intersection (which is also the irradiance of the point); if it hits light source, just return the emit value of the light source
    if (inter_shading_point.happened && inter_shading_point.m->hasEmission() == true) {
        return inter_shading_point.m->getEmission();
    }

    if (inter_shading_point.happened) {
        // the radiance part is consisted of two parts: direct lighting and indirect lighting
        Vector3f L_dir = Vector3f(0.0, 0.0, 0.0); // direct lighting
        Vector3f L_indir = Vector3f(0.0, 0.0, 0.0); // indirect lighting

        // get the direct lighting
        float pdf_light = 0.0;
        Vector3f wo = -ray.direction;
        Intersection inter_light;
        sampleLight(inter_light, pdf_light);

        // Calculate direction and distance to light
        Vector3f l2p = inter_shading_point.coords - inter_light.coords;  // point to light vector
        float dist2light = l2p.norm();  // distance to light
        Vector3f l2p_dirction = normalize(l2p); 
        Vector3f ws = -normalize(l2p);  // normalized direction to light

        // Cast shadow ray
        Intersection inter_middle = intersect(Ray(inter_light.coords, l2p_dirction));

        // Check if light is visible by verifying direction consistency
        if (inter_middle.happened) {
            // Calculate direction from middle point to light
            auto middle_to_shading_dis = std::fabs((inter_shading_point.coords - inter_middle.coords).norm());
            // Only calculate direct lighting if directions match (within small epsilon)
            // and the intersection distance matches the distance to light
            if (middle_to_shading_dis < 0.1f) {
                // L_dir = emit * eval(wo, ws, N) * dot(ws, N) * dot(ws,NN) / |x-p|^2 / pdf_light
                Vector3f NN = inter_light.normal;
                Vector3f emit = inter_light.emit;

                Vector3f f_r = inter_shading_point.m->eval(wo, ws, inter_shading_point.normal);
                float cosA = std::max(dotProduct(ws, inter_shading_point.normal), 0.0f);
                float cosB = std::max(dotProduct(-ws, NN), 0.0f);
                
                L_dir = emit * f_r * cosA * cosB / (dist2light * dist2light) / pdf_light;
            }
        }
        
        // get the indirect lighting
        if(get_random_float() < Scene::RussianRoulette) {
            Vector3f dir2nextObj = normalize(inter_shading_point.m->sample(wo, inter_shading_point.normal));
            Ray r(inter_shading_point.coords, dir2nextObj);
            Intersection inter_obj = intersect(r);
            // if the ray hits the light source, leave out because it's already been considered in direct lighting
            if (inter_obj.happened && inter_obj.obj->hasEmit() == false) {
                Vector3f f_r = inter_shading_point.m->eval(wo, dir2nextObj, inter_shading_point.normal);
                float cosA = std::max(dotProduct(dir2nextObj, inter_shading_point.normal), 0.0f);
                float pdf = inter_obj.m->pdf(wo, dir2nextObj, inter_shading_point.normal);

                L_indir = castRay(r, depth + 1) * f_r * cosA / pdf / Scene::RussianRoulette;
            }
        }
        return L_dir + L_indir;
    }
    // 3. if it doesn't hit object, return radiance of zero
    return Vector3f(0.0, 0.0, 0.0);
}
