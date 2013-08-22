#ifndef __PULSAR_PATH_TRACER_H__
#define __PULSAR_PATH_TRACER_H__

#include "Ray.h"
#include "Vector3.h"
#include "Random.h"
#include "Intersection.h"
#include "Object.h"

namespace pulsar{

	class PathTracer
	{
	public:
		PathTracer(const Object* pObj):pObj_(pObj){}
		Vector3 radiance(const Ray &ray, const Random& rnd, int depth)const;
	protected:
		bool intersect_scene(Intersection *intersection, const Ray &ray)const;
	protected:
		const Object* pObj_;
	};
}

#endif