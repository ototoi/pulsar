#ifndef __PULSAR_CAMERA_H__
#define __PULSAR_CAMERA_H__

#include "Ray.h"
#include "Vector3.h"

namespace pulsar{

	class Camera
	{
	public:
		virtual ~Camera(){}
		virtual Ray shoot(float i, float j)const = 0;
	};

	class PerspectiveCamera:public Camera
	{
	public:
		PerspectiveCamera(const Vector3& from, const Vector3& to, const Vector3& upper, float angle, float aspect);
		Ray shoot(float i, float j)const;
	private:
		Vector3 from_;
		Vector3 axis_x_;
		Vector3 axis_y_;
		Vector3 axis_z_;

		float cof_x_;
		float cof_y_;
	};

}

#endif
