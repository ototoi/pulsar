#include "CompositeObject.h"
#include <limits>

namespace pulsar
{
	static const float FAR_ = std::numeric_limits<float>::max()*1e-3f;

	CompositeObject::CompositeObject()
		:bnd_(+Vector3(FAR_,FAR_,FAR_),-Vector3(FAR_,FAR_,FAR_))
	{
		;
	}
	CompositeObject::~CompositeObject()
	{
		size_t sz = obj_.size();
		for(size_t i = 0;i<sz;i++)
		{
			delete obj_[i];
		}
	}

	bool CompositeObject::intersect(Intersection* info, const Ray& r, float tmin, float tmax)const
	{
		size_t sz = obj_.size();
		const Object* const * obj = &obj_[0];
		bool bRet = false;
		for(size_t i = 0;i<sz;i++){
			if(obj[i]->intersect(info, r, tmin, tmax))
			{
				tmax = info->t; 
				bRet = true;
			}
		}
		return bRet;
	}
	Bound CompositeObject::bound()const
	{
		return bnd_;
	}

	void CompositeObject::add(Object* obj)
	{
		obj_.push_back(obj);
		bnd_ = grow(bnd_, obj->bound());
	}

	size_t CompositeObject::size()const
	{
		return obj_.size();
	}

}