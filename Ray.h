#ifndef __PULSAR_RAY_H__
#define __PULSAR_RAY_H__

#include "Vector3.h"

namespace pulsar{
	
	class Ray
	{
	public:
		Ray(const Vector3& org, const Vector3& dir):org_(org), dir_(dir){}
		const Vector3& org()const{return org_;}
		const Vector3& dir()const{return dir_;}
	private:
		Vector3 org_;
		Vector3 dir_;
	};
	
}

#endif