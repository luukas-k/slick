#include "PhysicsSystem.h"

#include "components/Transform.h"
#include "components/RigidBody.h"

namespace Slick::Physics {
	
	PhysicsSystem::PhysicsSystem() {}

	PhysicsSystem::~PhysicsSystem() {}

	void PhysicsSystem::fixed_update(App::Scene& scene, ECS::Manager& mgr, float dt) {
		struct Body {
			TransformComponent* tf;
			RigidBody* rb;
			SphereCollider* coll;
		};
		std::vector<Body> bodies;

		// Collect
		mgr.view<TransformComponent, RigidBody, SphereCollider>([&](u32 e, TransformComponent* tc, RigidBody* rb, SphereCollider* sc) {
			bodies.push_back({
				tc,
				rb,
				sc
			});
		});

		for (auto& [tf, rb, sc] : bodies) {
			tf->position = tf->position + rb->velocity * dt;
			tf->rotation = tf->rotation * Math::axis_rotation(Math::normalize(rb->angularVelocity), Math::length(rb->angularVelocity) * dt);
			
			float Beta = 0.1f;
			if (tf->position.y < 0.f) {
				rb->velocity.y = -(Beta / dt) * tf->position.y;
			}

			rb->velocity.y -= 9.81f * dt;
		}

		struct Contact {
			Body* a;
			Body* b;
			Math::fVec3 ra, rb;
			Math::fVec3 n;
		};

		std::vector<Contact> contacts;

		for (u32 i = 0; i < bodies.size(); i++) {
			auto& a = bodies[i];
			for (u32 j = i + 1; j < bodies.size(); j++) {
				auto& b = bodies[j];
				
				float dist = Math::distance(a.tf->position, b.tf->position);
				Math::fVec3 towards = b.tf->position - a.tf->position;

				if (dist < a.coll->radius + b.coll->radius) {
					contacts.push_back({ 
						.a = &bodies[i], 
						.b = &bodies[j],
						.ra = normalize(towards) * a.coll->radius,
						.rb = -normalize(towards) * b.coll->radius,
						.n = normalize(towards)
					});
				}
			}
		}
		
		for (auto& col : contacts) {
			auto jva = -col.n;
			auto jwa = -Math::cross(col.ra, col.n);
			auto jvb = col.n;
			auto jwb = Math::cross(col.rb, col.n);
			float jv = 
				Math::dot(jva, col.a->rb->velocity) +
				Math::dot(jwa, col.a->rb->angularVelocity) + 
				Math::dot(jvb, col.b->rb->velocity) +
				Math::dot(jwb, col.b->rb->angularVelocity);

			float b = 0.05f;
			float effMass = 1.f;
			float lambda = effMass * (-(jv + b));

			col.a->rb->velocity			= col.a->rb->velocity		 + jva * 1.f * lambda;
			col.a->rb->angularVelocity	= col.a->rb->angularVelocity + jwa * 1.f * lambda;
			col.b->rb->velocity			= col.b->rb->velocity		 + jvb * 1.f * lambda;
			col.b->rb->angularVelocity	= col.b->rb->angularVelocity + jwb * 1.f * lambda;
		}
	}

}

