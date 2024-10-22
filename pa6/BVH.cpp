#include <algorithm>
#include <cassert>
#include "BVH.hpp"

BVHAccel::BVHAccel(std::vector<Object*> p, int maxPrimsInNode,
                   SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
      primitives(std::move(p))
{
    time_t start, stop;
    time(&start);
    if (primitives.empty())
        return;

    root = recursiveBuild(primitives);

    time(&stop);
    double diff = difftime(stop, start);
    int hrs = (int)diff / 3600;
    int mins = ((int)diff / 60) - (hrs * 60);
    int secs = (int)diff - (hrs * 3600) - (mins * 60);

    switch (this->splitMethod) {
        case SplitMethod::NAIVE:
            printf("Using Naive split method\n");
            break;
        case SplitMethod::SAH:
            printf("Using SAH split method\n");
            break;
        default:
            break;
    }

    printf(
        "\rBVH Generation complete: \nTime Taken: %i hrs, %i mins, %i secs\n\n",
        hrs, mins, secs);
}

BVHBuildNode* BVHAccel::recursiveBuild(std::vector<Object*> objects)
{
    BVHBuildNode* node = new BVHBuildNode();

    // Compute bounds of all primitives in BVH node
    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());
    if (objects.size() == 1) {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }
    else if (objects.size() == 2) {
        node->left = recursiveBuild(std::vector{objects[0]});
        node->right = recursiveBuild(std::vector{objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        return node;
    }
    else {
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds =
                Union(centroidBounds, objects[i]->getBounds().Centroid());
        int dim = centroidBounds.maxExtent();
        switch (dim) {
        case 0:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().x <
                       f2->getBounds().Centroid().x;
            });
            break;
        case 1:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().y <
                       f2->getBounds().Centroid().y;
            });
            break;
        case 2:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().z <
                       f2->getBounds().Centroid().z;
            });
            break;
        }

        switch(BVHAccel::splitMethod) {
            case BVHAccel::SplitMethod::NAIVE: {
                auto beginning = objects.begin();
                auto middling = objects.begin() + (objects.size() / 2);
                auto ending = objects.end();

                auto leftshapes = std::vector<Object*>(beginning, middling);
                auto rightshapes = std::vector<Object*>(middling, ending);

                assert(objects.size() == (leftshapes.size() + rightshapes.size()));

                node->left = recursiveBuild(leftshapes);
                node->right = recursiveBuild(rightshapes);

                node->bounds = Union(node->left->bounds, node->right->bounds);
                
                break;
            }
            case BVHAccel::SplitMethod::SAH: {
                // TODO: Implement SAH split method
                int bucketSize = 10;
                float minCost = std::numeric_limits<float>::infinity();
                int optimalSplitIndex = -1;
                float totalSurfaceArea = centroidBounds.SurfaceArea();
                for (int i = 1; i < bucketSize; ++i) {
                    auto beginning = objects.begin();
                    auto middling = objects.begin() + (objects.size() * i / bucketSize);
                    auto ending = objects.end();

                    auto leftshapes = std::vector<Object*>(beginning, middling);
                    auto rightshapes = std::vector<Object*>(middling, ending);

                    assert(objects.size() == (leftshapes.size() + rightshapes.size()));

                    Bounds3 leftBounds, rightBounds;
                    for (int j = 0; j < leftshapes.size(); ++j) {
                        leftBounds = Union(leftBounds, leftshapes[j]->getBounds());
                    }
                    for (int j = 0; j < rightshapes.size(); ++j) {
                        rightBounds = Union(rightBounds, rightshapes[j]->getBounds());
                    }

                    float cost = (leftshapes.size() * leftBounds.SurfaceArea() 
                               + rightshapes.size() * rightBounds.SurfaceArea()) 
                               / totalSurfaceArea;

                    if (cost < minCost) {
                        minCost = cost;
                        optimalSplitIndex = i;
                    }
                }
                auto beginning = objects.begin();
                auto middling = objects.begin() + (objects.size() * optimalSplitIndex / bucketSize);
                auto ending = objects.end();

                auto leftshapes = std::vector<Object*>(beginning, middling);
                auto rightshapes = std::vector<Object*>(middling, ending);

                assert(objects.size() == (leftshapes.size() + rightshapes.size()));

                node->left = recursiveBuild(leftshapes);
                node->right = recursiveBuild(rightshapes);

                node->bounds = Union(node->left->bounds, node->right->bounds);
                break;
            }
            default:
                break;
        }
    }

    return node;
}

Intersection BVHAccel::Intersect(const Ray& ray) const
{
    Intersection isect;
    if (!root)
        return isect;
    isect = BVHAccel::getIntersection(root, ray);
    return isect;
}

Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const
{
    // DOING Traverse the BVH to find intersection
    // directly returning the isect means no intersection happened
    Intersection isect;
    const auto invDir = Vector3f(1.0f / ray.direction.x, 1.0f / ray.direction.y, 1.0f / ray.direction.z);
    const auto dirIsNeg = std::array<int, 3>{int(invDir.x < 0), int(invDir.y < 0), int(invDir.z < 0)};
    
    if (node->bounds.IntersectP(ray, invDir, dirIsNeg)) {
        BVHBuildNode* selected_node = nullptr;
        if (node->left != nullptr || node->right != nullptr) {
            Intersection leftIsect, rightIsect;
            
            if (node->left != nullptr) {
                leftIsect = getIntersection(node->left, ray);
            }
            
            if (node->right != nullptr) {
                rightIsect = getIntersection(node->right, ray);
            }
            
            // 比较左右子树的交点，返回较近的那个
            if (leftIsect.happened && rightIsect.happened) {
                selected_node = (leftIsect.distance < rightIsect.distance) ? node->left : node->right;
            }
            else if (leftIsect.happened) {
                selected_node = node->left;
            }
            else if (rightIsect.happened) {
                selected_node = node->right;
            }
            
            if (selected_node != nullptr) {
                return getIntersection(selected_node, ray);
            }
            return isect;
        }
        // if node is a leaf node, check if ray intersects with any primitive in the leaf node 
        else if (node->object != nullptr) {
            return node->object->getIntersection(ray);
        }
    }

    return isect;
}
