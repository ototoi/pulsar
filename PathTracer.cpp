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
	static const int Depth = 5; // ロシアンルーレットで打ち切らない最大深度
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
		// シーンと交差判定
		if (!intersect_scene(&info, ray))return BackgroundColor;
		if (!info.pMaterial)return BackgroundColor;

		Vector3 orienting_normal = info.normal;// 交差位置の法線（物体からのレイの入出を考慮）
		if(dot(orienting_normal, ray.dir())>0){
			orienting_normal = orienting_normal*-1.0f;
		}
		// 色の反射率最大のものを得る。ロシアンルーレットで使う。
		// ロシアンルーレットの閾値は任意だが色の反射率等を使うとより良い。
		Vector3 difColor = info.pMaterial->diffuse();
		double russian_roulette_probability = std::max(difColor[0], std::max(difColor[1], difColor[2]));

		// 反射回数が一定以上になったらロシアンルーレットの確率を急上昇させる。（スタックオーバーフロー対策）
		if (depth > DpethLimit){
			russian_roulette_probability *= pow(0.5, depth - DpethLimit);
		}
		// ロシアンルーレットを実行し追跡を打ち切るかどうかを判断する。
		// ただしDepth回の追跡は保障する。
		if (depth > Depth) {
			if (rnd.next01() >= russian_roulette_probability){
				return info.pMaterial->emission();
			}
		} else {
			russian_roulette_probability = 1.0; // ロシアンルーレット実行しなかった
		}

		Vector3 incoming_radiance = Vector3(0,0,0);
		Vector3 weight = Vector3(1,1,1);
	
		int nType = info.pMaterial->type();
		switch (nType)
		{
		case REFLECTION_TYPE_DIFFUSE: // 完全拡散面
		{
			// orienting_normalの方向を基準とした正規直交基底(w, u, v)を作る。この基底に対する半球内で次のレイを飛ばす。
			Vector3 w, u, v;
			w = orienting_normal;
			if (fabs(w[0]) > kEPS) // ベクトルwと直交するベクトルを作る。w.xが0に近い場合とそうでない場合とで使うベクトルを変える。
				u = normalize(cross(Vector3(0.0, 1.0, 0.0), w));
			else
				u = normalize(cross(Vector3(1.0, 0.0, 0.0), w));

			v = cross(w, u);
			// コサイン項を使った重点的サンプリング
			float r1 = 2 * kPI * rnd.next01();
			float r2 = rnd.next01(), r2s = sqrt(r2);
			Vector3 dir = normalize((
				u * cos(r1) * r2s +
				v * sin(r1) * r2s +
				w * sqrt(1.0 - r2)));

			incoming_radiance = radiance(Ray(info.position, dir), rnd, depth+1);
			// レンダリング方程式に対するモンテカルロ積分を考えると、outgoing_radiance = weight * incoming_radiance。
			// ここで、weight = (ρ/π) * cosθ / pdf(ω) / R になる。
			// ρ/πは完全拡散面のBRDFでρは反射率、cosθはレンダリング方程式におけるコサイン項、pdf(ω)はサンプリング方向についての確率密度関数。
			// Rはロシアンルーレットの確率。
			// 今、コサイン項に比例した確率密度関数によるサンプリングを行っているため、pdf(ω) = cosθ/π
			// よって、weight = ρ/ R。
			weight = info.pMaterial->diffuse() * (1.0f/russian_roulette_probability);
		} 
		break;

		// 完全鏡面
		case REFLECTION_TYPE_SPECULAR: 
		{
			// 完全鏡面なのでレイの反射方向は決定的。
			// ロシアンルーレットの確率で除算するのは上と同じ。
			incoming_radiance = radiance(Ray(info.position, ray.dir() - info.normal * 2.0 * dot(info.normal, ray.dir())), rnd, depth+1);
			weight = info.pMaterial->diffuse() * (1.0f/russian_roulette_probability);
		}
		break;

		// 屈折率kIorのガラス
		case REFLECTION_TYPE_REFRACTION:
		{
			const Ray reflection_ray = Ray(info.position, ray.dir() - info.normal * 2.0 * dot(info.normal, ray.dir()));
			const bool into = dot(info.normal, orienting_normal) > 0.0; // レイがオブジェクトから出るのか、入るのか

			// Snellの法則
			float nc = 1.0; // 真空の屈折率
			float nt = info.pMaterial->Ior(); // オブジェクトの屈折率
			float nnt = into ? nc / nt : nt / nc;
			float ddn = dot(ray.dir(), orienting_normal);
			float cos2t = 1.0 - nnt * nnt * (1.0 - ddn * ddn);
			
			if (cos2t < 0.0) { // 全反射
				incoming_radiance = radiance(reflection_ray, rnd, depth+1);
				weight = info.pMaterial->diffuse() * (1.0f / russian_roulette_probability);
				break;
			}
			// 屈折の方向
			const Ray refraction_ray = Ray(info.position,
				normalize(ray.dir() * nnt - info.normal * (into ? 1.0 : -1.0) * (ddn * nnt + sqrt(cos2t))));
			
			// SchlickによるFresnelの反射係数の近似を使う
			float a = nt - nc, b = nt + nc;
			float R0 = (a * a) / (b * b);

			float c = 1.0 - (into ? -ddn : dot(refraction_ray.dir(), -1.0 * orienting_normal));
			float Re = R0 + (1.0 - R0) * pow(c, 5.0f); // 反射方向の光が反射してray.dirの方向に運ぶ割合。同時に屈折方向の光が反射する方向に運ぶ割合。
			float nnt2 = pow(into ? nc / nt : nt / nc, 2.0f); // レイの運ぶ放射輝度は屈折率の異なる物体間を移動するとき、屈折率の比の二乗の分だけ変化する。
			float Tr = (1.0 - Re) * nnt2; // 屈折方向の光が屈折してray.dirの方向に運ぶ割合
			
			// 一定以上レイを追跡したら屈折と反射のどちらか一方を追跡する。（さもないと指数的にレイが増える）
			// ロシアンルーレットで決定する。
			float probability  = 0.25 + 0.5 * Re;
			if (depth > 2) {
				if (rnd.next01() < probability) { // 反射
					incoming_radiance = radiance(reflection_ray, rnd, depth+1) * Re;
					weight = info.pMaterial->diffuse() * (1.0 / (probability * russian_roulette_probability));
				} else { // 屈折
					incoming_radiance = radiance(refraction_ray, rnd, depth+1) * Tr;
					weight = info.pMaterial->diffuse() * (1.0 / (((1.0 - probability) * russian_roulette_probability)));
				}
			} else { // 屈折と反射の両方を追跡
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
