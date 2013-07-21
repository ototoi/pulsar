#ifndef __PULSAR_MESH_LOADER_H__
#define __PULSAR_MESH_LOADER_H__

#include "Triangle.h"
#include <vector>

namespace pulsar
{

	class MeshLoader
	{
	public:
		virtual ~MeshLoader(){;}
		virtual bool load(std::vector<Vector3>& verts, std::vector<Triangle>& tris)const=0;
	};

}

#endif