#ifndef __PULSAR_TRANSFORM_MESH_LOADER_H__
#define __PULSAR_TRANSFORM_MESH_LOADER_H__

#include "MeshLoader.h"
#include "Matrix4.h"

namespace pulsar{

	class TransformMeshLoader:public MeshLoader
	{
	public:
		TransformMeshLoader(MeshLoader* ml, const Matrix4& mat);
		~TransformMeshLoader();
		bool load(std::vector<Vector3>& verts, std::vector<Triangle>& tris)const;
	private:
		MeshLoader* ml_;
		Matrix4 mat_;
	};

}

#endif