#pragma once

#include "Core.h"


namespace Slick::Utility {

	class FileWatcher {
	public:
		FileWatcher();
		~FileWatcher();

		void monitor(const std::string& fname, std::function<void()> changed);
	private:
		struct MonitorData {
			std::string path;
			u64 last_edit;
			std::function<void()> callback;
		};
		std::jthread mMonitorThread;
		std::mutex mMonitorLock;
		std::vector<MonitorData> mMonitors;
	};

}