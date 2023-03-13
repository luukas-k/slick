#include "FileWatcher.h"

#include <filesystem>

#include "Logger.h"

namespace Slick::Utility {

	FileWatcher::FileWatcher() {
		mMonitorThread = std::jthread([this](std::stop_token stop_t) {
			while (!stop_t.stop_requested()) {
				{
					std::lock_guard l(mMonitorLock);
					for (auto& m : mMonitors) {
						u64 t = std::filesystem::last_write_time(m.path).time_since_epoch().count();
						if (m.last_edit != t) {
							m.callback();
							m.last_edit = t;
						}
					}
				}
				std::this_thread::yield();
			}
		});
	}

	FileWatcher::~FileWatcher() {
		mMonitorThread.request_stop();
		mMonitorThread.join();
	}

	void FileWatcher::monitor(const std::string& fname, std::function<void()> changed) {
		std::lock_guard l(mMonitorLock);
		u64 t = std::filesystem::last_write_time(fname).time_since_epoch().count();
		mMonitors.emplace_back(fname, t, changed);
	}

}