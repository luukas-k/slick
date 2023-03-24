#pragma once

#include "Core.h"

namespace Slick::Utility {

	class CommandQueue {
	public:
		CommandQueue();
		~CommandQueue();

		void submit_command(std::function<void()> fn);

		void run_single_command();
		void run_commands();
	private:
		std::mutex mMutex;
		std::vector<std::function<void()>> mCommands;
	};

}