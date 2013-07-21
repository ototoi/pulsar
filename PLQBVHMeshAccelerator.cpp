/*
 * Partitioned Linear Bounding Volume Hierarchy construction
 */

#include "PLQBVHMeshAccelerator.h"

#include <memory>
#include <cstring>
#include <cmath>
#include <functional>
#include <algorithm>
#include <vector>
#include <utility>
#include <limits>

#include "aligned_vector.hpp"

#include <xmmintrin.h>
#include <emmintrin.h>

#include <stdlib.h>
#include <cassert>


#if defined(__unix__)||defined(__MACOS__)||defined(__APPLE__)
#include <stdint.h>
#elif defined(_MSC_VER) && _MSC_VER>=1600
#include <stdint.h>
#else
typedef int int32_t;
typedef unsigned int uint32_t; 
#endif

#define DIV_BIT 10
#define DIV_NUM 1024
#define VOX_M 6

#define MIN_FACE 4

static const int STACK_SIZE  = ((int)(sizeof(size_t)*16));


namespace pulsar{
namespace{

		typedef const Triangle* PCFACE;

		static
		inline bool testTriangle(PCFACE face, Intersection* info, const Ray& r, float tmin, float tmax)
		{
			static const float EPSILON = std::numeric_limits<float>::epsilon()*1e+3f;
			float u, v ,t;

			assert(info);	

			const Vector3& org(r.org());
			const Vector3& dir(r.dir());
			const Vector3& p0(*(face->points[0])); 
			const Vector3& p1(*(face->points[1])); 
			const Vector3& p2(*(face->points[2])); 
			
			//-e1 = p0-p1
			Vector3 e1(p0-p1);//vA
			
			//-e2 = p0-p2
			Vector3 e2(p0-p2);//vB 
			
			//dir = GHI
					
			Vector3 bDir(cross(e2,dir));
			
			float iM = dot(e1,bDir);

			if(EPSILON < iM){
				//p0-org
				Vector3 vOrg(p0 - org);

				u = dot(vOrg,bDir);
				if(u < 0 || iM < u)return false;

				Vector3 vE(cross(e1,vOrg));
		        
				v = dot(dir,vE);
				if(v < 0 || iM  < u+v)return false;

				t = -dot(e2,vE);
				if(t <= 0)return false;
			}else if(iM < -EPSILON){
				//p0-org
				Vector3 vOrg(p0 - org);//JKL

				u = dot(vOrg,bDir);
				if(u > 0 || iM > u)return false;
				
				Vector3 vE(cross(e1,vOrg));
		        
				v = dot(dir,vE);
				if(v > 0 || iM  > u+v)return false;

				t = -dot(e2,vE);
				if(t >= 0)return false;
			}else{
				return false;
			}

			iM = float(1.0)/iM;

			t *= iM;
			if ((t > tmin) && (t < tmax)) {
				u *= iM;
				v *= iM;

				
				info->t = (t);
				info->position = r.org() + t*r.dir();
				info->normal = normalize(cross(-e1,-e2));

				return true;
			}
			return false;
		}

		struct range_AABB
		{
			float tmin;
			float tmax;
		};

		bool test_AABB(range_AABB* rng, const Vector3 & min, const Vector3 & max, const Ray & r, float tmin, float tmax)
		{
			int phase = r.phase();
			int sign[3] = {(phase>>0)&1,(phase>>1)&1,(phase>>2)&1};
			Vector3 box[2] = {min,max};
			const Vector3& org  = r.org();
			const Vector3& idir = r.idir();

	        
			for(int i = 0;i<3;i++){
				tmin = std::max<float>(tmin,(box[  sign[i]][i]-org[i])*idir[i]);
				tmax = std::min<float>(tmax,(box[1-sign[i]][i]-org[i])*idir[i]);
			}
			
			if(tmin<=tmax)
			{
				rng->tmin = tmin;
				rng->tmax = tmax;
				return true;
			}
			return false;
		}

