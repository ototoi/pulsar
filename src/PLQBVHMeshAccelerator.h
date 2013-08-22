#ifndef __PULSAR_PLQBVH_MESH_ACCELERATOR_H__
#define __PULSAR_PLQBVH_MESH_ACCELERATOR_H__

#include "Object.h"
#include "Triangle.h"
#include <vector>


namespace pulsar{

	class PLQBVHMeshAcceleratorImp;
	class PLQBVHMeshAccelerator
	{
	public:
		PLQBVHMeshAccelerator(std::vector<const Triangle*>& tris);
		~PLQBVHMeshAccelerator();
		virtual bool intersect(Intersection* info, const Ray& r, float tmin, float tmax)const;
		virtual Bound bound()const;
	protected:
		PLQBVHMeshAcceleratorImp* imp_;
	};
}

#endif