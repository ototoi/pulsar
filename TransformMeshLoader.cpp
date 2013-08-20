#include "TransformMeshLoader.h"

namespace pulsar{

	TransformMeshLoader::TransformMeshLoader(MeshLoader* ml, const Matrix4& mat)
		:ml_(ml), mat_(mat)
	{

	}

	TransformMeshLoader::~TransformMeshLoader()
	{
		if(ml_)delete ml_;
	}

	bool TransformMeshLoader::load(std::vector<Vector3>& verts, std::vector<Triangle>& tris)const
	{
		if(!ml_)return false;
		if(ml_->load(verts, tris))
		{
			for(size_t i=0;i<verts.size();i++)
			{
				verts[i] = Multiply(mat_, verts[i]);
			}
			return true;
		}
		return false;
	}

}