#ifndef __PULSAR_MATERIALIZED_OBJECT_H__
#define __PULSAR_MATERIALIZED_OBJECT_H__

#include "Object.h"
#include "Material.h"

namespace pulsar{
	
	class MaterializedObject:public Object
	{
	public:
		MaterializedObject(Object* pObj, Material* pMat);
		~MaterializedObject();
		bool intersect(Intersection* info, const Ray& r, float tmin, float tmax)const;
		Bound bound()const;
	private:
		Object*   pObj_;
		Material* pMat_;
	};
	
}

#endif