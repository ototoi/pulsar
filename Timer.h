#ifndef __PULSAR_TIMER_H__
#define __PULSAR_TIMER_H__

#include <ctime>

namespace pulsar
{
	class Timer
	{
	public:
		//using namespace std;
		typedef clock_t time_t;
		
		void start(){t_[0] = clock();}
		void end()  {t_[1] = clock();}
		
		time_t sec() {return (time_t)( (t_[1]-t_[0]) / CLOCKS_PER_SEC );}
		time_t msec(){return (time_t)( (t_[1]-t_[0]) * 1000 / CLOCKS_PER_SEC );}
		time_t usec(){return (time_t)( (t_[1]-t_[0]) * 1000000 / CLOCKS_PER_SEC );}
	private:
		time_t t_[2];
	};

}

#endif