#pragma once

#include "Logger.h"

namespace Slick::Utility {

	class Timer {
	public:
		Timer();
		~Timer();

		double elapsed();
		void reset();
	private:
		double mStart;
	};

	class ScopeTimer {
	public:
		inline ScopeTimer(const std::string& msg) : mMessage(msg) {}
		inline ~ScopeTimer(){ Utility::Log("Scope: '" + mMessage + "' ended ", mTimer.elapsed()); }
	private:
		std::string mMessage;
		Timer mTimer;
	};

}

