#pragma once
#pragma warning(push)
// system header files are very noisy
#pragma warning (disable : 4625 4626 4820)

#include <thread>
#include <condition_variable>
#include <functional>

#pragma warning (pop)

namespace Communication
{
	class Timer
	{
	public:

		Timer();

		void Start(
			std::function<void()> timerMethod,
			int timerIntervalMsec);

		~Timer();
		
		void Stop();

	private:

		Timer(const Timer &) = delete;
		void operator=(const Timer &) = delete;
		void StopTimerMethod();
		void StartTimerMethod(int timerIntervalMsec);

		std::function<void()> _timerMethod;
		std::mutex _mutex;
		std::condition_variable _isRunningCondition;
		bool _isRunning;
#pragma warning (suppress : 4820) // 7 bytes padding
		std::thread _thread;
#pragma warning (suppress : 4820) // 4 bytes padding in x86 
	};
}
