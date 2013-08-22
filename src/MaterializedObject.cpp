#include "MaterializedObject.h"
#include "Intersection.h"

namespace pulsar{

	MaterializedObject::MaterializedObject(Object* o, Material* m)
		:pObj_(o), pMat_(m){}

	MaterializedObject::~MaterializedObject()
	{
		if(pObj_)delete pObj_;
		if(pMat_)delete pMat_;
	}

	bool MaterializedObject::intersect(Intersection* info, const Ray& r, float tmin, float tmax)const
	{
		if(pObj_->intersect(info, r, tmin, tmax))
		{
			if(pMat_){
					info->pMaterial = pMat_;
			}
			return true;
		}
		return false;
	}

	Bound MaterializedObject::bound()const
	{
		return pObj_->bound();
	}


}