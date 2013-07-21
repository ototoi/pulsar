#include "PLQBVHMeshAccelerator.h"

namespace pulsar{

	class PLQBVHMeshAcceleratorImp
	{
	public:
		PLQBVHMeshAcceleratorImp(std::vector<const Triangle*>& tris);
		virtual bool intersect(Intersection* info, const Ray& r, float tmin, float tmax)const;
		virtual Bound bound()const;
	protected:
		Vector3 min_;
		Vector3 max_;
	};

	PLQBVHMeshAcceleratorImp::PLQBVHMeshAcceleratorImp(std::vector<const Triangle*>& tris)
	{

	}

	bool PLQBVHMeshAcceleratorImp::intersect(Intersection* info, const Ray& r, float tmin, float tmax)const
	{
		return false;
	}
	Bound PLQBVHMeshAcceleratorImp::bound()const
	{
		return Bound(min_, max_);
	}

	//---------------------------------------------------------------------------------------
	PLQBVHMeshAccelerator::PLQBVHMeshAccelerator(std::vector<const Triangle*>& tris)
	{
		imp_ = new PLQBVHMeshAcceleratorImp(tris);	
	}

	PLQBVHMeshAccelerator::~PLQBVHMeshAccelerator()
	{
		delete imp_;
	}

	bool PLQBVHMeshAccelerator::intersect(Intersection* info, const Ray& r, float tmin, float tmax)const
	{
		return imp_->intersect(info, r, tmin, tmax);
	}

	Bound PLQBVHMeshAccelerator::bound()const
	{
		return imp_->bound();
	}
	
}
