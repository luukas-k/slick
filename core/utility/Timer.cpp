#include "Timer.h"

#include <GLFW/glfw3.h>

Slick::Utility::Timer::Timer() 
	:
	mStart(0.0)
{
	mStart = glfwGetTime();
}

Slick::Utility::Timer::~Timer() {}

double Slick::Utility::Timer::elapsed() {
	return glfwGetTime() - mStart;
}
