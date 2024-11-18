#include <iostream>
#include <vector>

#include "CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"

namespace CGL {

    Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes)
    {
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
            // calculate the force which give to b
            auto vectorB2A = s->m2->position - s->m1->position;
            auto distanceB2A = vectorB2A.norm();
            auto dirB2A = vectorB2A / distanceB2A;
            auto forceB2A = - s->k * dirB2A * (distanceB2A - s->rest_length);
            s->m2->forces += forceB2A;

            // calculate the force which give to a
            auto vectorA2B = s->m1->position - s->m2->position;
            auto distanceA2B = vectorA2B.norm();
            auto dirA2B = vectorA2B / distanceA2B;
            auto forceA2B = - s->k * dirA2B * (distanceA2B - s->rest_length);
            s->m1->forces += forceA2B;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position
                auto forces = m->forces + gravity;
                auto acc = forces / m->mass;
                auto last_frame_velocity = m->velocity;
                m->velocity = m->velocity + acc * delta_t;
                //m->position = m->position + m->velocity * delta_t; // semi-implicit Euler
                m->position = m->position + last_frame_velocity * delta_t; // explicit Euler
                //m->position = m->position + (m->velocity + last_frame_velocity) * delta_t / 2; // my own method

                // the method of last two can trigger the line to be in chaos, going large very quickly, why?

                // TODO (Part 2): Add global damping
            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }

    void Rope::simulateVerlet(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
            // calculate the force which give to b
            auto vectorB2A = s->m2->position - s->m1->position;
            auto distanceB2A = vectorB2A.norm();
            auto dirB2A = vectorB2A / distanceB2A;
            auto forceB2A = - s->k * dirB2A * (distanceB2A - s->rest_length);
            s->m2->forces += forceB2A;
            
            // calculate the force which give to a
            auto vectorA2B = s->m1->position - s->m2->position;
            auto distanceA2B = vectorA2B.norm();
            auto dirA2B = vectorA2B / distanceA2B;
            auto forceA2B = - s->k * dirA2B * (distanceA2B - s->rest_length);
            s->m1->forces += forceA2B;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                Vector2D temp_position = m->position;
                // TODO (Part 3.1): Set the new position of the rope mass
                auto forces = m->forces + gravity;
                auto acc = forces / m->mass;
                m->position = m->position + m->position - m->last_position + acc * acc * delta_t;

                m->last_position = temp_position;
                
                // TODO (Part 4): Add global Verlet damping
            }
        }
    }
}
