#ifndef __PULSAR_PLY_MESH_LOADER_H__
#define __PULSAR_PLY_MESH_LOADER_H__

#include "MeshLoader.h"
#include <string>

namespace pulsar
{

	class PLYMeshLoader:public MeshLoader
	{
	public:
		PLYMeshLoader(const char* szFileName);
		~PLYMeshLoader();
		virtual bool load(std::vector<Vector3>& verts, std::vector<Triangle>& tris)const;
	protected:
		std::string strFileName_;
	};

}

#endif