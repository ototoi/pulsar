#ifndef __PULSAR_MATERIAL_H__
#define __PULSAR_MATERIAL_H__

#include "Intersection.h"
#include "Object.h"

namespace pulsar{
	
	class Material
	{
	public:
		virtual ~Material(){}
		virtual Vector3 shade(const Intersection& info)const=0;
	};
	
	
}

#endif