#include "MeshObject.h"
#include "Triangle.h"
#include "PLQBVHMeshAccelerator.h"
#include <vector>

namespace pulsar{

	class MeshObjectImp
	{
	public:
		MeshObjectImp(const MeshLoader& ml);
		~MeshObjectImp();
		bool intersect(Intersection* info, const Ray& r, float tmin, float tmax)const
		{
			return acc_ && acc_->intersect(info, r, tmin, tmax);
		}
		Bound bound()const
		{
			return acc_->bound();
		}
	protected:
		std::vector<Vector3 > vertices_;
		std::vector<Triangle> faces_;
		PLQBVHMeshAccelerator* acc_;
	};

	MeshObjectImp::MeshObjectImp(const MeshLoader& ml)
		:acc_(0)
	{
		if(ml.load(vertices_,faces_))
		{
			size_t vsz = vertices_.size();
			size_t fsz = faces_.size();
			//convert to pointer
			for(size_t i = 0;i<fsz;i++)
			{
				faces_[i].points[0] = &vertices_[faces_[i].indices[0]];
				faces_[i].points[1] = &vertices_[faces_[i].indices[1]];
				faces_[i].points[2] = &vertices_[faces_[i].indices[2]];
			}
			//
			std::vector<const Triangle*> pTris(fsz);
			for(size_t i = 0;i<fsz;i++)
			{
				pTris[i] = &faces_[i];
			}
			acc_ = new PLQBVHMeshAccelerator(pTris);
		}
	}
	
	MeshObjectImp::~MeshObjectImp()
	{
		if(acc_)delete acc_;
	}


	//-----------------------------------------------------------------------------------------------------------


	MeshObject::MeshObject(const MeshLoader& ml)
	{
		imp_ = new MeshObjectImp(ml);
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