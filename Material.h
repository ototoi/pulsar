#ifndef __PULSAR_MATERIAL_H__
#define __PULSAR_MATERIAL_H__

#include "Intersection.h"
#include "Object.h"
#include "Light.h"

#include <vector>

namespace pulsar{

	enum RefectionType
	{
		REFLECTION_TYPE_DIFFUSE,
		REFLECTION_TYPE_SPECULAR,
		REFLECTION_TYPE_REFRACTION
	};
	
	class Material
	{
	public:
		virtual ~Material(){}
		virtual Vector3 shade(const Intersection& info,  const std::vector<const Light*>& l)const=0;
		virtual Vector3 diffuse()const=0;
		virtual Vector3 emission()const=0;
		virtual RefectionType type()const=0;
		virtual float Ior()const{return 1.0f;}
	};

	class EmissionMaterial:public Material
	{
	public:
		EmissionMaterial(const Vector3& Ka);
		Vector3 shade(const Intersection& info,  const std::vector<const Light*>& l)const;
		Vector3 diffuse()const{return Vector3(0,0,0);}
		Vector3 emission()const{return Ka_;}
		RefectionType type()const{return REFLECTION_TYPE_DIFFUSE;}
	private:
		Vector3 Ka_;
	}; 

	class MirrorMaterial:public Material
	{
	public:
		MirrorMaterial(const Vector3& Ka):Ka_(Ka){}
		Vector3 shade(const Intersection& info,  const std::vector<const Light*>& l)const{return Vector3(0,0,0);}
		Vector3 diffuse()const{return Ka_;}
		Vector3 emission()const{return Vector3(0,0,0);}
		RefectionType type()const{return REFLECTION_TYPE_SPECULAR;}
	private:
		Vector3 Ka_;
	};

	class GlassMaterial:public Material
	{
	public:
		GlassMaterial(const Vector3& Ka):Ka_(Ka){}
		Vector3 shade(const Intersection& info,  const std::vector<const Light*>& l)const{return Vector3(0,0,0);}
		Vector3 diffuse()const{return Ka_;}
		Vector3 emission()const{return Vector3(0,0,0);}
		RefectionType type()const{return REFLECTION_TYPE_REFRACTION;}
		float Ior()const{return 1.4f;}
	private:
		Vector3 Ka_;
	};

	class LambertMaterial:public Material
	{
	public:
		LambertMaterial(const Vector3& Kd);
		Vector3 shade(const Intersection& info,  const std::vector<const Light*>& l)const;
		Vector3 diffuse()const{return Kd_;}
		Vector3 emission()const{return Vector3(0,0,0);}
		RefectionType type()const{return REFLECTION_TYPE_DIFFUSE;}
	private:
		Vector3 Kd_;
	};
	
}

#endif