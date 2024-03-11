#ifndef CTIMER_H
#define CTIMER_H
#include <sys/timeb.h>
#include <chrono>


class CTimer
{
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> endTime;
	std::chrono::duration<long long, std::nano> pausedTime; // Tracks the total time paused
	bool isPaused;

public:
	CTimer() : startTime(), endTime(), pausedTime(0), isPaused(false) {}

	void doStart()
	{
		if (!isPaused)
			startTime = std::chrono::high_resolution_clock::now();
		else
		{
			// Adjust the start time to account for the paused time
			std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
			std::chrono::duration<long long, std::nano> elapsedTime = now - endTime;
			startTime += elapsedTime;
		}
		isPaused = false;
	}

	void doEnd()
	{
		if (!isPaused)
			endTime = std::chrono::high_resolution_clock::now();
	}

	void doReset()
	{
		startTime = endTime = std::chrono::time_point<std::chrono::high_resolution_clock>();
		pausedTime = std::chrono::duration<long long, std::nano>(0);
		isPaused = false;
	}

	void doPause()
	{
		if (!isPaused)
		{
			endTime = std::chrono::high_resolution_clock::now();
			isPaused = true;
		}
	}

	void doResume()
	{
		if (isPaused)
		{
			std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
			std::chrono::duration<long long, std::nano> elapsedTime = now - endTime;
			pausedTime += elapsedTime;
			endTime = std::chrono::time_point<std::chrono::high_resolution_clock>();
			isPaused = false;
		}
	}

	bool isReset()
	{
		return startTime == std::chrono::time_point<std::chrono::high_resolution_clock>();
	}

	int returnElapsed()
	{
		if (isReset())
			return 0;
		else
		{
			std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
			std::chrono::duration<long long, std::nano> elapsed = now - startTime - pausedTime;
			return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
		}
	}

	int returnTotalElapsed()
	{
		if (isReset() || endTime == std::chrono::time_point<std::chrono::high_resolution_clock>())
			return 0;
		else
		{
			std::chrono::duration<long long, std::nano> totalElapsed = endTime - startTime - pausedTime;
			return std::chrono::duration_cast<std::chrono::milliseconds>(totalElapsed).count();
		}
	}
};



class CTimer_no_pause
{
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> endTime;

public:
	CTimer_no_pause() : startTime(), endTime() {}

	void doStart()
	{
		startTime = std::chrono::high_resolution_clock::now();
	}

	void doEnd()
	{
		endTime = std::chrono::high_resolution_clock::now();
	}

	void doReset()
	{
		startTime = endTime = std::chrono::time_point<std::chrono::high_resolution_clock>();
	}

	bool isReset()
	{
		return startTime == std::chrono::time_point<std::chrono::high_resolution_clock>();
	}

	int returnElapsed()
	{
		if (isReset())
			return 0;
		else
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
	}

	int returnTotalElapsed()
	{
		if (isReset() || endTime == std::chrono::time_point<std::chrono::high_resolution_clock>())
			return 0;
		else
			return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
	}
};



class CTimer_OLD
{
private:

	int iStart;
	int iEnd;


	int getMilliCount() {
		timeb tb;
		ftime(&tb);
		int nCount = tb.millitm + (tb.time & 0xfffff) * 1000;
		return nCount;
	}

	int getMilliSpan(int nTimeStart) {
		int nSpan = getMilliCount() - nTimeStart;
		if (nSpan < 0)
			nSpan += 0x100000 * 1000;
		return nSpan;
	}
	int getMilliSpan(int nTimeStart, int nTimeEnd) {
		int nSpan = nTimeEnd - nTimeStart;
		if (nSpan < 0)
			nSpan += 0x100000 * 1000;
		return nSpan;
	}

public:

	CTimer_OLD() : iStart(0), iEnd(0) {}

	
	void	doStart() { if (isReset()) iStart = getMilliCount(); }
	void	doEnd() { if (iStart == 0) return; iEnd = getMilliCount(); }
	void	doReset() { iEnd = iStart = 0; }
	bool	isReset() {
		return (iStart == 0);
	}
	int		returnElapsed() {
		if (!iStart)
			return 0;
		else
			return getMilliSpan(iStart);
	}
	int		returnTotalElapsed() {
		if (!iStart || !iEnd)
			return 0;
		else
			return getMilliSpan(iStart, iEnd);
	}

};


#endif // CTIMER_H
