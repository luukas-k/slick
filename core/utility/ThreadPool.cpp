#include "ThreadPool.h"

Slick::Utility::ThreadPool::ThreadPool(u32 nThreads) {
	for (u32 i = 0; i < nThreads; i++) {
		mThreads.push_back(std::jthread([this](std::stop_token stop) {
			while (!stop.stop_requested()) {
				mQueue.run_single_command();
				std::this_thread::yield();
			}
		}));
	}
}

Slick::Utility::ThreadPool::~ThreadPool() {
	for (auto& t : mThreads) {
		t.request_stop();
		t.join();
	}
}

void Slick::Utility::ThreadPool::submit_command(std::function<void()> fn) {
	mQueue.submit_command(fn);
}
