#include "Logger.h"

std::vector<std::function<void(const std::string)>> s_log_handlers{
	[](const std::string& msg) {
		std::cout << msg << "\n";
	}
};

void Slick::Utility::register_log_handler(std::function<void(const std::string&)> cb) {
	s_log_handlers.push_back(cb);
}

void Slick::Utility::unregister_log_handler() {
	s_log_handlers.pop_back();
}

void Slick::Utility::write_log(const std::string& msg) {
	for (auto& lh : s_log_handlers) {
		lh(msg);
	}
}
