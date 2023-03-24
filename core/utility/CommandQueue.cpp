#include "CommandQueue.h"

namespace Slick::Utility {

	CommandQueue::CommandQueue() {}

	CommandQueue::~CommandQueue() {}

	void CommandQueue::submit_command(std::function<void()> fn) {
		std::lock_guard l(mMutex);
		mCommands.push_back(fn);
	}

	void CommandQueue::run_single_command() {
		bool has = false;
		std::function<void()> fn;
		{
			std::lock_guard l(mMutex);
			if (mCommands.size() > 0) {
				fn = mCommands.front();
				mCommands.erase(mCommands.begin());
				has = true;
			}
		}
		if(has)
			fn();
	}

	void CommandQueue::run_commands() {
		std::lock_guard l(mMutex);
		for (auto& fn : mCommands) {
			fn();
		}
		mCommands.clear();
	}

}

