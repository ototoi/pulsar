#include "Vector3.h"
#include "Matrix4.h"
#include "Ray.h"
#include "ppm.h"
#include "Object.h"
#include "Bound.h"
#include "Material.h"
#include "SphereObject.h"
#include "PlaneObject.h"
#include "CompositeObject.h"
#include "MaterializedObject.h"
#include "PLYMeshLoader.h"
#include "MeshObject.h"
#include "Bound.h"
#include "Camera.h"
#include "Light.h"
#include "TransformMeshLoader.h"
#include <memory>
#include <iostream>

using namespace pulsar;

static std::auto_ptr<Object> g_scene_object;
static std::auto_ptr<Light>  g_light_object;

static
float Radians( float x )
{
	static const float PI = 3.14159265358979323846f;
	return x*PI/180.0f;
}

void initScene()
{
	CompositeObject* comp = new CompositeObject();

	comp->add(new MaterializedObject(new SphereObject( Vector3( 1e5+1,  40.8, 81.6), 1e5), new LambertMaterial(Vector3(0.75, 0.25, 0.25))));//左
	comp->add(new MaterializedObject(new SphereObject( Vector3(-1e5+99, 40.8, 81.6), 1e5), new LambertMaterial(Vector3(0.25, 0.25, 0.75))));//右
	comp->add(new MaterializedObject(new SphereObject( Vector3(    50,  40.8,  1e5), 1e5), new LambertMaterial(Vector3(0.75, 0.75, 0.75))));//奥
	//Sphere(1e5, Vec(50, 40.8, -1e5+250), Color(),      Color(),                 REFLECTION_TYPE_DIFFUSE), // 手前
	comp->add(new MaterializedObject(new SphereObject( Vector3(      50, 1e5, 81.6), 1e5), new LambertMaterial(Vector3(0.75, 0.75, 0.75))));// 床
	comp->add(new MaterializedObject(new SphereObject( Vector3(50, -1e5+81.6, 81.6), 1e5), new LambertMaterial(Vector3(0.75, 0.75, 0.75))));// 天井
	comp->add(new MaterializedObject(new SphereObject( Vector3(65, 20, 20), 20), new LambertMaterial(Vector3(0.25, 0.75, 0.25))));// 緑球
	//Sphere(16.5,Vec(27, 16.5, 47),       Color(),      Color(0.99, 0.99, 0.99), REFLECTION_TYPE_SPECULAR), // 鏡
	//Sphere(16.5,Vec(77, 16.5, 78),       Color(),      Color(0.99, 0.99, 0.99), REFLECTION_TYPE_REFRACTION), //ガラス
	comp->add(new MaterializedObject(new SphereObject( Vector3(27, 16.5, 47), 16.5), new LambertMaterial(Vector3(0.99, 0.99, 0.99))));// 鏡
	comp->add(new MaterializedObject(new SphereObject( Vector3(77, 16.5, 78), 16.5), new LambertMaterial(Vector3(0.99, 0.99, 0.99))));//ガラス
	comp->add(new MaterializedObject(new SphereObject( Vector3(50.0, 90.0, 81.6), 15.0), new EmissionMaterial(Vector3(36,36,36))));// 照明

	float f = 0.75;
	Matrix4 mat = Matrix4::Traslation(50.0, 0, 20)*Matrix4::RotationY(Radians(180.0))*Matrix4::Scaling(f,f,f);
	TransformMeshLoader ml(new PLYMeshLoader("../../../../models/Armadillo.ply"), mat);
	comp->add(new MaterializedObject(new MeshObject(ml), new LambertMaterial(Vector3(1,0.1,0.1))));

	g_scene_object.reset(comp);

	//---------------------------

	g_light_object.reset(new PointLight(Vector3(1,1,1),Vector3(50.0, 90.0,81.6)));
}




void renderScene(float *img, int w, int h, int nsubsamples)
{
	Vector3 from = Vector3(50.0, 52.0, 220.0);
	Vector3 to   = from + 1000*normalize(Vector3(0.0, -0.04, -1.0));
	Vector3 upper= Vector3(0,1,0);

	PerspectiveCamera camera(from, to, upper, Radians(45), float(w)/h);

    int x, y;
    int u, v;

    float *fimg = (float *)malloc(sizeof(float) * w * h * 3);
    memset((void *)fimg, 0, sizeof(float) * w * h * 3);

	std::vector<const Light*> lights;
	lights.push_back(g_light_object.get());

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            
            for (v = 0; v < nsubsamples; v++) {
				float vv = (v+1)/float(nsubsamples+1);
                for (u = 0; u < nsubsamples; u++) {
					float uu = (u+1)/float(nsubsamples+1);
					float px01 = (x+uu)/float(w);
					float py01 = (h-(y+vv))/float(h);
                    float px = 2.0f*px01-1.0f;
                    float py = 2.0f*py01-1.0f;

					Ray ray = camera.shoot(px, py);

					Intersection info;

					if(g_scene_object->intersect(&info, ray, 0, 100000000))
					{
						info.ray_origin = ray.org();
						info.ray_direction = ray.dir();
						Vector3 col = info.pMaterial->shade(info, lights);
						fimg[3 * (y * w + x) + 0] += col[0];
                        fimg[3 * (y * w + x) + 1] += col[1];
                        fimg[3 * (y * w + x) + 2] += col[2];
					}
				}
			}

			fimg[3 * (y * w + x) + 0] /= (float)(nsubsamples * nsubsamples);
            fimg[3 * (y * w + x) + 1] /= (float)(nsubsamples * nsubsamples);
            fimg[3 * (y * w + x) + 2] /= (float)(nsubsamples * nsubsamples);
		}
	}

	memcpy(img, fimg, sizeof(float)*3*w*h);

	free(fimg);
}



int main()
{
	int width = 640;
	int height = 480;
	std::vector<float> buffer(width*height*3);
	initScene();
	renderScene(&buffer[0], width, height, 2);
	save_ppm_file("test.ppm", &buffer[0], width, height);
	
	return  0;
}