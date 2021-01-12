#include <chrono>

struct timer
{
	typedef std::chrono::high_resolution_clock Clock;
	typedef Clock::time_point TimePoint;
	typedef std::chrono::microseconds MicroSeconds;

	/// Returns the elapsed time, in milliseconds.
	static inline float elapsed(TimePoint start)
	{
		auto diff = get() - start;
		auto duration_us = std::chrono::duration_cast<MicroSeconds>( diff );
		return static_cast<float>( duration_us.count() ) * 0.001f;
	}
	static inline TimePoint get()
	{
		return Clock::now();
	}
};