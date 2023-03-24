#pragma once

#include "Core.h"

#include "CommandQueue.h"

namespace Slick::Utility {

	class ThreadPool {
	public:
		ThreadPool(u32 nThreads = 4);
		~ThreadPool();

		void submit_command(std::function<void()> fn);
	private:
		std::vector<std::jthread> mThreads;
		CommandQueue mQueue;
	};

}