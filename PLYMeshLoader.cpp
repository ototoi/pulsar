#include "PLYMeshLoader.h"
#include <vector>
#include <cassert>
#include "rply/rply.h"


namespace pulsar
{
namespace 
{
	static
	std::string get_full_path(const std::string& path)
	{
#if defined(_WIN32)
		char szFullPath[_MAX_PATH];
		_fullpath(szFullPath, path.c_str(), _MAX_PATH);
		return szFullPath;
#else
		char szFullPath[1024];
		realpath(path.c_str(), szFullPath);
		return szFullPath;
#endif
	}

	struct ply_v_input{
		std::vector<Vector3>* ms;

		float vtx[3];

		unsigned int idx;
		int cnt;
		float isz;
	};
	
	struct ply_f_input{
		std::vector<Triangle>* ms;

		unsigned long fce[3];

		unsigned int idx;
		int cnt;
		float isz;
	};

extern "C" {
	static int vertex_cb(p_ply_argument argument) {
	    int32_t plane;
		
		ply_v_input *input;
		unsigned int idx;
	    std::vector<Vector3>* ms;
		
		ply_get_argument_user_data(argument, (void**)&input, &plane);
		idx = input->idx;
		ms  = input->ms;
		
		if(plane >=3) return 0;
		
		if (plane == 2){
			input->vtx[2] = (float)ply_get_argument_value(argument);

			Vector3 vv;
			vv[0] = input->vtx[0];
			vv[1] = input->vtx[1];
			vv[2] = input->vtx[2];
			(*ms)[idx] = vv;
			
			input->idx = idx+1;
		}else{
			input->vtx[plane] = (float)ply_get_argument_value(argument);
		}
		
	    return 1;
	}
	
	static int face_cb(p_ply_argument argument) {
	    int32_t length, plane;
		ply_f_input *input;
		unsigned int idx;
		std::vector<Triangle> * ms;
		
		ply_get_argument_user_data(argument, (void**)&input, NULL);
		idx = input->idx;
		ms  = input->ms;
		
	    ply_get_argument_property(argument, NULL, &length, &plane);
		
		if(plane >=3) return 0;

		Triangle ff;

		switch(plane){
		case 0:
			input->fce[0] = (unsigned long)ply_get_argument_value(argument);break;
		case 1:
			input->fce[1] = (unsigned long)ply_get_argument_value(argument);break;
		case 2:
			input->fce[2] = (unsigned long)ply_get_argument_value(argument);

			ff.indices[0] = input->fce[0];
			ff.indices[1] = input->fce[1];
			ff.indices[2] = input->fce[2];
			(*ms)[idx] = ff;
	
			input->idx = idx+1;
		default:
			break;
		}
	    
	    return 1;
	}

	static int32_t ply_get_element_size(p_ply ply, const char *name) {
		p_ply_element element;
		int32_t size;
		const char* p_name;

		assert(ply && name); 
		
		element = 0;//
		while( (element = ply_get_next_element(ply,element)) ){//get next element!
			ply_get_element_info(element,&p_name,&size);
			if (!std::strcmp(p_name, name)) return size;
		}

		return 0;
	}
	    
	static
    void error_cb(const char *message) {
        printf(message);
    }
}

	class ply_mesh_loader_imp{
	public:
		ply_mesh_loader_imp(const char * name){
			ply_ = ply_open(name, error_cb);
			if(!ply_){
				std::string s = get_full_path(name);
				return;
			}
			if (!ply_read_header(ply_)){
				ply_close(ply_);
			}
		  
			unsigned long nsz = 0;
			
			if(!(nsz = ply_get_element_size(ply_, "face"))){
				ply_close(ply_);
				ply_ = 0;
				return;
			}
			nfaces_    = nsz;

			//nfaces_    = ply_set_read_cb(ply_, "face", "vertex_indices", NULL, NULL, 0);
			if(!(nsz = ply_get_element_size(ply_, "vertex"))){
				ply_close(ply_);
				ply_ = 0;
				return;
			}
			nvertices_ = nsz;
			//nvertices_ = ply_set_read_cb(ply_, "vertex", "x", NULL, NULL, 0);
		}
		
		~ply_mesh_loader_imp(){
			ply_close(ply_);
		}
		
		size_t get_f()const{return nfaces_;}
		size_t get_v()const{return nvertices_;}
		
		bool load(std::vector<Vector3>& verts, std::vector<Triangle>& tris)const{
			if(!ply_)return false;
			size_t fsize = this->get_f();
			size_t vsize = this->get_v();
			

			verts.resize(vsize);
			tris.resize(fsize);

			ply_v_input vinput;
			ply_f_input finput;
			
			
			vinput.ms = &verts;
			vinput.idx = 0;
			vinput.cnt = 0;
			vinput.isz = 100.0f/vsize;
			ply_set_read_cb(ply_, "vertex", "x", vertex_cb, &vinput, 0);
			ply_set_read_cb(ply_, "vertex", "y", vertex_cb, &vinput, 1);
			ply_set_read_cb(ply_, "vertex", "z", vertex_cb, &vinput, 2);
			
		
			finput.ms = &tris;
			finput.idx = 0;
			finput.cnt = 0;
			finput.isz = 100.0f/fsize;
			ply_set_read_cb(ply_, "face", "vertex_indices", face_cb, &finput, 0);
			
			if (!ply_read(ply_)) return false;
			return true;
		}
		
	private:
		p_ply ply_;
		size_t nfaces_;
		size_t nvertices_;
	};

}

	
	PLYMeshLoader::PLYMeshLoader(const char* szFileName)
		:strFileName_(szFileName)
	{
		;
	}

	PLYMeshLoader::~PLYMeshLoader()
	{
		;
	}

	bool PLYMeshLoader::load(std::vector<Vector3>& verts, std::vector<Triangle>& tris)const
	{
		ply_mesh_loader_imp imp(strFileName_.c_str());

		return imp.load(verts, tris);
	}

}