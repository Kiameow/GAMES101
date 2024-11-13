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

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    // confusing code here, need to understand
    // seems randomly choosing a lighting source, and do sampling on it 
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

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
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
    Intersection inter = intersect(ray);
    if (inter.happened) {
        Vector3f L_dir;
        Vector3f L_indir = Vector3f(0.0, 0.0, 0.0);
        float pdf_light = 0.0;
        Vector3f wo = -ray.direction;
        Intersection inter_light;
        sampleLight(inter_light, pdf_light);
        Vector3f x = inter_light.coords;
        Vector3f ws = normalize(x - inter.coords);
        Vector3f NN = inter_light.normal;
        Vector3f emit = inter_light.emit;
        Intersection inter_middle = intersect(Ray(inter.coords, ws));
        // direct light received
        if (inter_middle.happened && inter_middle.obj == inter_light.obj) {
            L_dir = emit * inter.m->eval(wo, ws, NN) * dotProduct(ws, inter.normal) * dotProduct(ws, NN) / std::pow(inter_middle.distance, 2) / pdf_light;
        }

        if(get_random_float() < RussianRoulette) {
            Vector3f wi = normalize(inter.m->sample(wo, inter.normal));
            Ray r(inter.coords, wi);
            Intersection inter_obj = intersect(r);
            if (inter_obj.happened && inter_obj.obj->hasEmit() == false) {
                L_indir = castRay(r, depth) * inter.m->eval(wo, wi, inter.normal) * dotProduct(wi, inter.normal) / inter_obj.m->pdf(wo, wi, inter.normal) / RussianRoulette;
            }
        }
        return L_dir + L_indir;
    }
}