	    //For float
        static 
        inline int test_AABB(
                             const __m128 bboxes[2][3],  //4boxes : min-max[2] of xyz[3] of boxes[4]
                             const __m128 org[3],        //Ray origin
                             const __m128 idir[3],       //Ray inveresed direction
                             const int sign[3],          //Ray xyz direction -> +:0,-:1
                             __m128 tmin, __m128 tmax    //Ray range tmin-tmax 
                             )
        {
            // x coordinate
            tmin = _mm_max_ps(
                              tmin,
                              _mm_mul_ps(_mm_sub_ps(bboxes[sign[0]][0],org[0]), idir[0])
                              );
            tmax = _mm_min_ps(
                              tmax,
                              _mm_mul_ps(_mm_sub_ps(bboxes[1 - sign[0]][0], org[0]), idir[0])
                              );
            
            // y coordinate
            tmin = _mm_max_ps(
                              tmin,
                              _mm_mul_ps(_mm_sub_ps(bboxes[sign[1]][1],org[1]), idir[1])
                              );
            tmax = _mm_min_ps(
                              tmax,
                              _mm_mul_ps(_mm_sub_ps(bboxes[1 - sign[1]][1], org[1]), idir[1])
                              );
            
            // z coordinate
            tmin = _mm_max_ps(
                              tmin,
                              _mm_mul_ps(_mm_sub_ps(bboxes[sign[2]][2],org[2]), idir[2])
                              );
            tmax = _mm_min_ps(
                              tmax,
                              _mm_mul_ps(_mm_sub_ps(bboxes[1 - sign[2]][2], org[2]), idir[2])
                              );
            
            return _mm_movemask_ps(_mm_cmpge_ps(tmax, tmin));
        }
        
        static const size_t EMPTY_MASK  = size_t(-1);       //0xFFFF...
        static const size_t SIGN_MASK   = ~(EMPTY_MASK>>1); //0x8000...

		inline bool IsLeaf(size_t i){
            return (SIGN_MASK&i)?true:false;
        }
        inline bool IsBranch(size_t i){
            return !IsLeaf(i);
        }    
        inline bool IsEmpty(size_t i){
            return i==EMPTY_MASK;
        }
        
        inline size_t MakeLeafIndex(size_t i)
        {
            return SIGN_MASK | i;
        }
        
        inline size_t GetFaceFirst(size_t i) {
            return (~SIGN_MASK) & i;
        }
        
#pragma pack(push,4)
        struct SIMDBVHNode{
            __m128 bboxes[2][3];//768
            size_t children[4]; //128
            int axis_top;       //128
            int axis_right;
            int axis_left;
            int reserved;
        };
#pragma pack(pop) 
        
        void get_minmax(Vector3& pmin, Vector3& pmax, PCFACE face){
            static const float FAR = std::numeric_limits<float>::max()*1e-3f;

            Vector3 min = Vector3(+FAR, +FAR, +FAR);
            Vector3 max = Vector3(-FAR, -FAR, -FAR);
            
            Vector3 points[3];
			points[0] = *(face->points[0]);
            points[1] = *(face->points[1]);
            points[2] = *(face->points[2]);
            
            for(int i = 0;i<3;i++){
                for(int j = 0;j<3;j++){
                    if(points[i][j]<min[j])min[j]=points[i][j];
                    if(points[i][j]>max[j])max[j]=points[i][j];                             
                }
            }
            
            pmin = min;
            pmax = max;
		}
        
        template<class ITER>
        void get_minmax(Vector3& pmin, Vector3& pmax, ITER a, ITER b){
            static const float FAR = std::numeric_limits<float>::max()*1e-3f;
            Vector3 min = Vector3(+FAR, +FAR, +FAR);
            Vector3 max = Vector3(-FAR, -FAR, -FAR);
            for(ITER i = a;i!=b;i++){
                Vector3 cmin;
                Vector3 cmax;
                get_minmax(cmin,cmax,*i);
                for(int j=0;j<3;j++){
                    if(cmin[j]<min[j])min[j]=cmin[j];
                    if(cmax[j]>max[j])max[j]=cmax[j];
                }
            }
            pmin = min;
            pmax = max;
        }
        
