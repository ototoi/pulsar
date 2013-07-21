#ifndef __PULSAR_Vector3_H__
#define __PULSAR_Vector3_H__

#include <cmath>
#include <cstring>
#include <iostream>
#include <sstream>

namespace pulsar{
	
	class Vector3
	{
	public:
		typedef float   value_type;
		typedef Vector3 this_type;
	public:
		Vector3(){}
		Vector3(float x, float y, float z)
		{
			e_[0]=x;
			e_[1]=y;
			e_[2]=z;
		}
		Vector3(const Vector3& rhs)
		{
			using namespace std;
			memcpy(e_, rhs.e_,sizeof(float)*3);
		}
		Vector3& operator=(const Vector3& rhs)
		{
			memcpy(e_, rhs.e_,sizeof(float)*3);
			return *this;
		}
		float& operator[](int i)     {return e_[i];}
		float  operator[](int i)const{return e_[i];}
		
#define DECLARE_OP_EQUAL( OP )									\
		this_type& operator OP     (const this_type& rhs){		\
			e_[0] OP rhs.e_[0];									\
			e_[1] OP rhs.e_[1];									\
			e_[2] OP rhs.e_[2];									\
			return *this;										\
		}
		
		DECLARE_OP_EQUAL( += )
		DECLARE_OP_EQUAL( -= )
		DECLARE_OP_EQUAL( *= )
		DECLARE_OP_EQUAL( /= )
		
#undef DECLARE_OP_EQUAL
		
		this_type& operator*= (float rhs){
			e_[0] *= rhs;
			e_[1] *= rhs;
			e_[2] *= rhs;
		
			return *this;
		}	 
		this_type& operator/= (float rhs){
			e_[0] /= rhs;
			e_[1] /= rhs;
			e_[2] /= rhs;
		
			return *this;
		}


		this_type& negate(){
			e_[0] *= -1;
			e_[1] *= -1;
			e_[2] *= -1;
			return *this;
		}
		
		float sqr_length()const{
			return e_[0]*e_[0]+e_[1]*e_[1]+e_[2]*e_[2];
		}
		float length()const{
			using namespace std;
			return sqrt(sqr_length());
		}
		
		this_type& normalize(){
			using namespace std;
			
			float l = 1.0f/length();
			e_[0] *= l;
			e_[1] *= l;
			e_[2] *= l;
			
			return *this;
		}
		
	private:
		float e_[3];
	};
	
#define DECLARE_OPERATOR(OP)												\
	inline Vector3 operator OP (const Vector3 &lhs, const Vector3 &rhs){	\
		return Vector3(lhs) OP ## = rhs;									\
	}
		DECLARE_OPERATOR(+)
		DECLARE_OPERATOR(-)
		DECLARE_OPERATOR(*)
		DECLARE_OPERATOR(/)
	
#undef DECLARE_OPERATOR
		
	inline Vector3 operator* (const Vector3 & rhs, float lhs){ 
		return Vector3(rhs) *= lhs ; 
	}
	inline Vector3 operator* (float lhs, const Vector3 & rhs){ 
		return Vector3(rhs) *= lhs ; 
	}
	inline Vector3 operator/ (const Vector3 & rhs, float lhs){ 
		return Vector3(rhs) /= lhs ; 
	}
	inline Vector3 operator/ (float lhs, const Vector3 & rhs){ 
		return Vector3(rhs) /= lhs ; 
	}
	
	inline Vector3 operator+(const Vector3 & rhs){ 
		return rhs; 
	}

	inline Vector3 operator-(const Vector3 & rhs){ 
		return Vector3(rhs).negate(); 
	}


	inline float sqr_length(const Vector3 &rhs){
		return rhs.sqr_length();
	}
	inline float length(const Vector3 &rhs){
		return rhs.length();
	}
	
	inline Vector3 normalize(const Vector3 &rhs){
		return Vector3(rhs).normalize();
	}
	
	inline float dot(const Vector3 &lhs, const Vector3 &rhs){
		return lhs[0]*rhs[0]+lhs[1]*rhs[1]+lhs[2]*rhs[2];
	}
	
	inline Vector3 cross(const Vector3 &lhs, const Vector3 &rhs){
		return
			Vector3(
				lhs[1]*rhs[2] - lhs[2]*rhs[1], //xyzzy
				lhs[2]*rhs[0] - lhs[0]*rhs[2], //yzxxz
				lhs[0]*rhs[1] - lhs[1]*rhs[0]  //zxyyx
			);
	}
	
	inline bool operator== (const Vector3 &lhs, const Vector3 &rhs){
		return (lhs[0] == rhs[0])&&(lhs[1] == rhs[1])&&(lhs[2] == rhs[2]);
	}
	
	inline bool operator!=(const Vector3 &lhs, const Vector3 &rhs){
		return !(lhs == rhs);
	}

	template<typename CharT, class Traits>
	std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const Vector3& rhs){
	    static const int Sz = 3;
		std::basic_ostringstream<CharT, Traits> s;
		s.flags(os.flags());
		s.imbue(os.getloc());
		s.precision(os.precision());
		s << "(";
		for(std::size_t i = 0; i < Sz-1; ++i){
			s << rhs[i] <<",";
		}
		s <<rhs[Sz-1] <<")";
		return os << s.str();
	}
	
}

#endif
