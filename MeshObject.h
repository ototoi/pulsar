#ifndef __PULSAR_MESH_OBJECT_H__
#define __PULSAR_MESH_OBJECT_H__

#include "Object.h"
#include "MeshLoader.h"

namespace pulsar{

	class MeshObjectImp;
	class MeshObject:public Object
	{
	public:
		MeshObject(const MeshLoader& ml);
		~MeshObject();
		virtual bool intersect(Intersection* info, const Ray& r, float tmin, float tmax)const;
		virtual Bound bound()const;
	protected:
		MeshObjectImp* imp_;
	};

}

#endif