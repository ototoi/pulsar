#ifndef __PULSAR_LIGHT_H__
#define __PULSAR_LIGHT_H__

#include "Intersection.h"

namespace pulsar{

	class Light
	{
	public:
		virtual ~Light(){}
		virtual void cast(Vector3& col, Vector3& dir, const Intersection& info)const=0;
	};

	class PointLight:public Light
	{
	public:
		PointLight(const Vector3& col, const Vector3& pos);
		void cast(Vector3& col, Vector3& dir, const Intersection& info)const;
	private:
		Vector3 col_;
		Vector3 pos_; 
	};

	class DirectionalLight:public Light
	{
	public:
		DirectionalLight(const Vector3& col, const Vector3& dir);
		void cast(Vector3& col, Vector3& dir, const Intersection& info)const;
	private:
		Vector3 col_;
		Vector3 dir_; 
	};


}

#endif