        static
        void get_minmax(Vector3& pmin, Vector3& pmax, const PCFACE* faces, const size_t* indices, size_t a, size_t b)
        {
            static const float FAR = std::numeric_limits<float>::max()*1e-3f;
            Vector3 min = Vector3(+FAR, +FAR, +FAR);
            Vector3 max = Vector3(-FAR, -FAR, -FAR);
            for(size_t i = a;i!=b;i++){
                size_t idx = indices[i];
                PCFACE face = faces[idx];
                Vector3 cmin;
                Vector3 cmax;
                get_minmax(cmin,cmax,face);
                for(int j=0;j<3;j++){
                    if(cmin[j]<min[j])min[j]=cmin[j];
                    if(cmax[j]>max[j])max[j]=cmax[j];
                }
            }
            pmin = min;
            pmax = max;
        }

		void get_morton_bound(Vector3& tmin, Vector3& tmax, const Vector3& min,  const Vector3& max)
		{
			Vector3 center = (min+max)*0.5;
			Vector3 extend = (max-min)*0.5;

			float wid = extend[0];
			if(wid<extend[1])wid=extend[1];
			if(wid<extend[2])wid=extend[2];
			wid += +std::numeric_limits<float>::epsilon()*1e+3f;
			tmin = center - Vector3(wid,wid,wid);
			tmax = center + Vector3(wid,wid,wid);
		}
        
        static
        inline uint32_t partby2(uint32_t n)
        {
            n = (n ^ (n<<16)) & 0xff0000ff;
            n = (n ^ (n<< 8)) & 0x0300f00f;
            n = (n ^ (n<< 4)) & 0x030c30c3;
            n = (n ^ (n<< 2)) & 0x09249249;
            return n;
        }
        
        static 
        inline uint32_t get_morton_code(uint32_t x, uint32_t y, uint32_t z)
        {
            return (partby2(x)<<2) | (partby2(y)<<1) | (partby2(z));
        }
        
        static
        inline uint32_t get_morton_code(const Vector3& p, const Vector3& min, const Vector3& max){
            uint32_t ix = (uint32_t)(DIV_NUM*((p[0]-min[0])/(max[0]-min[0])));
            uint32_t iy = (uint32_t)(DIV_NUM*((p[1]-min[1])/(max[1]-min[1])));
            uint32_t iz = (uint32_t)(DIV_NUM*((p[2]-min[2])/(max[2]-min[2])));
            return get_morton_code(ix,iy,iz);
        }

        
        static 
        inline Vector3 get_center(PCFACE face)
        {
            return (*(face->points[0]) + *(face->points[1]) + *(face->points[2]))*(float(1)/3);
        }
		
		static
		void create_morton_code(std::vector<uint32_t>& codes, const PCFACE* faces, size_t fsz, const Vector3 min, const Vector3 max)
		{
			for(size_t i = 0;i<fsz;i++)
			{
				codes[i] = get_morton_code(get_center(faces[i]), min, max);
			}
		}
        
        struct separator{
            separator(int level, const uint32_t* codes):codes_(codes)
            {
                int p = 3*DIV_BIT-1-level;
                nMask_ = 1<<p;
            }
            inline bool operator()(size_t i)const
            {
                return (nMask_ & codes_[i]) == 0;
            }
            
            uint32_t nMask_;
            const uint32_t* codes_;
        };

		static
		inline size_t get_branch_node_size(size_t face_num){
			if(face_num<=MIN_FACE)return 1;
			size_t p = face_num/4;
			if(face_num&3)p++;
			return std::max<size_t>(1,1+4*get_branch_node_size(p));
		}
		static
		inline size_t get_leaf_node_size(size_t face_num){
			return std::max<size_t>(MIN_FACE,face_num+(int)ceil(double(face_num)/(MIN_FACE)));
		}

