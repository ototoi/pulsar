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
		Object* pIntersection;//
		Material*     pMaterial;	//
	};
	
}

#endif