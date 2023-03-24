#include "Timer.h"

#include <GLFW/glfw3.h>

Slick::Utility::Timer::Timer() 
	:
	mStart(0.0)
{
	reset();
}

Slick::Utility::Timer::~Timer() {}

double Slick::Utility::Timer::elapsed() {
	return glfwGetTime() - mStart;
}

void Slick::Utility::Timer::reset() {
	mStart = glfwGetTime();
}