		static
		size_t construct_deep(
			  std::vector<PCFACE>& out_faces,
			  aligned_vector<SIMDBVHNode>& out_nodes,
			  Vector3& min, Vector3& max,
			  const PCFACE* faces,
			  const uint32_t* codes,
			  size_t* indices,
			  size_t a,
			  size_t b,
			  int level
		){
			static const float EPS = std::numeric_limits<float>::epsilon();
			static const float FAR = std::numeric_limits<float>::max();
	        
			size_t sz = b-a;
			if(sz==0)return EMPTY_MASK;
			if(sz<=MIN_FACE  || level >= DIV_BIT*3){
				size_t first = out_faces.size();
				size_t last  = first+sz+1;//zero terminate
				out_faces.resize(last);
				for(size_t i=0;i<sz;i++){
					out_faces[first+i] = faces[indices[a+i]];
				}
				out_faces[last-1]=0;                
				get_minmax(min,max, faces, indices, a, b);

				min -= Vector3(EPS,EPS,EPS);
				max += Vector3(EPS,EPS,EPS);
	            
				size_t nRet = MakeLeafIndex(first);
				assert(IsLeaf(nRet));
				return nRet;
			}else{
				size_t* ia = indices+a;
				size_t* ib = indices+b;
				size_t* ic;
				size_t* il;
				size_t* ir;
				{
					ic = std::partition(ia, ib, separator(level, codes));
					size_t* iters[2];
					size_t* idx[2][2] = {{ia,ic},{ic,ib}};
					for(int i = 0;i< 2;i++){
						iters[i] = std::partition(idx[i][0], idx[i][1], separator(level+1, codes));
					}
					il = iters[0];
					ir = iters[1];                
				}
	            
				size_t csz = ic-ia;
				size_t lsz = il-ia;
				size_t rsz = ir-ic;
	            
				size_t offset = out_nodes.size();
	            
				size_t c = a+csz;
				size_t l = a+lsz;
				size_t r = c+rsz;
	            
	            
				SIMDBVHNode node;
				node.axis_top   = level%3;
				node.axis_left = node.axis_right = (level+1)%3;
				out_nodes.push_back(node);
	            
				Vector3 minmax[4][2];
				for(int i = 0;i<4;i++){
					minmax[i][0] = Vector3(+FAR,+FAR,+FAR);
					minmax[i][1] = Vector3(-FAR,-FAR,-FAR);
				}
	            
				size_t ranges[4][2];
				ranges[0][0] = a;
				ranges[0][1] = l;
				ranges[1][0] = l;
				ranges[1][1] = c;
				ranges[2][0] = c;
				ranges[2][1] = r;
				ranges[3][0] = r;
				ranges[3][1] = b;
	            
				for(int i=0;i<4;i++){
					volatile size_t index = construct_deep(out_faces, out_nodes, minmax[i][0], minmax[i][1], faces, codes, indices, ranges[i][0], ranges[i][1], level+2);
					out_nodes[offset].children[i] = index;
				}
	            
				//convert & swizzle
				float bboxes[2][3][4];
				//for(int m = 0;m<2;m++){//minmax
				for(int j = 0;j<3;j++){//xyz
					for(int k = 0;k<4;k++){//box
						bboxes[0][j][k] = minmax[k][0][j];//
						bboxes[1][j][k] = minmax[k][1][j];//
					}
				}
				//}
	            
				//for(int i = 0;i<4;i++){
				for(int m = 0;m<2;m++){//minmax
					for(int j = 0;j<3;j++){//xyz
						out_nodes[offset].bboxes[m][j] = _mm_setzero_ps();
						out_nodes[offset].bboxes[m][j] = _mm_loadu_ps(bboxes[m][j]);
					}
				}
				//}
	            
				min = minmax[0][0];
				max = minmax[0][1];
				for(int i = 1;i<4;i++){
					for(int j = 0;j<3;j++){//xyz
						if(min[j]>minmax[i][0][j])min[j] = minmax[i][0][j];
						if(max[j]<minmax[i][1][j])max[j] = minmax[i][1][j];                   
					}
				}

				min -= Vector3(EPS,EPS,EPS);
				max += Vector3(EPS,EPS,EPS);
	            
				return offset;
			}
		}


