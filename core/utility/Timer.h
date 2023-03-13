#pragma once

namespace Slick::Utility {

	class Timer {
	public:
		Timer();
		~Timer();

		double elapsed();
	private:
		double mStart;
	};

}