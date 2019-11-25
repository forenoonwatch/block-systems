#pragma once

#include <engine/math/matrix.hpp>
#include <engine/serialization/archive.hpp>

#include <engine/ecs/ecs-system.hpp>

class Game;

struct Camera {
	Matrix4f projection;
	Matrix4f view;
	Matrix4f viewProjection;
	
	Matrix4f iView;
	Matrix4f iViewProjection;

	inline Vector3f getLookVector() const { return Vector3f(view[2]); }

	template <class Archive>
	void save(Archive& ar) const {
		ar(Serialization::encodeBinary(&projection, sizeof(Matrix4f)),
				Serialization::encodeBinary(&view, sizeof(Matrix4f)),
				Serialization::encodeBinary(&viewProjection, sizeof(Matrix4f)));
	}
	  
	template <class Archive>
	void load(Archive& ar) {
		ar.loadBinary(this, 3 * sizeof(Matrix4f));

		iView = Math::inverse(view);
		iViewProjection = Math::inverse(viewProjection);
	}
};

struct CameraComponent {
	Camera* camera;

	Vector3f position;
	float rotationX;
	float rotationY;
	Vector3f rayDirection;
};

class UpdateCameraSystem : public ECS::System {
	public:
		virtual void operator()(Game&, float) override;
};

