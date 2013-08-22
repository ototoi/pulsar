#ifndef __PULSAR_MATRIX4_H__
#define __PULSAR_MATRIX4_H__

#include <cmath>
#include <cstring>

#include "Matrix4.h"

namespace pulsar{

	class Matrix4
	{
	public:
		typedef float   value_type;
		typedef Matrix4 this_type;
	public:
		Matrix4(){}
		Matrix4(const float e[4*4])
		{
			memcpy(e_, e, sizeof(float)*16);
		}
		Matrix4(const Matrix4& rhs)
		{
			using namespace std;
			memcpy(e_, rhs.e_,sizeof(float)*16);
		}
		Matrix4& operator=(const Matrix4& rhs)
		{
			memcpy(e_, rhs.e_,sizeof(float)*16);
			return *this;
		}
		float& operator[](int i)     {return e_[i];}
		float  operator[](int i)const{return e_[i];}
		
#define DECLARE_OP_EQUAL( OP )									\
		this_type& operator OP     (const this_type& rhs){		\
			for(int i=0;i<16;i++)								\
				e_[i] OP rhs.e_[i];								\
			return *this;										\
		}
		
		DECLARE_OP_EQUAL( += )
		DECLARE_OP_EQUAL( -= )
		DECLARE_OP_EQUAL( *= )
		DECLARE_OP_EQUAL( /= )
		
#undef DECLARE_OP_EQUAL

		float& at(int i, int j)     {return e_[4*i+j];}
		float  at(int i, int j)const{return e_[4*i+j];}

	public:
		static Matrix4 Zero()
		{
			static const float f[16] = {};
			return Matrix4(f);
		}
		static Matrix4 Identity()
		{
			static const float f[] = 
			{
				1,0,0,0,
				0,1,0,0,
				0,0,1,0,
				0,0,0,1
			};
			return Matrix4(f);
		}

		static Matrix4 Traslation(float x, float y, float z)
		{
			static const float f[] = 
			{
				1,0,0,x,
				0,1,0,y,
				0,0,1,z,
				0,0,0,1
			};
			return Matrix4(f);
		}

		static Matrix4 RotationY(float radian)
		{
			float s = sin(radian);
			float c = cos(radian);

			static const float f[] = 
			{
				 c, 0, s, 0,
                 0, 1, 0, 0,
                -s, 0, c, 0,
                 0, 0, 0, 1
			};
			return Matrix4(f);
		}

		static Matrix4 Scaling(float x, float y, float z)
		{
			static const float f[] = 
			{
				 x, 0, 0, 0,
                 0, y, 0, 0,
                 0, 0, z, 0,
                 0, 0, 0, 1
			};
			return Matrix4(f);
		}
	private:
		float e_[4*4];
	};

	inline Matrix4 operator* (const Matrix4 &lhs, const Matrix4 &rhs){
		Matrix4 tmp = Matrix4::Zero();
		for(int i =0;i<4;++i){
			for(int k=0;k<4;k++){
				for(int j =0;j<4;++j){
					tmp.at(i,j) += lhs.at(i,k)*rhs.at(k,j);
				}
			}
		}
		return tmp;
	}


	inline Vector3 Multiply(const Matrix4& m, const Vector3& v)
	{
		float p[4] = {v[0], v[1], v[2], 1.0f};
		float q[4];
		for(int i =0;i<4;++i){
			float sum = 0.0f;
			for(int j=0;j<4;j++){
				sum += m.at(i,j)*p[j];    
			}
			q[i] = sum;
		}

		return Vector3(q[0]/q[3],q[1]/q[3],q[2]/q[3]);
	}

}

#endif