#pragma once

#include "Core.h"

namespace Slick::Net {

	class Connection {
	public:
		Connection();
		~Connection();

		void connect(const std::string& addr, u32 port);
	private:
	};

	class Server {
	public:
		Server();
		~Server();

		bool listen(u32 port);
		void stop();

		inline bool is_active() const { return mActive; }
	private:
		bool mActive;
	};

}