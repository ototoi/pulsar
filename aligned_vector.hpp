#ifndef __PULSAR_ALIGNED_VECTOR_HPP__
#define __PULSAR_ALIGNED_VECTOR_HPP__

#include "aligned_allocator.hpp"
#include <vector>

namespace pulsar{
    namespace aligned_vector_detail{
	    template<class T>
        struct same_size_traits{
            typedef struct{
                char buffuer[sizeof(T)];
            } type;
        };
    }
	
	template<class T, size_t Sz = 16>
    class aligned_vector
        :public std::vector<typename aligned_vector_detail::same_size_traits<T>::type, aligned_allocator<typename aligned_vector_detail::same_size_traits<T>::type,Sz> >{
    public:
        typedef typename aligned_vector_detail::same_size_traits<T>::type buffer_type;
        typedef std::vector<buffer_type, aligned_allocator<buffer_type> > base_type;
    public:
        aligned_vector(){}
        aligned_vector(const aligned_vector<T>& rhs):base_type(rhs){}
        aligned_vector(size_t sz):base_type(sz){}
        T&       operator[](size_t i)     {return reinterpret_cast<T&>(base_type::operator[](i));}
        const T& operator[](size_t i)const{return reinterpret_cast<const T&>(base_type::operator[](i));}
        void push_back(const T& val)      {base_type::push_back(reinterpret_cast<const buffer_type&>(val));}
        T& front()           {return reinterpret_cast<T&>(base_type::front());}
        const T& front()const{return reinterpret_cast<const T&>(base_type::front());}
        T& back()            {return reinterpret_cast<T&>(base_type::back());}
        const T& back()const {return reinterpret_cast<const T&>(base_type::back());}
        //void swap(aligned_vector<T>& rhs){base_type::swap((base_type&)rhs);}
    };
	
}
#endif
