#ifndef __PULSAR_TRIANGLE_H__
#define __PULSAR_TRIANGLE_H__

#include <cmath>
#include "Vector3.h"

namespace pulsar{
	
	struct Triangle
	{
		union {
			struct {
				const Vector3* points[3];
			};
			struct {
				size_t indices[3];
			};
		};
	};

}

#endif