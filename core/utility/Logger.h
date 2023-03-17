#pragma once

#include "Core.h"

namespace Slick {

	template<typename T>
	std::string format(T v) {
		return std::to_string(v);
	}
	
	template<typename Q, typename...T>
	std::string format(Q a, T...v) {
		return format<Q>(a) + " " + format<T...>(v...);
	}

	template<>
	inline std::string format<const char*>(const char* v) {
		return v;
	}
	
	template<>
	inline std::string format<std::string>(std::string v) {
		return v;
	}

	template<>
	inline std::string format<bool>(bool v) {
		return v ? "true" : "false";
	}

}

namespace Slick::Utility {

	void register_log_handler(std::function<void(const std::string&)> cb);
	void unregister_log_handler();

	void write_log(const std::string& msg);

	template<typename...T>
	void Log(T...args) {
		std::string formatted = format<T...>(args...);
		write_log(formatted);
	}
	
	template<typename...T>
	void Assert(bool cond, T...args) {
		if (!cond) {
			Log("-----Assertion failed-----\n", args...);
			assert(cond);
			// throw;	
		}
	}

	inline std::string Repeat(const std::string& str, u32 n) {
		std::string res;
		for(u32 i = 0; i < n; i++)
			res.append(str);
		return res;
	}

}