		static
		void construct(
			std::vector<PCFACE>& out_faces,
			aligned_vector<SIMDBVHNode>& out_nodes,
			const PCFACE* faces,
			const uint32_t* codes,
			size_t* indices,
			size_t a,
			size_t b
		){
			static const float FAR = std::numeric_limits<float>::max();
			static const int level = 0;
	        
			size_t sz = b-a;
			if(sz==0)return;
			size_t* ia = indices+a;
			size_t* ib = indices+b;
			size_t* ic = NULL;;
			size_t* il = NULL;
			size_t* ir = NULL;
			{
				ic = std::partition(ia, ib, separator(level, codes));
				size_t* iters[2] = {0,0};
				size_t* idx[2][2] = {};
				idx[0][0] = ia;
				idx[0][1] = ic;
				idx[1][0] = ic;
				idx[1][1] = ib;
				//{{ia,ic},{ic,ib}};
				{
					//#pragma omp parallel for
					for(int i = 0;i< 2;i++){
						iters[i] = std::partition(idx[i][0], idx[i][1], separator(level+1, codes));
					}
				}
				il = iters[0];
				ir = iters[1];            
			}
	        
			size_t csz = ic-ia;
			size_t lsz = il-ia;
			size_t rsz = ir-ic;
	        
			size_t offset = out_nodes.size();
	        
			size_t c = a+csz;
			size_t l = a+lsz;
			size_t r = c+rsz;
	        
	        
			SIMDBVHNode node;
			node.axis_top   = level&3;
			node.axis_left = node.axis_right = (level+1)%3;
			out_nodes.push_back(node);
	        
			Vector3 minmax[4][2];
			for(int i = 0;i<4;i++){
				minmax[i][0] = Vector3(+FAR,+FAR,+FAR);
				minmax[i][1] = Vector3(-FAR,-FAR,-FAR);
			}
	        
			size_t ranges[4][2];
			ranges[0][0] = a;
			ranges[0][1] = l;
			ranges[1][0] = l;
			ranges[1][1] = c;
			ranges[2][0] = c;
			ranges[2][1] = r;
			ranges[3][0] = r;
			ranges[3][1] = b;
	        
			{
				size_t sub_indices[4];
				std::vector<PCFACE> sub_faces[4];
				aligned_vector<SIMDBVHNode> sub_nodes[4];
				for(int i = 0;i<4;i++){
					size_t fsz = ranges[i][1]-ranges[i][0];
					sub_faces[i].reserve(get_leaf_node_size(fsz));
					sub_nodes[i].reserve(get_branch_node_size(fsz));
				}
	            
				{
            		#pragma omp parallel for
            		for(int i=0;i<4;i++){
                		sub_indices[i] = construct_deep(sub_faces[i], sub_nodes[i], minmax[i][0], minmax[i][1], faces, codes, indices, ranges[i][0], ranges[i][1], level+2);
					}
				}
	            
				//PRINTLOG("plqbvh_mesh_accelerator:construct deep.\n");
	            
				size_t face_offsets[4+1];
				size_t node_offsets[4+1];
				face_offsets[0]=0;
				node_offsets[0]=1;
				for(int i=0;i<4;i++){
					face_offsets[i+1] = sub_faces[i].size();
					node_offsets[i+1] = sub_nodes[i].size();
				}
				for(int i=0;i<4;i++){
					face_offsets[i+1] += face_offsets[i];
					node_offsets[i+1] += node_offsets[i];
				}
	            
				for(int k=0;k<4;k++){
					size_t idx = sub_indices[k];
					if(IsBranch(idx)){//
						idx += node_offsets[k];
						sub_indices[k] = idx;
					}else if(!IsEmpty(idx)){//
						idx = GetFaceFirst(idx);
						idx += face_offsets[k];
						sub_indices[k] = MakeLeafIndex(idx);
					}
					out_nodes[offset].children[k] = sub_indices[k];
				}      
	            
				out_faces.resize(face_offsets[4]);
				out_nodes.resize(node_offsets[4]);
	            
				{
					#pragma omp parallel for
					for(int i=0;i<4;i++){
						aligned_vector<SIMDBVHNode>& nodes = sub_nodes[i];
						size_t jsz = nodes.size();
						for(size_t j = 0;j<jsz;j++){
							SIMDBVHNode& nd = nodes[j];
							for(int k=0;k<4;k++){
								size_t idx = nd.children[k];
								if(IsBranch(idx)){//
									idx += node_offsets[i];
									nd.children[k] = idx;
								}else if(!IsEmpty(idx)){//
									idx = GetFaceFirst(idx);
									idx += face_offsets[i];
									nd.children[k] = MakeLeafIndex(idx);
								}
							}                    
						}
	            
						if(!sub_faces[i].empty()){memcpy(&out_faces[face_offsets[i]], &(sub_faces[i][0]), sizeof(PCFACE)*sub_faces[i].size());}
						if(!sub_nodes[i].empty()){memcpy(&out_nodes[node_offsets[i]], &(sub_nodes[i][0]), sizeof(SIMDBVHNode)*sub_nodes[i].size());}
					}
				}
			}
	        
			//convert & swizzle
			float bboxes[2][3][4];
			//for(int m = 0;m<2;m++){//minmax
			for(int j = 0;j<3;j++){//xyz
				for(int k = 0;k<4;k++){//box
					bboxes[0][j][k] = minmax[k][0][j];//
					bboxes[1][j][k] = minmax[k][1][j];//
				}
			}
			//}
	        
			//for(int i = 0;i<4;i++){
			for(int m = 0;m<2;m++){//minmax
				for(int j = 0;j<3;j++){//xyz
					out_nodes[offset].bboxes[m][j] = _mm_setzero_ps();
					out_nodes[offset].bboxes[m][j] = _mm_loadu_ps(bboxes[m][j]);
				}
			}
			//}
		}


}


