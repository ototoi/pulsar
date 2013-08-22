#ifndef __PULSAR_RAY_H__
#define __PULSAR_RAY_H__

#include "Vector3.h"

namespace pulsar{
	
	class Ray
	{
	public:
		Ray(const Vector3& org, const Vector3& dir):org_(org), dir_(dir){
			idir_ = Inverse(dir);
			data_ = Phase(dir);
		}
		const Vector3& org()const{return org_;}
		const Vector3& dir()const{return dir_;}
		const Vector3& idir()const{return idir_;}
		int phase()const{return data_;}
	public:
		static 
		Vector3 Inverse(const Vector3& v)
		{
			return Vector3(1.0f/v[0], 1.0f/v[1], 1.0f/v[2]);
		}
		static
		int     Phase(const Vector3& v)
		{
			int phase = 0;
			if(v[0]<0)phase |= 1;
			if(v[1]<0)phase |= 2;
			if(v[2]<0)phase |= 4;
			return phase;
		}
	private:
		Vector3 org_;
		Vector3 dir_;
		Vector3 idir_;
		int     data_;
	};
	
}

#endif