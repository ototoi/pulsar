#include "Vector3.h"
#include "Ray.h"
#include "ppm.h"
#include "Object.h"
#include "Bound.h"
#include "Material.h"
#include "SphereObject.h"
#include "PlaneObject.h"
#include "CompositeObject.h"
#include "PLYMeshLoader.h"
#include "MeshObject.h"
#include "Bound.h"
#include "Camera.h"
#include <memory>
#include <iostream>

using namespace pulsar;

static std::auto_ptr<Object> g_scene_object;

/*
spheres[0].center.x = -2.0;
    spheres[0].center.y =  0.0;
    spheres[0].center.z = -3.5;
    spheres[0].radius = 0.5;
    
    spheres[1].center.x = -0.5;
    spheres[1].center.y =  0.0;
    spheres[1].center.z = -3.0;
    spheres[1].radius = 0.5;
    
    spheres[2].center.x =  1.0;
    spheres[2].center.y =  0.0;
    spheres[2].center.z = -2.2;
    spheres[2].radius = 0.5;

	plane.p.x = 0.0;
    plane.p.y = -0.5;
    plane.p.z = 0.0;

    plane.n.x = 0.0;
    plane.n.y = 1.0;
    plane.n.z = 0.0;



*/
void initScene()
{
	CompositeObject* comp = new CompositeObject();
	/*
	comp->add(new SphereObject(Vector3(-2.0,0.0,-3.5), 0.5));
	comp->add(new SphereObject(Vector3(-0.5,0.0,-3.0), 0.5));
	comp->add(new SphereObject(Vector3( 1.0,0.0,-2.2), 0.5));
	comp->add(new PlaneObject (Vector3( 0.0,-0.5,0.0), Vector3( 0.0,1.0,0.0)));
	*/

	PLYMeshLoader ml("../../../../models/Armadillo.ply");
	comp->add(new MeshObject(ml));

	g_scene_object.reset(comp);
}


static
float Radians( float x )
{
	static const float PI = 3.14159265358979323846f;
	return x*PI/180.0f;
}

void renderScene(float *img, int w, int h, int nsubsamples)
{
	Bound bnd = g_scene_object->bound();
	Vector3 target = bnd.max()-bnd.min();
	std::cout << target << std::endl;

	Vector3 from = Vector3(0,0,-200);

	PerspectiveCamera camera(from, target, Vector3(0,1,0), Radians(45), float(w)/h);

    int x, y;
    int u, v;

    float *fimg = (float *)malloc(sizeof(float) * w * h * 3);
    memset((void *)fimg, 0, sizeof(float) * w * h * 3);

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            
            for (v = 0; v < nsubsamples; v++) {
                for (u = 0; u < nsubsamples; u++) {
                    float px = (x + (u / (float)nsubsamples) - (w / 2.0)) / (w / 2.0);
                    float py = -(y + (v / (float)nsubsamples) - (h / 2.0)) / (h / 2.0);

					//Vector3 dir = normalize(target + Vector3(px, py, +1));
                    //Ray ray(Vector3(0,0,0), dir);
					Ray ray = camera.shoot(px, py);

					Intersection info;

					if(g_scene_object->intersect(&info, ray, 0, 100000))
					{
						Vector3 col(1,1,1);
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
	renderScene(&buffer[0], width, height, 1);
	save_ppm_file("test.ppm", &buffer[0], width, height);
	
	return  0;
}