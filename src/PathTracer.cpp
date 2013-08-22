#include "PathTracer.h"
#include "Intersection.h"
#include "Object.h"
#include "Material.h"
#include <limits>

namespace pulsar{

	static const Vector3 BackgroundColor = Vector3(0,0,0);
	const float kPI = 3.14159265358979323846f;
	const float kINF = std::numeric_limits<float>::max();
	const float kEPS = 1e-6f;
	static const int Depth = 5; // ���V�A�����[���b�g�őł��؂�Ȃ��ő�[�x
	static const int DpethLimit = 64;


	bool PathTracer::intersect_scene(Intersection *info, const Ray &ray)const
	{
		if(pObj_->intersect(info, ray, 0.1f, 100000.0f))
		{
			info->ray_origin = ray.org();
			info->ray_direction = ray.dir();
			return true;
		}
		return false;
	}

	Vector3 PathTracer::radiance(const Ray &ray, const Random& rnd, int depth)const
	{
		Intersection info;
		info.pMaterial = 0;
		// �V�[���ƌ�������
		if (!intersect_scene(&info, ray))return BackgroundColor;
		if (!info.pMaterial)return BackgroundColor;

		Vector3 orienting_normal = info.normal;// �����ʒu�̖@���i���̂���̃��C�̓��o���l���j
		if(dot(orienting_normal, ray.dir())>0){
			orienting_normal = orienting_normal*-1.0f;
		}
		// �F�̔��˗��ő�̂��̂𓾂�B���V�A�����[���b�g�Ŏg���B
		// ���V�A�����[���b�g��臒l�͔C�ӂ����F�̔��˗������g���Ƃ��ǂ��B
		Vector3 difColor = info.pMaterial->diffuse();
		double russian_roulette_probability = std::max(difColor[0], std::max(difColor[1], difColor[2]));

		// ���ˉ񐔂����ȏ�ɂȂ����烍�V�A�����[���b�g�̊m�����}�㏸������B�i�X�^�b�N�I�[�o�[�t���[�΍�j
		if (depth > DpethLimit){
			russian_roulette_probability *= pow(0.5, depth - DpethLimit);
		}
		// ���V�A�����[���b�g�����s���ǐՂ�ł��؂邩�ǂ����𔻒f����B
		// ������Depth��̒ǐՂ͕ۏႷ��B
		if (depth > Depth) {
			if (rnd.next01() >= russian_roulette_probability){
				return info.pMaterial->emission();
			}
		} else {
			russian_roulette_probability = 1.0; // ���V�A�����[���b�g���s���Ȃ�����
		}

		Vector3 incoming_radiance = Vector3(0,0,0);
		Vector3 weight = Vector3(1,1,1);
	
		int nType = info.pMaterial->type();
		switch (nType)
		{
		case REFLECTION_TYPE_DIFFUSE: // ���S�g�U��
		{
			// orienting_normal�̕�������Ƃ������K�������(w, u, v)�����B���̊��ɑ΂��锼�����Ŏ��̃��C���΂��B
			Vector3 w, u, v;
			w = orienting_normal;
			if (fabs(w[0]) > kEPS) // �x�N�g��w�ƒ�������x�N�g�������Bw.x��0�ɋ߂��ꍇ�Ƃ����łȂ��ꍇ�ƂŎg���x�N�g����ς���B
				u = normalize(cross(Vector3(0.0, 1.0, 0.0), w));
			else
				u = normalize(cross(Vector3(1.0, 0.0, 0.0), w));

			v = cross(w, u);
			// �R�T�C�������g�����d�_�I�T���v�����O
			float r1 = 2 * kPI * rnd.next01();
			float r2 = rnd.next01(), r2s = sqrt(r2);
			Vector3 dir = normalize((
				u * cos(r1) * r2s +
				v * sin(r1) * r2s +
				w * sqrt(1.0 - r2)));

			incoming_radiance = radiance(Ray(info.position, dir), rnd, depth+1);
			// �����_�����O�������ɑ΂��郂���e�J�����ϕ����l����ƁAoutgoing_radiance = weight * incoming_radiance�B
			// �����ŁAweight = (��/��) * cos�� / pdf(��) / R �ɂȂ�B
			// ��/�΂͊��S�g�U�ʂ�BRDF�Ńς͔��˗��Acos�Ƃ̓����_�����O�������ɂ�����R�T�C�����Apdf(��)�̓T���v�����O�����ɂ��Ă̊m�����x�֐��B
			// R�̓��V�A�����[���b�g�̊m���B
			// ���A�R�T�C�����ɔ�Ⴕ���m�����x�֐��ɂ��T���v�����O���s���Ă��邽�߁Apdf(��) = cos��/��
			// ����āAweight = ��/ R�B
			weight = info.pMaterial->diffuse() * (1.0f/russian_roulette_probability);
		} 
		break;

		// ���S����
		case REFLECTION_TYPE_SPECULAR: 
		{
			// ���S���ʂȂ̂Ń��C�̔��˕����͌���I�B
			// ���V�A�����[���b�g�̊m���ŏ��Z����̂͏�Ɠ����B
			incoming_radiance = radiance(Ray(info.position, ray.dir() - info.normal * 2.0 * dot(info.normal, ray.dir())), rnd, depth+1);
			weight = info.pMaterial->diffuse() * (1.0f/russian_roulette_probability);
		}
		break;

		// ���ܗ�kIor�̃K���X
		case REFLECTION_TYPE_REFRACTION:
		{
			const Ray reflection_ray = Ray(info.position, ray.dir() - info.normal * 2.0 * dot(info.normal, ray.dir()));
			const bool into = dot(info.normal, orienting_normal) > 0.0; // ���C���I�u�W�F�N�g����o��̂��A����̂�

			// Snell�̖@��
			float nc = 1.0; // �^��̋��ܗ�
			float nt = info.pMaterial->Ior(); // �I�u�W�F�N�g�̋��ܗ�
			float nnt = into ? nc / nt : nt / nc;
			float ddn = dot(ray.dir(), orienting_normal);
			float cos2t = 1.0 - nnt * nnt * (1.0 - ddn * ddn);
			
			if (cos2t < 0.0) { // �S����
				incoming_radiance = radiance(reflection_ray, rnd, depth+1);
				weight = info.pMaterial->diffuse() * (1.0f / russian_roulette_probability);
				break;
			}
			// ���܂̕���
			const Ray refraction_ray = Ray(info.position,
				normalize(ray.dir() * nnt - info.normal * (into ? 1.0 : -1.0) * (ddn * nnt + sqrt(cos2t))));
			
			// Schlick�ɂ��Fresnel�̔��ˌW���̋ߎ����g��
			float a = nt - nc, b = nt + nc;
			float R0 = (a * a) / (b * b);

			float c = 1.0 - (into ? -ddn : dot(refraction_ray.dir(), -1.0 * orienting_normal));
			float Re = R0 + (1.0 - R0) * pow(c, 5.0f); // ���˕����̌������˂���ray.dir�̕����ɉ^�Ԋ����B�����ɋ��ܕ����̌������˂�������ɉ^�Ԋ����B
			float nnt2 = pow(into ? nc / nt : nt / nc, 2.0f); // ���C�̉^�ԕ��ˋP�x�͋��ܗ��̈قȂ镨�̊Ԃ��ړ�����Ƃ��A���ܗ��̔�̓��̕������ω�����B
			float Tr = (1.0 - Re) * nnt2; // ���ܕ����̌������܂���ray.dir�̕����ɉ^�Ԋ���
			
			// ���ȏヌ�C��ǐՂ�������܂Ɣ��˂̂ǂ��炩�����ǐՂ���B�i�����Ȃ��Ǝw���I�Ƀ��C��������j
			// ���V�A�����[���b�g�Ō��肷��B
			float probability  = 0.25 + 0.5 * Re;
			if (depth > 2) {
				if (rnd.next01() < probability) { // ����
					incoming_radiance = radiance(reflection_ray, rnd, depth+1) * Re;
					weight = info.pMaterial->diffuse() * (1.0 / (probability * russian_roulette_probability));
				} else { // ����
					incoming_radiance = radiance(refraction_ray, rnd, depth+1) * Tr;
					weight = info.pMaterial->diffuse() * (1.0 / (((1.0 - probability) * russian_roulette_probability)));
				}
			} else { // ���܂Ɣ��˂̗�����ǐ�
				incoming_radiance = 
					radiance(reflection_ray, rnd, depth+1) * Re +
					radiance(refraction_ray, rnd, depth+1) * Tr;
				weight = info.pMaterial->diffuse() * (1.0f / russian_roulette_probability);
			}
		}
		break;
		}

		return info.pMaterial->emission() + (weight * incoming_radiance);
	}

}
