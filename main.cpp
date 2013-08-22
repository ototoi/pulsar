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
#include "PathTracer.h"
#include "Timer.h"

#include <memory>
#include <iostream>
#include <stdio.h>
#include <string.h>

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
	comp->add(new MaterializedObject(new SphereObject( Vector3( 50, 40.8, -1e5+250), 1e5), new LambertMaterial(Vector3(0.0, 0.0, 0.0))));//奥
	
	//Sphere(1e5, Vec(50, 40.8, -1e5+250), Color(),      Color(),                 REFLECTION_TYPE_DIFFUSE), // 手前
	comp->add(new MaterializedObject(new SphereObject( Vector3(      50, 1e5, 81.6), 1e5), new LambertMaterial(Vector3(0.75, 0.75, 0.75))));// 床
	comp->add(new MaterializedObject(new SphereObject( Vector3(50, -1e5+81.6, 81.6), 1e5), new LambertMaterial(Vector3(0.75, 0.75, 0.75))));// 天井
	//comp->add(new MaterializedObject(new SphereObject( Vector3(65, 20, 20), 20), new LambertMaterial(Vector3(0.25, 0.75, 0.25))));// 緑球
	comp->add(new MaterializedObject(new SphereObject( Vector3(27, 16.5, 47), 16.5), new MirrorMaterial(Vector3(0.99, 0.99, 0.99))));// 鏡
	comp->add(new MaterializedObject(new SphereObject( Vector3(77, 16.5, 78), 16.5), new GlassMaterial(Vector3(0.99, 0.99, 0.99))));//ガラス
	comp->add(new MaterializedObject(new SphereObject( Vector3(50.0, 90.0, 81.6), 15.0), new EmissionMaterial(Vector3(36,36,36))));// 照明

	float f = 0.75;
	Matrix4 mat = Matrix4::Traslation(50.0, 0, 20)*Matrix4::RotationY(Radians(180.0))*Matrix4::Scaling(f,f,f);
	TransformMeshLoader ml(new PLYMeshLoader("../../../../models/Armadillo.ply"), mat);
	comp->add(new MaterializedObject(new MeshObject(ml), new LambertMaterial(Vector3(0.1,0.7,0.1))));

	g_scene_object.reset(comp);

	//---------------------------

	g_light_object.reset(new PointLight(Vector3(1,1,1),Vector3(50.0, 90.0,81.6)));
}

static
void saveImagefile(const char* szFile, int nFrame, const float buffer[], int width, int height)
{
	char sname[256];
	sprintf(sname, "%s_%04d.ppm", szFile,nFrame);
	save_ppm_file(sname, buffer, width, height);
}

static
void averageImagefile(float oo[], const float aa[], const float bb[], float alpha, int width, int height)
{
	int total = width*height*3;
	for(int i=0;i<total;i++)
	{
		oo[i] = (alpha*aa[i]+(1.0f-alpha)*bb[i]);
	}
}



void renderScene(const char* szFilename, int w, int h, int nsubsamples, int iter)
{
	Vector3 from = Vector3(50.0, 52.0, 220.0);
	Vector3 to   = from + 1000*normalize(Vector3(0.0, -0.04, -1.0));
	Vector3 upper= Vector3(0,1,0);

	PerspectiveCamera camera(from, to, upper, Radians(45), float(w)/h);

	PathTracer pt(g_scene_object.get());
	Random rnd(123456);

    int x, y;
    int u, v;

	std::vector<float> prevImage(w*h*3);
	
	//std::vector<const Light*> lights;
	//lights.push_back(g_light_object.get());

	int total_pixel = w*h;

	Timer t;
	t.start();

	Timer tx;
	tx.start();

	int num_P =1;
	for(int nFrame = 0;nFrame<iter;nFrame++){
		std::vector<float> crntImage(w*h*3);
		memset(&crntImage[0], 0 , sizeof(float)*w*h*3);

		int prog = 0;
		//#pragma omp parallel for
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

						{
							Vector3 col = pt.radiance(ray, rnd, 0);
							crntImage[3 * (y * w + x) + 0] += col[0];
							crntImage[3 * (y * w + x) + 1] += col[1];
							crntImage[3 * (y * w + x) + 2] += col[2];
						}
					}
				}

				crntImage[3 * (y * w + x) + 0] /= (float)(nsubsamples * nsubsamples);
				crntImage[3 * (y * w + x) + 1] /= (float)(nsubsamples * nsubsamples);
				crntImage[3 * (y * w + x) + 2] /= (float)(nsubsamples * nsubsamples);

				prog++;
			}
			printf("%d:line:%d/%d\n",nFrame+1, y,h);
			
		}

		//printf("%d:line:%d/%d\n",nFrame+1, y,h);


		std::vector<float> outImage(w*h*3);
		if(nFrame==0)
		{
			outImage = crntImage;
			saveImagefile(szFilename, nFrame+1, &outImage[0], w, h);
		}else{
			float alpha = float(nFrame)/(nFrame+1);//1:0.5, 2
			averageImagefile(&outImage[0], &prevImage[0], &crntImage[0], alpha, w, h);
			saveImagefile(szFilename, nFrame+1, &outImage[0], w, h);
			prevImage.swap(outImage);
		}

		{
			tx.end();
			Timer::time_t tt = tx.sec();
			if(tt>60){
				{
					std::string str = szFilename;
					str += "_at_";
					saveImagefile(str.c_str(), num_P, &outImage[0], w, h);
				}

				num_P++;
				tx.start();
			}else{
				;//
			}
		}

		t.end();
		Timer::time_t tt = t.sec();
		if(tt>60*60){
			break;
		}
	}
}



int main(int argc, char** argv)
{
	int width = 640;
	int height = 480;
	int samples = 2;
	int iter = 1000;

	std::string strFilename = "result";

	if(argc < 2){
		printf("pulsar [filename] [width] [height] [samples] [iteration]\n");
	}

	if(argc>=2)
	{
		strFilename = argv[1];
	}

	if(argc>=3)
	{
		width = atoi(argv[2]);
	}

	if(argc>=4)
	{
		height = atoi(argv[3]);
	}

	if(argc>=5)
	{
		samples = atoi(argv[4]);
	}

	if(argc>=6)
	{
		iter = atoi(argv[5]);
	}



	initScene();
	renderScene(strFilename.c_str(), width, height, samples, iter);
	
	return  0;
}