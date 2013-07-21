#ifndef __PULSAR_ALIGNED_ALLOCATOR_HPP__
#define __PULSAR_ALIGNED_ALLOCATOR_HPP__

#include <cstdlib>
#include <memory>

#include <stdlib.h>
#if !defined(__APPLE__) && !defined(__OpenBSD__)
#include <malloc.h> // for _alloca, memalign
#endif
#if !defined(WIN32) && !defined(__APPLE__) && !defined(__OpenBSD__)
#include <alloca.h>
#endif

namespace pulsar{
	
	template <typename T, size_t Sz = 16>
    class aligned_allocator
    {
    public:
#ifdef _WIN32
    	static
	    inline void* aligned_malloc(size_t sz, size_t al){
	        return ::_aligned_malloc(sz,al);
	    }

    	static
	    inline void aligned_free(void* p){
	        ::_aligned_free(p);
	    }
#elif defined(__APPLE__) 
		static
	    inline void* aligned_malloc(size_t sz, size_t al){
#if 0
	    	void* p = NULL;
	        posix_memalign(&p,al,sz);
	        return p;
#else
	    	return valloc(sz);
#endif
	    }
		static
	    inline void aligned_free(void* p){
	        free(p);
	    }
#elif defined(__unix__) || defined(__GNUC__) 
    	static
	    inline void* aligned_malloc(size_t sz, size_t al){
	        void* p = NULL;
	        posix_memalign(&p,al,sz);
	        return p;
	    }
		static
	    inline void aligned_free(void* p){
	        free(p);
	    }
#endif
    public:
        // Typedefs
        typedef size_t    size_type;
        typedef ptrdiff_t difference_type;
        typedef T*        pointer;
        typedef const T*  const_pointer;
        typedef T&        reference;
        typedef const T&  const_reference;
        typedef T         value_type;

    public: 
        // Constructors
        aligned_allocator() throw(){}
        aligned_allocator( const aligned_allocator& ) throw(){} 
#if _MSC_VER >= 1300 // VC6 can't handle template members
        template <typename U>
        aligned_allocator( const aligned_allocator<U>& ) throw(){}
#endif
        aligned_allocator& operator=( const aligned_allocator& ){return *this;}

        // Destructor
        ~aligned_allocator() throw(){}

        // Utility functions
        pointer address( reference r ) const{return &r;}
        const_pointer address( const_reference c ) const{return &c;} 
        size_type max_size() const{ return std::numeric_limits<size_t>::max() / sizeof(T); }

        // In-place construction
        void construct( pointer p, const_reference c )
        {                                   
            new( reinterpret_cast<void*>(p) ) T(c); // placement new operator
        }

        // In-place destruction
        void destroy( pointer p )
        {                           
            (p)->~T();    // call destructor directly
        }

        // Rebind to allocators of other types
        template <typename U>
        struct rebind
        {
            typedef aligned_allocator<U> other;
        };

        // Allocate raw memory
        void deallocate(pointer p, size_type)
	    {
	    	if(p)aligned_free((void*)p);
	    }

        pointer allocate(size_type n, const void * = NULL)
        {
            void* p = aligned_malloc(sizeof(T)*n,Sz);
            if(p==NULL)throw std::bad_alloc();
            return pointer(p);
        }
    }; // end of aligned_allocator

    // Comparison
    template <typename T1, typename T2>
    bool operator==( const aligned_allocator<T1>&, const aligned_allocator<T2>& ) throw(){return true;}

    template <typename T1, typename T2>
    bool operator!=( const aligned_allocator<T1>&, const aligned_allocator<T2>& ) throw(){return false;}

}

#endif

