#include "MeshObject.h"
#include "Triangle.h"
#include <vector>

namespace pulsar{

	class MeshObjectImp
	{
	public:
		MeshObjectImp() {}
		~MeshObjectImp(){}
		bool intersect(Intersection* info, const Ray& r, float tmin, float tmax)const
		{
			return false;
		}
		Bound bound()const
		{
			return Bound(min_, max_); 
		}
	protected:
		std::vector<Vector3 > vertices_;
		std::vector<Triangle> faces_;
		Vector3 min_;
		Vector3 max_;
	};


	MeshObject::MeshObject()
	{
		imp_ = new MeshObjectImp();
	}
	MeshObject::~MeshObject()
	{
		delete imp_;
	}
	bool MeshObject::intersect(Intersection* info, const Ray& r, float tmin, float tmax)const
	{
		return imp_->intersect(info, r, tmin, tmax);
	}
	Bound MeshObject::bound()const
	{
		return imp_->bound();
	}

}