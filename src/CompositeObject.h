#ifndef __PULSAR_COMPOSITE_OBJECT_H__
#define __PULSAR_COMPOSITE_OBJECT_H__

#include "Object.h"
#include <Vector>

namespace pulsar{
	
	class CompositeObject:public Object{
	public:
		 CompositeObject();
		~CompositeObject();
		virtual bool intersect(Intersection* info, const Ray& r, float tmin, float tmax)const;
		virtual Bound bound()const;
	public:
		void add(Object* obj);
		size_t size()const;
	private:
		std::vector<Object*> obj_;
		Bound bnd_;
	};
	
}

#endif