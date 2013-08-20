#ifndef __PULSAR_INTERSECTION_H__
#define __PULSAR_INTERSECTION_H__

#include "Vector3.h"

namespace pulsar{
	
	class Object;
	class Material;
	
	struct Intersection
	{
		float t;//distance to intersection
		Vector3 position;	//world position
		Vector3 normal;		//world normal
		Vector3 tex;		//
		//
		Vector3 ray_origin;
		Vector3 ray_direction;
		const Object* pObject;//
		const Material*     pMaterial;	//
	};
	
}

#endif