	class PLQBVHMeshAcceleratorImp
	{
	public:
		PLQBVHMeshAcceleratorImp(std::vector<const Triangle*>& tris);
		virtual bool intersect(Intersection* info, const Ray& r, float tmin, float tmax)const;
		virtual Bound bound()const;
	protected:
		bool intersect_inner              (Intersection* info, const Ray& r, float tmin, float tmax)const;
		bool intersect_faces(size_t index, Intersection* info, const Ray& r, float tmin, float tmax)const;
	protected:
		Vector3 min_;
		Vector3 max_;
		std::vector<PCFACE>         faces_;
        aligned_vector<SIMDBVHNode> nodes_;
	};

	PLQBVHMeshAcceleratorImp::PLQBVHMeshAcceleratorImp(std::vector<const Triangle*>& tris)
	{
		size_t sz = tris.size();
		{
			get_minmax(min_, max_, tris.begin(), tris.end());
		}

		PCFACE* begin = &tris[0];

        std::vector<uint32_t> codes(sz);
        {
            Vector3 tmin,tmax;
            get_morton_bound(tmin, tmax, min_, max_);
            create_morton_code(codes, begin, sz, tmin, tmax); 
        }
        
        std::vector<size_t> indices(sz);
        for(size_t i=0;i<sz;i++){
            indices[i]=i;
        }
        //std::random_shuffle(indices.begin(), indices.end());
        construct(faces_, nodes_, begin, &codes[0], &indices[0], 0,sz);
	}

	bool PLQBVHMeshAcceleratorImp::intersect(Intersection* info, const Ray& r, float tmin, float tmax)const
	{
		range_AABB rng;
        if(test_AABB(&rng, min_, max_, r, tmin, tmax)){
            tmin = std::max<float>(tmin, rng.tmin);
            tmax = std::min<float>(tmax, rng.tmax);
            return intersect_inner(info, r, tmin, tmax);
        }else{
            return false;
        }
	}

	bool PLQBVHMeshAcceleratorImp::intersect_faces(size_t index, Intersection* info, const Ray& r, float tmin, float tmax)const
	{
		const PCFACE* faces = &faces_[0];
        bool bRet = false;
        size_t i = index;
        while(faces[i]){
            if(testTriangle(faces[i], info, r, tmin, tmax)){
                tmax = info->t;
                bRet = true;
            }
            i++;
        }
		return bRet;
	}
	//visit order table 
    //8*16 = 128
    static const int OrderTable[] = {
        0x44444,0x44444,0x44444,0x44444,0x44444,0x44444,0x44444,0x44444,
        0x44440,0x44440,0x44440,0x44440,0x44440,0x44440,0x44440,0x44440,
        0x44441,0x44441,0x44441,0x44441,0x44441,0x44441,0x44441,0x44441,
        0x44401,0x44401,0x44410,0x44410,0x44401,0x44401,0x44410,0x44410,
        0x44442,0x44442,0x44442,0x44442,0x44442,0x44442,0x44442,0x44442,
        0x44402,0x44402,0x44402,0x44402,0x44420,0x44420,0x44420,0x44420,
        0x44412,0x44412,0x44412,0x44412,0x44421,0x44421,0x44421,0x44421,
        0x44012,0x44012,0x44102,0x44102,0x44201,0x44201,0x44210,0x44210,
        0x44443,0x44443,0x44443,0x44443,0x44443,0x44443,0x44443,0x44443,
        0x44403,0x44403,0x44403,0x44403,0x44430,0x44430,0x44430,0x44430,
        0x44413,0x44413,0x44413,0x44413,0x44431,0x44431,0x44431,0x44431,
        0x44013,0x44013,0x44103,0x44103,0x44301,0x44301,0x44310,0x44310,
        0x44423,0x44432,0x44423,0x44432,0x44423,0x44432,0x44423,0x44432,
        0x44023,0x44032,0x44023,0x44032,0x44230,0x44320,0x44230,0x44320,
        0x44123,0x44132,0x44123,0x44132,0x44231,0x44321,0x44231,0x44321,
        0x40123,0x40132,0x41023,0x41032,0x42301,0x43201,0x42310,0x43210,
    };

