#include <iostream>
#include <vector>

#include "CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"

#define EXPLICI
#define VERLET

namespace CGL {

    Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes)
    {
        if (num_nodes < 2) return;
        // Create the masses
        for (int i = 0; i < num_nodes; i++) {
            Mass *m = new Mass(start + (end - start) * i / num_nodes, node_mass, false);
            masses.push_back(m);
        }

        for (auto &i : pinned_nodes) {
            masses[i]->pinned = true;
        }

        // Create the springs
        for (int i = 0; i < num_nodes - 1; i++) {
            Spring *s = new Spring(masses[i], masses[i + 1], k);
            springs.push_back(s);
        }
    }

    void Rope::simulateEuler(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 2): Use Hooke's law to calculate the force on a node
            auto ab = s->m2->position - s->m1->position;
            auto f = s->k * (ab.norm() - s->rest_length) * ab.unit();
            s->m1->forces += f;
            s->m2->forces -= f;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position
                m->forces += gravity;
                m->forces += - friction_coefficient * m->velocity;
                auto acc = m->forces / m->mass;

                #ifdef EXPLICIT
                // explicit Euler
                m->position = m->position + m->velocity * delta_t; 
                m->velocity = m->velocity + acc * delta_t;
                #else
                // semi-implicit Euler
                m->velocity = m->velocity + acc * delta_t;
                m->position = m->position + m->velocity * delta_t;
                #endif
                //m->position = m->position + (m->velocity + last_frame_velocity) * delta_t / 2; // my own method
            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }

    void Rope::simulateVerlet(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            #ifdef VERLET
            // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
            auto ab = s->m2->position - s->m1->position;
            auto f = s->k * (ab.norm() - s->rest_length) * ab.unit();
            s->m1->forces += f;
            s->m2->forces -= f;
            #endif
        }

        for (auto &m : masses)
        {
            #ifdef VERLET
            if (!m->pinned)
            {
                Vector2D temp_position = m->position;
                // TODO (Part 3.1): Set the new position of the rope mass
                // TODO (Part 4): Add global Verlet damping
                m->forces += gravity;
                auto acc = m->forces / m->mass;
                m->position = m->position + (1 - damping_factor) * (m->position - m->last_position) + acc * delta_t * delta_t;

                m->last_position = temp_position;
            }
            #endif
        }
    }
}