	bool PLQBVHMeshAcceleratorImp::intersect_inner(Intersection* info, const Ray& r, float tmin, float tmax)const
	{
		if(nodes_.empty())return false;
        
        __m128 sseOrg[3];
        __m128 sseiDir[3];
        int sign[3];
        __m128 sseTMin;
        __m128 sseTMax;
        
        sseOrg[0] = _mm_set1_ps(r.org()[0]);
        sseOrg[1] = _mm_set1_ps(r.org()[1]);
        sseOrg[2] = _mm_set1_ps(r.org()[2]);
        
        sseiDir[0] = _mm_set1_ps(r.idir()[0]);
        sseiDir[1] = _mm_set1_ps(r.idir()[1]);
        sseiDir[2] = _mm_set1_ps(r.idir()[2]);
        
        int phase = r.phase();
        sign[0] = (phase>>0)&1;
        sign[1] = (phase>>1)&1;
        sign[2] = (phase>>2)&1;
        
        sseTMin = _mm_set1_ps(tmin);
        sseTMax = _mm_set1_ps(tmax);
        
        const SIMDBVHNode* nodes = &nodes_[0];
        
        int todoNode = 0;
        size_t nodeStack[STACK_SIZE];
        nodeStack[0] = 0;
        
        bool bRet = false;
        while (todoNode >= 0) {
            size_t idx = nodeStack[todoNode];
            
            if(IsBranch(idx)){
                todoNode--;//pop stack
                const SIMDBVHNode& node = nodes[idx];
                
                int HitMask = test_AABB(node.bboxes, sseOrg, sseiDir, sign, sseTMin, sseTMax);
                
                if(HitMask){
                    int NodeIdx = (sign[node.axis_top] << 2) | (sign[node.axis_left] << 1) | (sign[node.axis_right]);
                    int Order   = OrderTable[HitMask*8+NodeIdx];
                    
                    while(!(Order&0x4)){
                        todoNode++;
                        nodeStack[todoNode] = node.children[Order & 0x3];
                        Order>>=4;
                    }
                }
                
            }else{// if(IsLeaf(idx))
                todoNode--;
                if(IsEmpty(idx))continue;
                
                size_t fi = GetFaceFirst(idx);//convert face index.
                
                if(this->intersect_faces(fi, info, r, tmin, tmax)){
                    bRet = true;
                    tmax = info->t;
                    sseTMax = _mm_set1_ps(tmax);
                }
            }
            
            assert(todoNode<STACK_SIZE);
        }
        
        return bRet;
	}

	Bound PLQBVHMeshAcceleratorImp::bound()const
	{
		return Bound(min_, max_);
	}

	//---------------------------------------------------------------------------------------
	PLQBVHMeshAccelerator::PLQBVHMeshAccelerator(std::vector<const Triangle*>& tris)
	{
		imp_ = new PLQBVHMeshAcceleratorImp(tris);	
	}

	PLQBVHMeshAccelerator::~PLQBVHMeshAccelerator()
	{
		delete imp_;
	}

	bool PLQBVHMeshAccelerator::intersect(Intersection* info, const Ray& r, float tmin, float tmax)const
	{
		return imp_->intersect(info, r, tmin, tmax);
	}

	Bound PLQBVHMeshAccelerator::bound()const
	{
		return imp_->bound();
	}
	
}
